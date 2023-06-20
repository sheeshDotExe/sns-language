#include "httpLibrary.h"

#ifndef __unix__
int init_winsock(struct ProcessState* processState){
	WSADATA wsaData;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	return 0;
}
#endif

int init_SSL(struct ProcessState* processState){
	SSL_library_init();
	return 0;
}

SSL_CTX* create_server_context(struct ProcessState* processState){
	SSL_CTX* ctx;
	SSL_METHOD* method;

	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	method = SSLv23_server_method();
	ctx = SSL_CTX_new(method);

	if (ctx == NULL){
		raise_error("Failed to create context\n", 1, processState);
	}

	return ctx;
}

struct Server* create_server(struct HeaderOptions* headerOptions, struct ProcessState* processState){
	struct Server* socket_s = (struct Server*)malloc(sizeof(struct Server));

	if (headerOptions->tcpOptions->sslOptions->useSSL){
		SSL_CTX *ctx;

	    ctx = create_server_context(processState);
	    SSL_CTX_use_certificate_file(ctx, headerOptions->tcpOptions->sslOptions->sslCertificate, SSL_FILETYPE_PEM);
	    SSL_CTX_use_PrivateKey_file(ctx, headerOptions->tcpOptions->sslOptions->keyPath, SSL_FILETYPE_PEM);

	    if (!SSL_CTX_check_private_key(ctx)){
	    	raise_error("Private key doesn't match public certificate\n", 1, processState);
	    }

	    socket_s->serverCtx = ctx;
	}

    printf("context created\n");

    #ifndef __unix__
	SOCKET s;
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		raise_error("Could not create socket", 1, processState);
	}
	#else
	int s;
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == 0)
	{
		raise_error("Could not create socket", 1, processState);
	}
	#endif

	#ifndef __unix__
	unsigned int timeout = 1;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	#else
	int flags =1;
	//setsockopt(s, SOL_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
	#endif

	int flag = 1;

	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	struct sockaddr_in* server = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	if (!headerOptions->tcpOptions->localHost){
		server->sin_addr.s_addr = INADDR_ANY;
	} else {
		server->sin_addr.s_addr = inet_addr(headerOptions->tcpOptions->hostAddress);
	}
	server->sin_family = AF_INET;
	server->sin_port = htons(headerOptions->tcpOptions->port);

	socket_s->id = s;
	socket_s->maxQueue = headerOptions->tcpOptions->connectionQueue;
	socket_s->addr = server;

	#ifndef __unix__

	if( bind(s ,(struct sockaddr *)server , sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		raise_error("connect error\n", 1, processState);
	}

	#else

	if( bind(s ,(struct sockaddr *)server , sizeof(struct sockaddr_in)) < 0)
	{
		raise_error("connect error\n", 1, processState);
	}

	#endif

	listen(s, socket_s->maxQueue);

	printf("listening on port %d\n", headerOptions->tcpOptions->port);

	return socket_s;
}

struct Client* get_client(struct Server* server, struct HeaderOptions* headerOptions, struct ProcessState* processState){
	struct Client* client = (struct Client*)malloc(sizeof(struct Client));
	struct sockaddr_in* addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));

	int size = sizeof(struct sockaddr_in);

	client->addr = addr;

	#ifndef __unix__
	SOCKET c = accept(server->id, (struct sockaddr *)addr, &size);

	client->id = c;
	if (c == INVALID_SOCKET)
	{
		client->valid = 0;
		return client;
	}
	#else
	int c = accept(server->id, (struct sockaddr *)addr, &size);

	int flags = fcntl(c, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(c, F_SETFL, flags);

	client->id = c;
	if (c < 0)
	{
		client->valid = 0;
		return client;
	} 
	#endif

	if (headerOptions->tcpOptions->sslOptions->useSSL){
		client->ssl = SSL_new(server->serverCtx);

		BIO* bio = BIO_new_socket(client->id, BIO_NOCLOSE);
		SSL_set_bio(client->ssl, bio, bio);

		//SSL_set_fd(client->ssl, client->id);
		if (SSL_accept(client->ssl) == -1){
			ERR_print_errors_fp(stderr);
			return NULL;
		}
	}

	client->maxQueue = 0;
	client->valid = 1;

	return client;
}

struct RequestLines* get_request_lines(char* request, unsigned int length, char key, struct ProcessState* processState){
	struct RequestLines* requestLines = (struct RequestLines*)malloc(sizeof(struct RequestLines));

	int numberOfLines = 1;
	for (int i = 0; i < length; i++){
		if (request[i] == key) numberOfLines++;
	}

	int* lineLoc = (int*)malloc((numberOfLines)*sizeof(int));
	int lineIndex = 0;
	for (int i = 0; i < length; i++){
		if (request[i] == key){
			lineLoc[lineIndex] = i;
			lineIndex++;
		}
	}
	lineLoc[lineIndex] = length;

	requestLines->lines = (char**)malloc(numberOfLines*sizeof(char*));
	requestLines->length = numberOfLines;

	int lastIndex = 0;
	for (int i = 0; i < numberOfLines; i++){
		requestLines->lines[i] = (char*)malloc(((lineLoc[i] - lastIndex)+1)*sizeof(char));
		int z = 0;
		for (int j = lastIndex; j < lineLoc[i]; j++){
			requestLines->lines[i][z] = request[j];
			z++;
		}
		requestLines->lines[i][z] = '\0';
		lastIndex = lineLoc[i] + 1;
	}

	free(lineLoc);

	return requestLines;
}

int get_method(char* method, struct ProcessState* processState){
	if (!strcmp(method, "GET")) return GET_METHOD;
	if (!strcmp(method, "POST")) return POST_METHOD;
	if (!strcmp(method, "PUT")) return PUT_METHOD;
	if (!strcmp(method, "DELETE")) return DELETE_METHOD;
	if (!strcmp(method, "PATCH")) return PATCH_METHOD;
	if (!strcmp(method, "HEAD")) return HEAD_METHOD;

	return INVALID_METHOD;
}

struct HttpRequest* parse_request_fields(char* request, unsigned int length, struct ProcessState* processState){
	struct HttpRequest* httpRequest = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));

	httpRequest->path = strdup("INVALID");

	struct RequestLines* requestLines = get_request_lines(request, length, '\n', processState);

	if (requestLines->length == 0){
		httpRequest->method = INVALID_METHOD;
		return httpRequest;
	}

	char* requestLine = requestLines->lines[0];

	printf("%s\n", requestLine);

	struct RequestLines* keys = get_request_lines(requestLine, strlen(requestLine), ' ', processState);
	if (keys->length < 3){
		httpRequest->method = INVALID_METHOD;
		return httpRequest;
	}

	int method = get_method(keys->lines[0], processState);
	httpRequest->method = method;

	if (method == INVALID_METHOD){
		return httpRequest;
	}

	httpRequest->path = strdup(keys->lines[1]);

	for (int i = 0; i < keys->length; i++){
		free(keys->lines[i]);
	}
	free(keys->lines);
	free(keys);

	for (int i = 0; i < requestLines->length; i++){
		free(requestLines->lines[i]);
	}
	free(requestLines->lines);
	free(requestLines);

	return httpRequest;
}


struct HttpRequest* recive(struct Client* client, struct HeaderOptions* headerOptions, struct ProcessState* processState){

	unsigned char* buf = (unsigned char*)malloc(MAX_PACKET_SIZE*sizeof(unsigned char));
	int read = 1;
	int total = 0;

	#ifndef __unix__
	while (read > 0){
		if (headerOptions->tcpOptions->sslOptions->useSSL){
			read = SSL_read(client->ssl, buf + total, (MAX_PACKET_SIZE - total)*sizeof(unsigned char));
		} else {
			read = recv(client->id, buf + total, (MAX_PACKET_SIZE - total)*sizeof(unsigned char), 0);
		}
		if (read > 0){
			total += read;
		}
	}
	#else 
	if (headerOptions->tcpOptions->sslOptions->useSSL){
		read = SSL_read(client->ssl, buf, (MAX_PACKET_SIZE)*sizeof(unsigned char));
	} else {
		read = recv(client->id, buf, (MAX_PACKET_SIZE)*sizeof(unsigned char), 0);
	}
	if (read > 0) total += read;
	#endif

	buf[total] = '\0';

	char* message = (char*)malloc((total+1)*sizeof(char));

	if (total) memcpy(message, buf, total*sizeof(char));
	message[total] = '\0';

	printf("debug: %s\n", message);
	
	struct HttpRequest* httpRequest = parse_request_fields(message, total, processState);

	free(message);
	free(buf);

	return httpRequest;
}

void send_data(struct Client* client, char* response, struct HeaderOptions* headerOptions, struct ProcessState* processState){
	if (headerOptions->tcpOptions->sslOptions->useSSL){
		SSL_write(client->ssl, response, strlen(response));
	} else {
		int sr = send(client->id, response, strlen(response), 0);
		if (sr < 0){
			printf("Send error: %d\n", sr);
		}
	}
}

char* get_client_ip(struct Client* client, struct ProcessState* processState){
	char* ip = inet_ntoa(client->addr->sin_addr);
	return ip;
}

void free_socket(struct Client* sock, struct HeaderOptions* headerOptions, struct ProcessState* processState){
	if (headerOptions->tcpOptions->sslOptions->useSSL){
		SSL_free(sock->ssl);
	}
	#ifndef __unix__
	closesocket(sock->id);
	#else
	close(sock->id);
	#endif
	free(sock->addr);
	free(sock);
}