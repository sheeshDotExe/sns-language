#include "httpParser.h"

#ifndef __unix__
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep(x * 1000)
#endif

struct ThreadInfo *createThreadInfo(struct State *state, struct HeaderOptions *headerOptions, struct RequestQueue *requestQueue, pthread_mutex_t *lock)
{
	struct ThreadInfo *threadInfo = (struct ThreadInfo *)malloc(sizeof(struct ThreadInfo));
	threadInfo->headerOptions = headerOptions;
	threadInfo->state = state;
	threadInfo->processState = (struct ProcessState *)malloc(sizeof(struct ProcessState));
	threadInfo->processState->running = 1;
	threadInfo->requestQueue = requestQueue;
	threadInfo->lock = lock;
	return threadInfo;
}

void startHTTPServer(struct State *state, struct HeaderOptions *headerOptions, struct Body *body, struct ProcessState *processState)
{

#ifndef __unix__
	if (initWinsock(processState))
	{
		raiseError("winsock init failed", 1, processState);
	}
#endif

	initSSL(processState);

	struct Server *server = createServer(headerOptions, processState);
	printf("server started\n");

	while (1)
	{
		struct Client *client = getClient(server, headerOptions, processState);

		if (client == NULL)
		{
			freeSocket(client, headerOptions, processState);
			continue;
		}

		if (!client->valid)
		{
#ifndef __unix__
			printf("error %d\n", WSAGetLastError());
#else
			printf("error %d\n", errno);
#endif
		}
		freeSocket(client, headerOptions);
	}
}

int handleRequest(struct Client *client, struct State *state, struct HeaderOptions *headerOptions, struct ProcessState *processState)
{

	time_t currentTime = time(0);
	char *localTime = ctime(&currentTime);
	char *timeFormatted = strtok(localTime, "\n");
	printf("\n\n[%s] IP: %s\n", timeFormatted, getClientIP(client, processState));
	// free(localTime);

	struct HttpRequest *httpRequest = recive(client, headerOptions, processState);

	char *response = parseRequest(state, httpRequest, processState);

	sendData(client, response, headerOptions, processState);

	free(response);

	free(httpRequest->path);
	free(httpRequest);

	freeSocket(client, headerOptions, processState);

	return 0;
}

int isValidRoute(struct Path *path, struct Route *route, struct ProcessState *processState)
{

	struct Path *serverPath = route->path;

	if (!route->isStatic && path->folderCount != serverPath->folderCount)
		return 0;

	unsigned int currentVarIndex = 0;

	for (int i = 0; i < serverPath->folderCount; i++)
	{
		if (!serverPath->varCount || serverPath->varIndexes[currentVarIndex] != i)
		{
			if (strcmp(path->folders[i], serverPath->folders[i]))
				return 0;
		}
		else if (serverPath->varCount && serverPath->varIndexes[currentVarIndex] == i)
		{
			if (route->isStatic)
				continue; // var type doesn't matter for static files

			struct Var *pathVar = serverPath->pathVars[currentVarIndex];

			struct Var *otherVar = generateVarFromString(path->folders[i], strlen(path->folders[i]), processState);

			struct CommonTypes commonTypes = getCommonTypes(pathVar, otherVar, processState);
			free(commonTypes.codes);

			freeVar(otherVar, processState);
			free(otherVar);

			if (!commonTypes.length)
				return 0;

			currentVarIndex++;
		}
	}

	return 1;
}

struct PathInfo *getValidRoute(struct State *state, struct HttpRequest *request, struct ProcessState *processState)
{

	struct PathInfo *pathInfo = (struct PathInfo *)malloc(sizeof(struct PathInfo));
	pathInfo->valid = 0;

	struct Path *path = generatePath(request->path, strlen(request->path), processState); // sus strlen

	struct Routes *routes = state->routes;

	for (int i = 0; i < routes->numberOfRoutes; i++)
	{
		struct Route *route = routes->routes[i];

		int check = isValidRoute(path, route, processState);

		if (!check)
			continue;

		pathInfo->valid = 1;
		pathInfo->route = route;

		freePath(path, processState);

		return pathInfo;
	}

	freePath(path, processState);

	return pathInfo;
}

struct Var *parseRoute(struct State *state, struct Route *route, struct HttpRequest *request, struct ProcessState *processState)
{
	struct Path *path = generatePath(request->path, strlen(request->path), processState); // sus strlen

	struct Path *serverPath = route->path;

	unsigned int currentVarIndex = 0;

	for (int i = 0; i < serverPath->folderCount; i++)
	{

		if (serverPath->varCount && serverPath->varIndexes[currentVarIndex] == i)
		{
			struct Var *pathVar = serverPath->pathVars[currentVarIndex];
			struct Param *param = route->function->param;
			struct Var *paramVar = param->inputVars[currentVarIndex];

			struct Var *otherVar = NULL;
			if (!route->isStatic)
				otherVar = generateVarFromString(path->folders[i], strlen(path->folders[i]), processState);
			else
				otherVar = generateVarFromString(request->path, strlen(request->path), processState);

			assignValue(paramVar, otherVar, processState);

			freeVar(otherVar, processState);
			free(otherVar);

			currentVarIndex++;
		}
	}

	route->function->function->varScope = createVarScope(route->function, processState);
	struct State *copiedState = copyState(state, processState);
	struct Var *returnValue = callFunction(route->function, copiedState, processState);

	freeParam(route->function->param, processState);
	route->function->param = copyParam(route->function->originalParam, processState);

	free(copiedState);
	freeVarScope(route->function->function->varScope, processState);

	freePath(path, processState);

	return returnValue;
}

char *getValidContentType(char *extension, struct ProcessState *processState)
{
	if (!strcmp(extension, ".html"))
		return "text/html";
	if (!strcmp(extension, ".css"))
		return "text/css";
	if (!strcmp(extension, ".js"))
		return "application/javascript";

	return "text/plain";
}

char *parseRequest(struct State *state, struct HttpRequest *request, struct ProcessState *processState)
{

	if (request->method == INVALID_METHOD)
	{
		return strdup("HTTP/1.1 400 Bad Request\r\n\r\n");
	}

	struct PathInfo *pathInfo = getValidRoute(state, request, processState);
	if (!pathInfo->valid)
	{
		free(pathInfo);
		return strdup("HTTP/1.1 404 Not Found\r\n\r\n");
	}

	struct Var *returnValue = parseRoute(state, pathInfo->route, request, processState);
	struct String *responseString = (struct String *)(getType(String_c, returnValue, processState)->type);
	char *response = responseString->cString;

	char *ok = "HTTP/1.1 200 OK\r\nContent-Type: ";
	char *contentLength = ";charset=utf-8\r\nContent-Length: ";

	char *contentType = getValidContentType(state->fileExtension[0], processState);

	char *length = (char *)malloc(100);
	sprintf(length, "%d", strlen(response));

	int responseLength = strlen(response) + strlen(ok) + strlen(contentLength) + strlen(length) + strlen(contentType) + 4 + 1;

	char *httpResponse = (char *)malloc(responseLength * sizeof(char));

	strcpy(httpResponse, ok);
	strcat(httpResponse, contentType);
	strcat(httpResponse, contentLength);
	strcat(httpResponse, length);
	strcat(httpResponse, "\r\n\r\n");
	strcat(httpResponse, response);
	httpResponse[responseLength - 1] = '\0';

	free(length);
	free(pathInfo);
	freeVar(returnValue, processState);
	free(returnValue);

	// httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain;charset=utf-8\r\nContent-Length:15\r\n\r\nunicode: ååå";

	return httpResponse;
}

struct Path *generatePath(char *rawPath, unsigned int length, struct ProcessState *processState)
{
	struct Path *path = (struct Path *)malloc(sizeof(struct Path));
	struct SplitPath *splitPath = getSplitPath(rawPath, length, processState);

	path->folders = (char **)malloc((splitPath->length - 1) * sizeof(char *));
	path->folderCount = splitPath->length - 1;

	for (int i = 0; i < splitPath->length - 1; i++)
	{
		unsigned int size = splitPath->nextPath[i + 1] - splitPath->nextPath[i];
		path->folders[i] = (char *)malloc((size) * sizeof(char));
		memcpy(path->folders[i], rawPath + splitPath->nextPath[i] + 1, (size - 1) * sizeof(char));
		path->folders[i][size - 1] = '\0';
	}

	free(splitPath->nextPath);
	free(splitPath);

	return path;
}

void freePath(struct Path *path, struct ProcessState *processState)
{
	for (int i = 0; i < path->folderCount; i++)
	{
		free(path->folders[i]);
	}
	free(path->folders);
	free(path);
}