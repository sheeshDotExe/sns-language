#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "../header/findHeader.h"
#include "../types/types.h"
#include "../body/interpretBody.h"
#include "../body/bodyTypes.h"
#include "../builtins/builtins.h"
#include "httpLibrary/httpLibrary.h"

struct PathInfo {
	int valid;
	struct Route* route;
};

void startHTTPServer(struct State* state, struct HeaderOptions* headerOptions, struct Body* body);
char* parseRequest(struct State* state, struct HttpRequest* request);
struct Path* generatePath(char* rawPath, unsigned int length);
void freePath(struct Path* path);

#endif