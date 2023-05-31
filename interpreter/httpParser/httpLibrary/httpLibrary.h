#ifndef HTTP_LIBRARY
#define HTTP_LIBRARY

#ifdef __unix__
#include <sys/socket.h>
#include <arpa/inet.h>

#else
#include <winsock2.h>
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>
#include <openssl/bio.h>

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

// unix sockets
#ifdef __unix__
struct Server {
	int id;
	struct sockaddr_in* addr;
	int maxQueue;
	int valid;
	SSL_CTX* serverCtx;
};

struct Client {
	int id;
	struct sockaddr_in* addr;
	int maxQueue;
	int valid;
	SSL* ssl;
};

#else

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

#endif

#ifndef __unix___
int initWinsock();
#endif
int initSSL();
struct Server* createServer(struct HeaderOptions* headerOptions);
struct Client* getClient(struct Server* server, struct HeaderOptions* headerOptions);
void freeSocket(struct Client* sock, struct HeaderOptions* headerOptions);
char* getClientIP(struct Client* client);
struct HttpRequest* recive(struct Client* client, struct HeaderOptions* headerOptions);
void sendData(struct Client* client, char* response, struct HeaderOptions* headerOptions);

#endif