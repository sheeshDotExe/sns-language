#include "httpParser.h"

void startHTTPServer(struct State* state, struct HeaderOptions* headerOptions, struct Body* body){

	#ifndef __unix__
	if (initWinsock()){
		raiseError("winsock init failed", 1);
	}
	#endif

	initSSL();

	struct Server* server = createServer(headerOptions);
	printf("server started\n");

	while (1){
		struct Client* client = getClient(server, headerOptions);

		if (client == NULL){
			freeSocket(client, headerOptions);
			continue;
		}

		if (client->valid){
			printf("\n\nnew client %s\n", getClientIP(client));

			struct HttpRequest* httpRequest = recive(client, headerOptions);

			printf("recived request\n");

			char* response = parseRequest(state, httpRequest);

			sendData(client, response, headerOptions);

			printf("sent response %s\n", response);

			free(response);

			free(httpRequest->path);
			free(httpRequest);
		} else {
			#ifndef __unix__
			printf("error %d\n", WSAGetLastError());
			#else
			printf("error %d\n", errno);
			#endif
		}
		freeSocket(client, headerOptions);
	}
}

int isValidRoute(struct Path* path, struct Route* route){

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

			struct Var* otherVar = generateVarFromString(path->folders[i], strlen(path->folders[i]));

			struct CommonTypes commonTypes = getCommonTypes(pathVar, otherVar);
			free(commonTypes.codes);

			freeVar(otherVar);
			free(otherVar);

			if (!commonTypes.length) return 0;

			currentVarIndex++;
		}
	}

	return 1;
}

struct PathInfo* getValidRoute(struct State* state, struct HttpRequest* request){

	struct PathInfo* pathInfo = (struct PathInfo*)malloc(sizeof(struct PathInfo));
	pathInfo->valid = 0;

	struct Path* path = generatePath(request->path, strlen(request->path)); // sus strlen

	struct Routes* routes = state->routes;

	for (int i = 0; i < routes->numberOfRoutes; i++){
		struct Route* route = routes->routes[i];

		int check = isValidRoute(path, route);

		if (!check) continue;

		pathInfo->valid = 1;
		pathInfo->route = route;

		freePath(path);

		return pathInfo;
	}

	freePath(path);

	return pathInfo;
}

struct Var* parseRoute(struct State* state, struct Route* route, struct HttpRequest* request){
	struct Path* path = generatePath(request->path, strlen(request->path)); // sus strlen

	struct Path* serverPath = route->path;

	unsigned int currentVarIndex = 0;

	for (int i = 0; i < serverPath->folderCount; i++){

		if (serverPath->varCount && serverPath->varIndexes[currentVarIndex] == i){
			struct Var* pathVar = serverPath->pathVars[currentVarIndex];
			struct Param* param = route->function->param;
			struct Var* paramVar = param->inputVars[currentVarIndex];

			struct Var* otherVar = NULL;
			if (!route->isStatic) otherVar = generateVarFromString(path->folders[i], strlen(path->folders[i]));
			else otherVar = generateVarFromString(request->path, strlen(request->path));

			assignValue(paramVar, otherVar);

			freeVar(otherVar);
			free(otherVar);

			currentVarIndex++;
		}
	}

	route->function->function->varScope = createVarScope(route->function);
	struct State* copiedState = copyState(state);
	struct Var* returnValue = callFunction(route->function, copiedState);

	freeParam(route->function->param);
	route->function->param = copyParam(route->function->originalParam);

	free(copiedState);
	freeVarScope(route->function->function->varScope);

	freePath(path);

	return returnValue;
}

char* getValidContentType(char* extension){
	if (!strcmp(extension, ".html")) return "text/html";
	if (!strcmp(extension, ".css")) return "text/css";
	if (!strcmp(extension, ".js")) return "application/javascript";

	return "text/plain";
}

char* parseRequest(struct State* state, struct HttpRequest* request){

	if (request->method == INVALID_METHOD){
		return strdup("HTTP/1.1 400 Bad Request\r\n\r\n");
	}

	struct PathInfo* pathInfo = getValidRoute(state, request);
	if (!pathInfo->valid){
		free(pathInfo);
		return strdup("HTTP/1.1 404 Not Found\r\n\r\n");
	}

	struct Var* returnValue = parseRoute(state, pathInfo->route, request);
	struct String* responseString = (struct String*)(getType(String_c, returnValue)->type);
	char* response = responseString->cString;

	char* ok = "HTTP/1.1 200 OK\r\nContent-Type: ";
	char* contentLength = ";charset=utf-8\r\nContent-Length:";

	char* contentType = getValidContentType(state->fileExtension[0]);

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
	freeVar(returnValue);
	free(returnValue);

	//httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain;charset=utf-8\r\nContent-Length:15\r\n\r\nunicode: ååå";

	return httpResponse;
}

struct Path* generatePath(char* rawPath, unsigned int length){
	struct Path* path = (struct Path*)malloc(sizeof(struct Path));
	struct SplitPath* splitPath = getSplitPath(rawPath, length);

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

void freePath(struct Path* path){
	for (int i = 0; i < path->folderCount; i++){
		free(path->folders[i]);
	}
	free(path->folders);
	free(path);
}