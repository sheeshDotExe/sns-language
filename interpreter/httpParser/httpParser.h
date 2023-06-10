#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "../header/findHeader.h"
#include "../types/types.h"
#include "../body/interpretBody.h"
#include "../body/bodyTypes.h"
#include "../builtins/builtins.h"
#include "httpLibrary/httpLibrary.h"
#include <time.h>
#include <pthread.h>

#include "../processState.h"

#ifdef __unix__
#include <errno.h>
#endif

struct ThreadInfo {
	struct HeaderOptions* headerOptions;
	struct State* state;
	struct ProcessState* processState;
	struct RequestQueue* requestQueue;
	struct pthread_mutex_t* lock;
};

struct Request {
	struct Client* client;
	struct Request* nextRequest;
	int nextExists;
};

struct RequestQueue {
	struct Request* current;
	struct Request* last;
	int hasClient;
};

struct PathInfo {
	int valid;
	struct Route* route;
};

void startHTTPServer(struct State* state, struct HeaderOptions* headerOptions, struct Body* body, struct ProcessState* processState);
char* parseRequest(struct State* state, struct HttpRequest* request, struct ProcessState* processState);
struct Path* generatePath(char* rawPath, unsigned int length, struct ProcessState* processState);
void freePath(struct Path* path, struct ProcessState* processState);
int handleRequest(struct Client* client, struct State* state, struct HeaderOptions* headerOptions, struct ProcessState* processState);
void* requestHandler(void* threadData);

#endif