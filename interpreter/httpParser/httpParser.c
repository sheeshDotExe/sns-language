#include "httpParser.h"

#ifndef __unix__
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep(x*1000)
#endif

struct ThreadInfo* create_thread_info(struct State* state, struct HeaderOptions* headerOptions, struct Server* server){
	struct ThreadInfo* threadInfo = (struct ThreadInfo*)malloc(sizeof(struct ThreadInfo));
	threadInfo->headerOptions = headerOptions;
	threadInfo->state = state;
	threadInfo->processState = (struct ProcessState*)malloc(sizeof(struct ProcessState));
	threadInfo->processState->running = 1;
	threadInfo->server = server;
	return threadInfo;
}

void start_HTTP_server(struct State* state, struct HeaderOptions* headerOptions, struct Body* body, struct ProcessState* processState){

	#ifndef __unix__
	if (init_winsock(processState)){
		raise_error("winsock init failed", 1, processState);
	}
	#endif

	init_SSL(processState);

	struct Server* server = create_server(headerOptions, processState);
	printf("server started\n");

	int numberOfThreads = 10;

	pthread_t* ids = (pthread_t*)malloc(numberOfThreads*sizeof(pthread_t));
	pthread_t crashHandlerId;
 	
 	struct ThreadInfo** threadInfos = (struct ThreadInfo**)malloc(numberOfThreads*sizeof(struct ThreadInfo*));

	for (int i = 0; i < numberOfThreads; i++){
		struct State* copiedState = hardcopy_state(state, processState);
		struct ThreadInfo* threadInfo = create_thread_info(copiedState, headerOptions, server);
		threadInfos[i] = threadInfo;
		ids[i] = pthread_create(&ids[i], NULL, request_handler, (void*)threadInfo);
	}

	struct CrashHandlerInfo* crashHandlerInfo = (struct CrashHandlerInfo*)malloc(sizeof(struct CrashHandlerInfo));
	
	crashHandlerInfo->numberOfThreads = numberOfThreads;
	crashHandlerInfo->ids = ids;
	crashHandlerInfo->threadInfos = threadInfos;

	crashHandlerInfo->state = state;
	crashHandlerInfo->headerOptions = headerOptions;
	crashHandlerInfo->server = server;

	crashHandlerInfo->processState = (struct ProcessState*)malloc(sizeof(struct ProcessState));
	crashHandlerInfo->processState->running = 1;

	crashHandlerId = pthread_create(&crashHandlerId, NULL, crash_handler, (void*)crashHandlerInfo);

	if (crashHandlerId != 0){
		printf("failed to create thread, %d\n", errno);
		return;
	}

	while (crashHandlerInfo->processState->running) Sleep(1);
	printf("Crashhandler crashed ;(\n");
	return;
}

void* crash_handler(void* threadData){
	struct CrashHandlerInfo* crashHandlerInfo = (struct CrashHandlerInfo*)threadData;

	int numberOfThreads = crashHandlerInfo->numberOfThreads;
	pthread_t* ids = crashHandlerInfo->ids;
	struct ThreadInfo** threadInfos = crashHandlerInfo->threadInfos;

	struct State* state = crashHandlerInfo->state;
	struct HeaderOptions* headerOptions = crashHandlerInfo->headerOptions;
	struct Server* server = crashHandlerInfo->server;

	struct ProcessState* processState = crashHandlerInfo->processState;


	while (1){
		// handle thread crash [500]
		for (int i = 0; i < numberOfThreads; i++){
			struct ThreadInfo* threadInfo = threadInfos[i];
			if (!threadInfo->processState->running){
				printf("Thread %d crashed, restarting thread\n", i+1);

				processState->running = 0;
				pthread_exit(NULL);

				struct Request* request = threadInfo->currentRequest;

				send_data(request->client, "HTTP/1.1 500\r\nContent-Type: text/plain\r\n\r\nThread crashed", headerOptions, processState);
				free_socket(request->client, headerOptions, processState);
				free(request);

				free(threadInfo->processState);
				free(threadInfo);

				struct ThreadInfo* newThreadInfo = create_thread_info(state, headerOptions, server);
				threadInfos[i] = newThreadInfo;
				ids[i] = pthread_create(&ids[i], NULL, request_handler, (void*)newThreadInfo);
			}
		}
		Sleep(1);
	}

	processState->running = 0;
	pthread_exit(NULL);
}

void* request_handler(void* threadData){
	struct ThreadInfo* threadInfo = (struct ThreadInfo*)threadData;

	struct HeaderOptions* headerOptions = threadInfo->headerOptions;
	struct State* state = threadInfo->state;
	struct ProcessState* processState = threadInfo->processState;
	struct Server* server = threadInfo->server;

	while (1){
		struct Client* client = get_client(server, headerOptions, processState);

		if (client == NULL){
			free_socket(client, headerOptions, processState);
			continue;
		}

		if (!client->valid){
			#ifndef __unix__
			printf("error %d\n", WSAGetLastError());
			#else
			printf("error %d\n", errno);
			#endif
			continue;
		}

		struct Request* request = (struct Request*)malloc(sizeof(struct Request));
		request->client = client;
		request->nextExists = 0;

		// we now have a private request for this thread
		threadInfo->currentRequest = request;
		handle_request(request->client, state, headerOptions, processState);
		free(request);

		Sleep(1);
	}
}

int handle_request(struct Client* client, struct State* state, struct HeaderOptions* headerOptions, struct ProcessState* processState){
	
	time_t currentTime = time(0);
	char* localTime = ctime(&currentTime);
	char* timeFormatted = strtok(localTime, "\n");
	printf("\n\n[%s] IP: %s\n",timeFormatted, get_client_ip(client, processState));
	//free(localTime);

	struct HttpRequest* httpRequest = recive(client, headerOptions, processState);

	char* response = parse_request(state, httpRequest, processState);

	send_data(client, response, headerOptions, processState);

	free(response);

	free(httpRequest->path);
	free(httpRequest);

	free_socket(client, headerOptions, processState);

	return 0;
}

int is_valid_route(struct Path* path, struct Route* route, struct ProcessState* processState){

	struct Path* serverPath = route->path;

	if (!route->isStatic && path->folderCount != serverPath->folderCount) return 0;

	unsigned int currentVarIndex = 0;

	for (int i = 0; i < serverPath->folderCount; i++){
		if (!serverPath->varCount || serverPath->varIndexes[currentVarIndex] != i){
			if (strcmp(path->folders[i], serverPath->folders[i])) return 0;
		}
		else if (serverPath->varCount && serverPath->varIndexes[currentVarIndex] == i){
			if (route->isStatic) continue; // var type doesn't matter for static files

			struct Var* pathVar = serverPath->pathVars[currentVarIndex];

			struct Var* otherVar = generate_var_from_string(path->folders[i], strlen(path->folders[i]), processState);

			struct CommonTypes commonTypes = get_common_types(pathVar, otherVar, processState);
			free(commonTypes.codes);

			free_var(otherVar, processState);
			free(otherVar);

			if (!commonTypes.length) return 0;

			currentVarIndex++;
		}
	}

	return 1;
}

struct PathInfo* get_valid_route(struct State* state, struct HttpRequest* request, struct ProcessState* processState){

	struct PathInfo* pathInfo = (struct PathInfo*)malloc(sizeof(struct PathInfo));
	pathInfo->valid = 0;

	struct Path* path = generate_path(request->path, strlen(request->path), processState); // sus strlen

	struct Routes* routes = state->routes;

	for (int i = 0; i < routes->numberOfRoutes; i++){
		struct Route* route = routes->routes[i];

		int check = is_valid_route(path, route, processState);

		if (!check) continue;

		pathInfo->valid = 1;
		pathInfo->route = route;

		free_path(path, processState);

		return pathInfo;
	}

	free_path(path, processState);

	return pathInfo;
}

struct Var* parse_route(struct State* state, struct Route* route, struct HttpRequest* request, struct ProcessState* processState){
	struct Path* path = generate_path(request->path, strlen(request->path), processState); // sus strlen

	struct Path* serverPath = route->path;

	unsigned int currentVarIndex = 0;

	for (int i = 0; i < serverPath->folderCount; i++){

		if (serverPath->varCount && serverPath->varIndexes[currentVarIndex] == i){
			struct Var* pathVar = serverPath->pathVars[currentVarIndex];
			struct Param* param = route->function->param;
			struct Var* paramVar = param->inputVars[currentVarIndex];

			struct Var* otherVar = NULL;
			if (!route->isStatic) otherVar = generate_var_from_string(path->folders[i], strlen(path->folders[i]), processState);
			else otherVar = generate_var_from_string(request->path, strlen(request->path), processState);

			assign_value(paramVar, otherVar, processState);

			free_var(otherVar, processState);
			free(otherVar);

			currentVarIndex++;
		}
	}

	route->function->function->varScope = create_var_scope(route->function, processState);
	struct State* copiedState = copy_state(state, processState);
	struct Var* returnValue = call_function(route->function, copiedState, processState);

	free_param(route->function->param, processState);
	route->function->param = copy_param(route->function->originalParam, processState);

	free(copiedState);
	free_var_scope(route->function->function->varScope, processState);

	free_path(path, processState);

	return returnValue;
}

char* get_valid_content_type(char* extension, struct ProcessState* processState){
	if (!strcmp(extension, ".html")) return "text/html";
	if (!strcmp(extension, ".css")) return "text/css";
	if (!strcmp(extension, ".js")) return "application/javascript";

	return "text/plain";
}

char* parse_request(struct State* state, struct HttpRequest* request, struct ProcessState* processState){

	if (request->method == INVALID_METHOD){
		return strdup("HTTP/1.1 400 Bad Request\r\n\r\n");
	}

	struct PathInfo* pathInfo = get_valid_route(state, request, processState);
	if (!pathInfo->valid){
		free(pathInfo);
		return strdup("HTTP/1.1 404 Not Found\r\n\r\n");
	}

	struct Var* returnValue = parse_route(state, pathInfo->route, request, processState);
	struct String* responseString = (struct String*)(get_type(String_c, returnValue, processState)->type);
	char* response = responseString->cString;

	char* ok = "HTTP/1.1 200 OK\r\nContent-Type: ";
	char* contentLength = ";charset=utf-8\r\nContent-Length: ";

	char* contentType = get_valid_content_type(state->fileExtension[0], processState);

	char* length = (char*)malloc(100);
	sprintf(length, "%d", strlen(response));

	int responseLength = strlen(response) + strlen(ok) + strlen(contentLength) + strlen(length) + strlen(contentType) + 4 + 1;

	char* httpResponse = (char*)malloc(responseLength*sizeof(char));

	strcpy(httpResponse, ok);
	strcat(httpResponse, contentType);
	strcat(httpResponse, contentLength);
	strcat(httpResponse, length);
	strcat(httpResponse, "\r\n\r\n");
	strcat(httpResponse, response);
	httpResponse[responseLength-1] = '\0';

	free(length);
	free(pathInfo);
	free_var(returnValue, processState);
	free(returnValue);

	//httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain;charset=utf-8\r\nContent-Length:15\r\n\r\nunicode: ååå";

	return httpResponse;
}

struct Path* generate_path(char* rawPath, unsigned int length, struct ProcessState* processState){
	struct Path* path = (struct Path*)malloc(sizeof(struct Path));
	struct SplitPath* splitPath = get_split_path(rawPath, length, processState);

	path->folders = (char**)malloc((splitPath->length-1) * sizeof(char*));
	path->folderCount = splitPath->length-1;

	for (int i = 0; i < splitPath->length-1; i++){
		unsigned int size = splitPath->nextPath[i+1] - splitPath->nextPath[i];
		path->folders[i] = (char*)malloc((size) * sizeof(char));
		memcpy(path->folders[i], rawPath + splitPath->nextPath[i] + 1, (size - 1)*sizeof(char));
		path->folders[i][size - 1] = '\0';
	}

	free(splitPath->nextPath);
	free(splitPath);

	return path;
}

void free_path(struct Path* path, struct ProcessState* processState){
	for (int i = 0; i < path->folderCount; i++){
		free(path->folders[i]);
	}
	free(path->folders);
	free(path);
}