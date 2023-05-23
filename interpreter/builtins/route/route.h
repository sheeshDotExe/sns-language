#ifndef ROUTE_H
#define ROUTE_H

#include "../../types/types.h"
#include "../../body/functionLogic/functionLogic.h"
#include "../../body/bodyTypes.h"
#include "../../fileReader/fileReader.h"
#include "../builtins.h"

void addRoute(struct State* state, struct Var* function, struct Path* path);
void addStaticRoute(struct State* state, struct Var* function, struct Path* path);
struct SplitPath* getSplitPath(char* path, unsigned int length);
struct Path * interpretPath(struct State* state, char* path, unsigned int length);
struct Var *route(struct Param *params, struct State *state);
struct Var *route_static(struct Param *params, struct State *state);

#endif