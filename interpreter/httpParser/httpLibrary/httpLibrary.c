#include "httpLibrary.h"

#ifndef __unix__
int initWinsock(struct ProcessState* processState){
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

int initSSL(struct ProcessState* processState){
	SSL_library_init();
	return 0;
}

SSL_CTX* createServerContext(struct ProcessState* processState){
	SSL_CTX* ctx;
	SSL_METHOD* method;

	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	method = SSLv23_server_method();
	ctx = SSL_CTX_new(method);

	if (ctx == NULL){
		raiseError("Failed to create context\n", 1, processState);
	}

	return ctx;
}

struct Server* createServer(struct HeaderOptions* headerOptions, struct ProcessState* processState){
	struct Server* socket_s = (struct Server*)malloc(sizeof(struct Server));

	if (headerOptions->tcpOptions->sslOptions->useSSL){
		SSL_CTX *ctx;

	    ctx = createServerContext(processState);
	    SSL_CTX_use_certificate_file(ctx, headerOptions->tcpOptions->sslOptions->sslCertificate, SSL_FILETYPE_PEM);
	    SSL_CTX_use_PrivateKey_file(ctx, headerOptions->tcpOptions->sslOptions->keyPath, SSL_FILETYPE_PEM);

	    if (!SSL_CTX_check_private_key(ctx)){
	    	raiseError("Private key doesn't match public certificate\n", 1, processState);
	    }

	    socket_s->serverCtx = ctx;
	}

    printf("context created\n");

    #ifndef __unix__
	SOCKET s;
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		raiseError("Could not create socket", 1, processState);
	}
	#else
	int s;
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == 0)
	{
		raiseError("Could not create socket", 1, processState);
	}
	#endif

	#ifndef __unix__
	unsigned int timeout = 1;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	#else
	int flags =1;
	setsockopt(s, SOL_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
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
		raiseError("connect error\n", 1, processState);
	}

	#else

	if( bind(s ,(struct sockaddr *)server , sizeof(struct sockaddr_in)) < 0)
	{
		raiseError("connect error\n", 1, processState);
	}

	#endif

	listen(s, socket_s->maxQueue);

	printf("listening on port %d\n", headerOptions->tcpOptions->port);

	return socket_s;
}

struct Client* getClient(struct Server* server, struct HeaderOptions* headerOptions, struct ProcessState* processState){
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

struct RequestLines* getRequestLines(char* request, unsigned int length, char key, struct ProcessState* processState){
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

int getMethod(char* method, struct ProcessState* processState){
	if (!strcmp(method, "GET")) return GET_METHOD;
	if (!strcmp(method, "POST")) return POST_METHOD;
	if (!strcmp(method, "PUT")) return PUT_METHOD;
	if (!strcmp(method, "DELETE")) return DELETE_METHOD;
	if (!strcmp(method, "PATCH")) return PATCH_METHOD;
	if (!strcmp(method, "HEAD")) return HEAD_METHOD;

	return INVALID_METHOD;
}

struct HttpRequest* parseRequestFields(char* request, unsigned int length, struct ProcessState* processState){
	struct HttpRequest* httpRequest = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));

	httpRequest->path = strdup("INVALID");

	struct RequestLines* requestLines = getRequestLines(request, length, '\n', processState);

	if (requestLines->length == 0){
		httpRequest->method = INVALID_METHOD;
		return httpRequest;
	}

	char* requestLine = requestLines->lines[0];

	printf("%s\n", requestLine);

	struct RequestLines* keys = getRequestLines(requestLine, strlen(requestLine), ' ', processState);
	if (keys->length < 3){
		httpRequest->method = INVALID_METHOD;
		return httpRequest;
	}

	int method = getMethod(keys->lines[0], processState);
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
	
	struct HttpRequest* httpRequest = parseRequestFields(message, total, processState);

	free(message);
	free(buf);

	return httpRequest;
}

void sendData(struct Client* client, char* response, struct HeaderOptions* headerOptions, struct ProcessState* processState){
	if (headerOptions->tcpOptions->sslOptions->useSSL){
		SSL_write(client->ssl, response, strlen(response));
	} else {
		int sr = send(client->id, response, strlen(response), 0);
		if (sr < 0){
			printf("Send error: %d\n", sr);
		}
	}
}

char* getClientIP(struct Client* client, struct ProcessState* processState){
	char* ip = inet_ntoa(client->addr->sin_addr);
	return ip;
}

void freeSocket(struct Client* sock, struct HeaderOptions* headerOptions, struct ProcessState* processState){
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