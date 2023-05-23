#ifndef BUILTINS_H
#define BUILTINS_H

#include "../types/types.h"
#include "../body/functionLogic/functionLogic.h"
#include "../body/bodyTypes.h"
#include "../fileReader/fileReader.h"
#include "./route/route.h"
#include "./html/html.h"
#include "./if/if.h"

/*
Routing
*/
// ---------------

struct Route {
	struct Var* function;
	struct Path* path;
	int isStatic;
};

struct Path {
	char** folders;
	unsigned int folderCount;

	struct Var** pathVars;
	unsigned int* varIndexes;
	unsigned int varCount;
};

struct SplitPath {
	unsigned int* nextPath;
	unsigned int length;
};

struct VarLoc {
	unsigned int start;
	unsigned int end;
	unsigned int exist;
};

// ---------------

// file loading

struct UserFile{
	char* data;
	unsigned int length;
	char* path;
};

// -----------------

struct BuiltinFunction{
	char* name;
	unsigned int nameLength;
	struct Param* params;
	struct Param* originalParams;	
	struct Var*(*function)(struct Param* params, struct State* state);
};

struct Builtins{
	struct BuiltinFunction* functions;
	unsigned int numberOfFunctions;
};

void addBuiltin(struct Builtins* builtins, unsigned int index, char* name, unsigned int inputs, struct Var** inputVars, struct Var* returnValue, struct Var*(*function)(struct Param* params, struct State* state));
struct Builtins* createBuiltins();
int isBuiltin(struct Builtins* builtins, char* name);
struct BuiltinFunction* getBuiltin(struct Builtins* builtins, int index);
#endif