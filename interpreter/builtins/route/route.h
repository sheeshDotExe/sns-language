#ifndef ROUTE_H
#define ROUTE_H

#include "../../types/types.h"
#include "../../body/functionLogic/functionLogic.h"
#include "../../body/bodyTypes.h"
#include "../../fileReader/fileReader.h"
#include "../builtins.h"

void add_route(struct State* state, struct Var* function, struct Path* path, struct ProcessState* processState);
void add_static_route(struct State* state, struct Var* function, struct Path* path, struct ProcessState* processState);
struct SplitPath* get_split_path(char* path, unsigned int length, struct ProcessState* processState);
struct Path * interpret_path(struct State* state, char* path, unsigned int length, struct ProcessState* processState);
struct Var *route(struct Param *params, struct State *state, struct ProcessState* processState);
struct Var *route_static(struct Param *params, struct State *state, struct ProcessState* processState);

#endif