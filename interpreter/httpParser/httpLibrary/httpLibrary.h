#ifndef HTTP_LIBRARY
#define HTTP_LIBRARY

#include <winsock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

#include "../../header/findHeader.h"
#include "../../types/types.h"
#include "../../body/interpretBody.h"
#include "../../body/bodyTypes.h"

enum METHODS {
	GET_METHOD,
	POST_METHOD,
	HEAD_METHOD,
	PUT_METHOD,
	DELETE_METHOD,
	CONNECT_METHOD,
	OPTIONS_METHOD,
	TRACE_METHOD,
	PATCH_METHOD,
	INVALID_METHOD,
};

struct RequestLines {
	char** lines;
	unsigned int length;
};

#define MAX_PACKET_SIZE 65535
#define CERT_FILE "server.crt"
#define KEY_FILE "server.key"

struct HttpRequest {
	int method;
	char* path;
};

struct Socket {
	SOCKET id;
	struct sockaddr_in* addr;
	int maxQueue;
	int valid;
};

int initWinsock();
struct Socket* createServer(struct HeaderOptions* headerOptions);
struct Socket* getClient(struct Socket* server);
void freeSocket(struct Socket* sock);
char* getClientIP(struct Socket* client);
struct HttpRequest* recive(struct Socket* client);
void sendData(struct Socket* client, char* response);

#endif