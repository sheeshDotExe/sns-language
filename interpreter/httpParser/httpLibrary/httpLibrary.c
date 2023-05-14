#include "httpLibrary.h"

int initWinsock(){
	WSADATA wsaData;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	//SSL_library_init();

	return 0;
}

struct Socket* createServer(struct HeaderOptions* headerOptions){
	struct Socket* socket_s = (struct Socket*)malloc(sizeof(struct Socket));

	//SSL_CTX *ctx;
    //SSL *ssl;

    //ctx = SSL_CTX_new(SSLv23_server_method());
    //SSL_CTX_use_certificate_file(ctx, CERT_FILE, SSL_FILETYPE_PEM);
    //SSL_CTX_use_PrivateKey_file(ctx, KEY_FILE, SSL_FILETYPE_PEM);

    printf("context created\n");

	SOCKET s;
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		raiseError("Could not create socket", 1);
	}

	unsigned int timeout = 1;

	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(unsigned int));

	struct sockaddr_in* server = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	if (!headerOptions->tcpOptions.localHost){
		server->sin_addr.s_addr = INADDR_ANY;
	} else {
		server->sin_addr.s_addr = inet_addr(headerOptions->tcpOptions.hostAddress);
	}
	server->sin_family = AF_INET;
	server->sin_port = htons(headerOptions->tcpOptions.port);

	socket_s->id = s;
	socket_s->maxQueue = headerOptions->tcpOptions.connectionQueue;
	socket_s->addr = server;

	if( bind(s ,(struct sockaddr *)server , sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		raiseError("connect error\n", 1);
	}

	listen(s, socket_s->maxQueue);

	printf("listening on port %d\n", headerOptions->tcpOptions.port);

	return socket_s;
}

struct Socket* getClient(struct Socket* server){
	struct Socket* client = (struct Socket*)malloc(sizeof(struct Socket));
	struct sockaddr_in* addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));

	int size = sizeof(struct sockaddr_in);

	client->addr = addr;
	SOCKET c = accept(server->id, (struct sockaddr *)addr, &size);

	client->id = c;
	if (c == INVALID_SOCKET)
	{
		client->valid = 0;
		return client;
	}
	client->maxQueue = 0;
	client->valid = 1;

	return client;
}

struct RequestLines* getRequestLines(char* request, unsigned int length, char key){
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
		requestLines->lines[i] = (char*)malloc((lineLoc[i] - lastIndex)+1);
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

int getMethod(char* method){
	if (!strcmp(method, "GET")) return GET_METHOD;
	if (!strcmp(method, "POST")) return POST_METHOD;
	if (!strcmp(method, "PUT")) return PUT_METHOD;
	if (!strcmp(method, "DELETE")) return DELETE_METHOD;
	if (!strcmp(method, "PATCH")) return PATCH_METHOD;
	if (!strcmp(method, "HEAD")) return HEAD_METHOD;

	return INVALID_METHOD;
}

struct HttpRequest* parseRequestFields(char* request, unsigned int length){
	struct HttpRequest* httpRequest = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));

	httpRequest->path = strdup("INVALID");

	struct RequestLines* requestLines = getRequestLines(request, length, '\n');

	if (requestLines->length == 0){
		httpRequest->method = INVALID_METHOD;
		return httpRequest;
	}

	char* requestLine = requestLines->lines[0];

	printf("%s\n", requestLine);

	struct RequestLines* keys = getRequestLines(requestLine, strlen(requestLine), ' ');
	if (keys->length < 3){
		httpRequest->method = INVALID_METHOD;
		return httpRequest;
	}

	int method = getMethod(keys->lines[0]);
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

struct HttpRequest* recive(struct Socket* client){

	char buf[MAX_PACKET_SIZE];
	int read = 1;
	int total = 0;

	while (read > 0){
		read = recv(client->id, &buf + total, MAX_PACKET_SIZE - total, 0);
		if (read > 0){
			total += read;
		}
	}

	char* message = (char*)malloc((total+1)*sizeof(char));
	memcpy(message, &buf, total);
	message[total] = '\0';

	struct HttpRequest* httpRequest = parseRequestFields(message, total);

	free(message);

	return httpRequest;
}

void sendData(struct Socket* client, char* response){
	send(client->id, response, strlen(response), 0);
}

char* getClientIP(struct Socket* client){
	return inet_ntoa(client->addr->sin_addr);
}

void freeSocket(struct Socket* sock){
	closesocket(sock->id);
	free(sock->addr);
	free(sock);
}