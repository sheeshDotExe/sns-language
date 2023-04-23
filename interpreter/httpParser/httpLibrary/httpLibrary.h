#ifndef HTTP_LIBRARY
#define HTTP_LIBRARY

#include <sys/socket.h>

enum METHODS {
	GET,
	POST,
	HEAD,
	PUT,
	DELETE,
	CONNECT,
	OPTIONS,
	TRACE,
	PATCH,
};

struct Socket {
	int id;
	int domain;
	int type;
	int protocol;
};

struct Socket* createServer()

#endif