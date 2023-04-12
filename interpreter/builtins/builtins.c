#include "builtins.h"

struct Var* route(struct Param* params, struct State* state){
	printf("test %s\n", state->globalScope->vars[0]->value);
	return NULL;
}

void addBuiltin(struct Builtins* builtins, unsigned int index, char* name, unsigned int inputs, struct Var*(*function)(struct Param* params, struct State* state)){
	builtins->functions[index].function = function;
	builtins->functions[index].name = name;
	builtins->functions[index].nameLength = strlen(name);
	builtins->functions[index].params = (struct Param*)malloc(sizeof(struct Param));
	builtins->functions[index].params->inputCount = inputs;
}

struct Builtins* createBuiltins(){
	struct Builtins* builtins = (struct Builtins*)malloc(sizeof(struct Builtins));
	builtins->numberOfFunctions = 1;
	builtins->functions = (struct BuiltinFunction*)malloc(sizeof(struct BuiltinFunction)*builtins->numberOfFunctions);

	addBuiltin(builtins, 0, (char*)"route", &route);

	return builtins;
}