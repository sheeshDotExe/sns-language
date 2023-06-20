#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "../header/findHeader.h"
#include "../types/types.h"
#include "../body/interpretBody.h"
#include "../body/bodyTypes.h"
#include "../builtins/builtins.h"
#include "httpLibrary/httpLibrary.h"
#include <time.h>
#ifdef __unix__
#include <pthread.h>
#endif
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

#ifdef __unix__
struct CrashHandlerInfo {
	int numberOfThreads;
	pthread_t* ids;
	struct HeaderOptions* headerOptions;
	struct State* state;
	struct Server* server;
	struct ProcessState* processState;
	struct ThreadInfo** threadInfos;
};
#else
struct CrashHandlerInfo {
	int numberOfThreads;
	HANDLE* ids;
	struct HeaderOptions* headerOptions;
	struct State* state;
	struct Server* server;
	struct ProcessState* processState;
	struct ThreadInfo** threadInfos;
};
#endif

struct Request {
	struct Client* client;
	struct Request* nextRequest;
	int nextExists;
};

struct PathInfo {
	int valid;
	struct Route* route;
};

void start_HTTP_server(struct State* state, struct HeaderOptions* headerOptions, struct Body* body, struct ProcessState* processState);
char* parse_request(struct State* state, struct HttpRequest* request, struct ProcessState* processState);
struct Path* generate_path(char* rawPath, unsigned int length, struct ProcessState* processState);
void free_path(struct Path* path, struct ProcessState* processState);
int handle_request(struct Client* client, struct State* state, struct HeaderOptions* headerOptions, struct ProcessState* processState);
void* request_handler(void* threadData);
void* crash_handler(void* threadData);

#endif