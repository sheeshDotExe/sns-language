#ifndef HTTP_LIBRARY
#define HTTP_LIBRARY

#ifdef __unix__
#include <sys/socket.h>

#else
#include <winsock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>
#include <openssl/bio.h>
#endif

#include <wchar.h>

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
#define CERT_FILE "certificate/certificate.crt"
#define KEY_FILE "certificate/private.key"

struct HttpRequest {
	int method;
	char* path;
};

struct Server {
	SOCKET id;
	struct sockaddr_in* addr;
	int maxQueue;
	int valid;
	SSL_CTX* serverCtx;
};

struct Client {
	SOCKET id;
	struct sockaddr_in* addr;
	int maxQueue;
	int valid;
	SSL* ssl;
};

int initWinsock();
struct Server* createServer(struct HeaderOptions* headerOptions);
struct Client* getClient(struct Server* server);
void freeSocket(struct Client* sock);
char* getClientIP(struct Client* client);
struct HttpRequest* recive(struct Client* client);
void sendData(struct Client* client, char* response);

#endif