#include "builtins.h"

struct Var *route(struct Param *params, struct State *state)
{
	struct Var *path = params->inputVars[0];
	printf("test name %s\n", path->name);
	struct String* string = (struct String*)(getType(String_c, path)->type);
	printf("test type %d %s %s\n", path->types[0].code, ((struct String*)path->types[0].type)->cString, string->cString);
	//struct String* string = (struct String*)getType(String_c, path);
	//printf("test %s\n", string->cString);
	return NULL;
}

int isBuiltin(struct Builtins *builtins, char *name)
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

void addBuiltin(struct Builtins *builtins, unsigned int index, char *name, unsigned int inputs, struct Var **inputVars, struct Var *(*function)(struct Param *params, struct State *state))
{
	builtins->functions[index].function = function;
	builtins->functions[index].name = name;
	builtins->functions[index].nameLength = strlen(name);
	builtins->functions[index].params = (struct Param *)malloc(sizeof(struct Param));
	builtins->functions[index].params->inputCount = inputs;
	builtins->functions[index].params->inputVars = inputVars; // yoink
}

struct Builtins *createBuiltins()
{
	struct Builtins *builtins = (struct Builtins *)malloc(sizeof(struct Builtins));
	builtins->numberOfFunctions = 1;
	builtins->functions = (struct BuiltinFunction *)malloc(sizeof(struct BuiltinFunction) * builtins->numberOfFunctions);

	struct Var **inputVars;
	int types[NUMBER_OF_TYPES];

	// route builtin
	inputVars = (struct Var **)malloc(sizeof(struct Var *) * 1);
	types[0] = String_c;
	inputVars[0] = generateVar(&types, 1, (char *)"path", "", NULL);
	addBuiltin(builtins, 0, (char *)"route", 1, inputVars, &route);

	return builtins;
}