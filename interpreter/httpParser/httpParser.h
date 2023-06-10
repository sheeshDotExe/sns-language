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
#include "../body/functionLogic/functionLogic.h"

#include "../processState.h"

#ifdef __unix__
#include <errno.h>
#endif

struct ThreadInfo {
	struct HeaderOptions* headerOptions;
	struct State* state;
	struct ProcessState* processState;
	struct Request* currentRequest;
	struct Server* server;
};

struct CrashHandlerInfo {
	int numberOfThreads;
	pthread_t* ids;
	struct HeaderOptions* headerOptions;
	struct State* state;
	struct Server* server;
	struct ProcessState* processState;
	struct ThreadInfo** threadInfos;
};

struct Request {
	struct Client* client;
	struct Request* nextRequest;
	int nextExists;
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
void* crashHandler(void* threadData);

#endif