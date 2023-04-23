#include "builtins.h"

// Route ------------------
struct Var *route(struct Param *params, struct State *state)
{
	struct Var *rawPath = params->inputVars[0];
	printf("test name %s\n", rawPath->name);
	struct String* string = (struct String*)(getType(String_c, rawPath)->type);
	printf("test type %d %s %s\n", rawPath->types[0].code, ((struct String*)rawPath->types[0].type)->cString, string->cString);
	//struct String* string = (struct String*)getType(String_c, path);
	//printf("test %s\n", string->cString);

	struct Path* path = interpretPath(state, string->cString, strlen(string->cString));

	struct Param* routeVars = (struct Param*)malloc(sizeof(struct Param));
	routeVars->inputCount = path->varCount;
	routeVars->inputVars = (struct Var**)malloc(path->varCount*sizeof(struct Var*));
	for (int i = 0; i < path->varCount; i++){
		routeVars->inputVars[i] = path->pathVars[i];
	}
	int codes[1] = {String_c};

	routeVars->returnValue = generateVar(&codes, 1, (char*)"return", "", (struct Param*)NULL);

	codes[0] = Function_c;

	struct Var* routeFunction = generateVar(&codes, 1, (char*)"route", "", routeVars);

	routeFunction->hasParam = 1;

	//struct returnVar = generateVar()

	return routeFunction;
}

struct SplitPath* getSplitPath(char* path, unsigned int length){
	struct SplitPath* splitPath = (struct SplitPath*)malloc(sizeof(struct SplitPath));
	unsigned int count = 0;
	for (unsigned int i = 0; i < length; i++){
		if (path[i] == '\\' || path[i] == '/'){
			count++;
		}
	}

	splitPath->length = count+1;
	splitPath->nextPath = (unsigned int*)malloc(splitPath->length*sizeof(unsigned int));

	unsigned int index = 0;
	for (unsigned int i = 0; i < length; i++){
		if (path[i] == '\\' || path[i] == '/'){
			splitPath->nextPath[index] = i;
			index++;
		}
	}
	splitPath->nextPath[index] = length;

	return splitPath;
}

struct VarLoc getPathVar(char* folder, unsigned int length){
	struct VarLoc varLoc;
	varLoc.exist = 0;
	for (int i = 0; i < length; i++){
		if (folder[i] == '{'){
			varLoc.start = i;
			varLoc.exist = 1;
		}
		else if (folder[i] == '}'){
			varLoc.end = i;
		}
	}

	return varLoc; 
}

struct Path * interpretPath(struct State* state, char* rawPath, unsigned int length){
	struct Path* path = (struct Path*)malloc(sizeof(struct Path));
	struct SplitPath* splitPath = getSplitPath(rawPath, length);

	path->folders = (char**)malloc((splitPath->length-1) * sizeof(char*));
	path->folderCount = splitPath->length-1;

	for (int i = 0; i < splitPath->length-1; i++){
		unsigned int size = splitPath->nextPath[i+1] - splitPath->nextPath[i];
		path->folders[i] = (char*)malloc((size-1) * sizeof(char));
		memcpy(path->folders[i], rawPath + splitPath->nextPath[i] + 1, size - 1);
		path->folders[i][size-1] = '\0';
	}

	unsigned int varCount = 0;

	for (int i = 0; i < splitPath->length-1; i++){
		printf("path folder %s\n", path->folders[i]);
		struct VarLoc varLoc = getPathVar(path->folders[i], strlen(path->folders[i]));
		varCount += varLoc.exist;
	}

	path->pathVars = (struct Var**)malloc(varCount*sizeof(struct Var*));
	path->varCount = varCount;
	path->varIndexes = (unsigned int*)malloc(varCount*sizeof(unsigned int));

	unsigned int index = 0;
	for (int i = 0; i < splitPath->length-1; i++){
		unsigned int pathLength = strlen(path->folders[i]);
		struct VarLoc varLoc = getPathVar(path->folders[i], pathLength);
		if (varLoc.exist){
			//printf("path var %s\n", path->folders[i]);
			path->varIndexes[index] = i;

			unsigned int keysCount = getKeysCount(state, path->folders[i] + 1, pathLength - 2, 0);

			//printf("%d\n", keysCount);
	
			struct KeyPos** keyPositions = getKeyPositions(keysCount, state, path->folders[i] + 1, pathLength - 2, 0);

			struct KeyWord** keyWords = getKeyWords(keysCount, keyPositions, state, path->folders[i] + 1, pathLength - 2);
			
			unsigned int increment = 0;
			struct Var* pathVar = getVarTypes(keyWords[0]->value, keyPositions, keyWords, keysCount, i, &increment);

			//printf("var name %s\n", pathVar->name);
			freeLines(keyPositions, keyWords, keysCount);

			path->pathVars[index] = pathVar;
			
			index++;
		}
	}

	free(splitPath->nextPath);
	free(splitPath);

	return path;
}

// --------------

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