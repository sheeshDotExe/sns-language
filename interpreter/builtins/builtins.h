#ifndef BUILTINS_H
#define BUILTINS_H

#include "../types/types.h"
#include "../body/functionLogic/functionLogic.h"
#include "../body/bodyTypes.h"

struct BuiltinFunction{
	char * name;
	unsigned int nameLength;
	struct Var*(*function)(struct Param* params, struct State* state);
};

struct Builtins{
	struct BuiltinFunction* functions;
};

struct Builtins* createBuiltins();
int isBuiltin(struct Builtins* Builtins);
#endif