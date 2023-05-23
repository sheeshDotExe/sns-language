#include "builtins.h"

int isBuiltin(struct Builtins *builtins, char* name)
{
	for (int i = 0; i < builtins->numberOfFunctions; i++)
	{
		if (!strcmp(builtins->functions[i].name, name))
		{
			return i + 1;
		}
	}
	return 0;
}

struct BuiltinFunction *getBuiltin(struct Builtins *builtins, int index)
{
	return &builtins->functions[index - 1];
}

void addBuiltin(struct Builtins *builtins, unsigned int index, char *name, unsigned int inputs, struct Var **inputVars, struct Var* returnValue, struct Var *(*function)(struct Param *params, struct State *state))
{
	builtins->functions[index].function = function;
	builtins->functions[index].name = name;
	builtins->functions[index].nameLength = strlen(name);
	builtins->functions[index].params = (struct Param *)malloc(sizeof(struct Param));
	builtins->functions[index].params->inputCount = inputs;
	builtins->functions[index].params->inputVars = inputVars;
	builtins->functions[index].params->returnValue = returnValue;
	builtins->functions[index].originalParams = copyParam(builtins->functions[index].params);
}

struct Builtins* createBuiltins()
{
	struct Builtins* builtins = (struct Builtins*)malloc(sizeof(struct Builtins));
	builtins->numberOfFunctions = 5;
	builtins->functions = (struct BuiltinFunction*)malloc(sizeof(struct BuiltinFunction) * builtins->numberOfFunctions);

	struct Var** inputVars;
	struct Var* returnValue;
	int types[NUMBER_OF_TYPES];

	// route builtin
	inputVars = (struct Var**)malloc(sizeof(struct Var*) * 1);
	types[0] = String_c;
	inputVars[0] = generateVar(&types, 1, "path", "", NULL);
	returnValue = generateVar(NULL, 0, "return", "", NULL);
	addBuiltin(builtins, 0, strdup("route"), 1, inputVars, returnValue, &route);

	inputVars = (struct Var**)malloc(sizeof(struct Var*) * 1);
	types[0] = String_c;
	inputVars[0] = generateVar(&types, 1, "path", "", NULL);
	returnValue = generateVar(NULL, 0, "return", "", NULL);
	addBuiltin(builtins, 1, strdup("route_static"), 1, inputVars, returnValue, &route_static);

	// html builtin
	inputVars = (struct Var**)malloc(sizeof(struct Var*) * 1);
	types[0] = String_c;
	inputVars[0] = generateVar(&types, 1, "path", "", NULL);
	returnValue = generateVar(NULL, 0, "return", "", NULL);
	addBuiltin(builtins, 2, strdup("html"), 1, inputVars, returnValue, &html);

	// if/else builtin
	inputVars = (struct Var**)malloc(sizeof(struct Var*) * 1);
	types[0] = String_c;
	types[1] = Int_c;
	types[2] = Float_c;
	inputVars[0] = generateVar(&types, 3, "condition", "", NULL);
	returnValue = generateVar(NULL, 0, "return", "", NULL);
	addBuiltin(builtins, 3, strdup("if"), 1, inputVars, returnValue, &if_implementation);

	inputVars = (struct Var**)malloc(0);
	returnValue = generateVar(NULL, 0, "return", "", NULL);
	addBuiltin(builtins, 4, strdup("else"), 0, inputVars, returnValue, &else_implementation);

	return builtins;
}