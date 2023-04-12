#ifndef BUILTINS_H
#define BUILTINS_H

#include "../types/types.h"
#include "../body/functionLogic/functionLogic.h"
#include "../body/bodyTypes.h"

struct BuiltinFunction{
	char * name;
	unsigned int nameLength;
	struct Param* params;	
	struct Var*(*function)(struct Param* params, struct State* state);
};

struct Builtins{
	struct BuiltinFunction* functions;
	unsigned int numberOfFunctions;
};

void addBuiltin(struct Builtins* builtins, unsigned int index, char* name, unsigned int inputs, struct Var** inputVars, struct Var*(*function)(struct Param* params, struct State* state));
struct Builtins* createBuiltins();
int isBuiltin(struct Builtins* builtins, char* name);
struct BuiltinFunction* getBuiltin(struct Builtins* builtins, int index);
#endif