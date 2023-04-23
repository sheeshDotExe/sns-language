#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "../header/findHeader.h"
#include "../types/types.h"
#include "../body/interpretBody.h"
#include "../body/bodyTypes.h"
#include "../builtins/builtins.h"
#include "httpLibrary/httpLibrary.h"

void startHTTPServer(struct State* state, struct HeaderOptions* headerOptions, struct Body* body);

#endif