#include "route.h"

// Route ------------------
struct Var *route(struct Param *params, struct State *state, struct ProcessState* processState)
{
	struct Var *rawPath = params->inputVars[0];
	struct String* string = (struct String*)(getType(String_c, rawPath, processState)->type);
	struct Path* path = interpretPath(state, string->cString, strlen(string->cString), processState);

	struct Param* routeVars = (struct Param*)malloc(sizeof(struct Param));
	routeVars->inputCount = path->varCount;
	routeVars->inputVars = (struct Var**)malloc(path->varCount*sizeof(struct Var*));
	for (int i = 0; i < path->varCount; i++){
		routeVars->inputVars[i] = copyVar(path->pathVars[i], processState);
	}
	int codes[1] = {String_c};

	routeVars->returnValue = generateVar(&codes, 1, "return", "", (struct Param*)NULL, processState);

	codes[0] = Function_c;

	struct Var* routeFunction = generateVar(&codes, 1, "route", "", routeVars, processState);

	routeFunction->hasParam = 1;
	routeFunction->assignable = 1;
	routeFunction->inheritScopes = 1;

	addRoute(state, routeFunction, path, processState);

	return routeFunction;
}

struct Var*route_static(struct Param* params, struct State* state, struct ProcessState* processState)
{
	struct Var *rawPath = params->inputVars[0];
	struct String* string = (struct String*)(getType(String_c, rawPath, processState)->type);
	struct Path* path = interpretPath(state, string->cString, strlen(string->cString), processState);

	struct Param* routeVars = (struct Param*)malloc(sizeof(struct Param));
	routeVars->inputCount = path->varCount;
	routeVars->inputVars = (struct Var**)malloc(path->varCount*sizeof(struct Var*));
	for (int i = 0; i < path->varCount; i++){
		routeVars->inputVars[i] = copyVar(path->pathVars[i], processState);
	}
	int codes[1] = {String_c};

	routeVars->returnValue = generateVar(&codes, 1, "return", "", (struct Param*)NULL, processState);

	codes[0] = Function_c;

	struct Var* routeFunction = generateVar(&codes, 1, "route", "", routeVars, processState);

	routeFunction->hasParam = 1;
	routeFunction->assignable = 1;
	routeFunction->inheritScopes = 1;

	addStaticRoute(state, routeFunction, path, processState);

	return routeFunction;
}

struct SplitPath* getSplitPath(char* path, unsigned int length, struct ProcessState* processState){
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

struct VarLoc getPathVar(char* folder, unsigned int length, struct ProcessState* processState){
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

struct Path * interpretPath(struct State* state, char* rawPath, unsigned int length, struct ProcessState* processState){
	struct Path* path = (struct Path*)malloc(sizeof(struct Path));
	struct SplitPath* splitPath = getSplitPath(rawPath, length, processState);

	path->folders = (char**)malloc((splitPath->length-1) * sizeof(char*));
	path->folderCount = splitPath->length-1;

	for (int i = 0; i < splitPath->length-1; i++){
		unsigned int size = splitPath->nextPath[i+1] - splitPath->nextPath[i];
		path->folders[i] = (char*)malloc((size) * sizeof(char));
		memcpy(path->folders[i], rawPath + splitPath->nextPath[i] + 1, (size - 1)*sizeof(char));
		path->folders[i][size - 1] = '\0';
	}

	unsigned int varCount = 0;

	for (int i = 0; i < splitPath->length-1; i++){
		struct VarLoc varLoc = getPathVar(path->folders[i], strlen(path->folders[i]), processState);
		varCount += varLoc.exist;
	}

	path->pathVars = (struct Var**)malloc(varCount*sizeof(struct Var*));
	path->varCount = varCount;
	path->varIndexes = (unsigned int*)malloc(varCount*sizeof(unsigned int));

	unsigned int index = 0;
	for (int i = 0; i < splitPath->length-1; i++){
		unsigned int pathLength = strlen(path->folders[i]);
		struct VarLoc varLoc = getPathVar(path->folders[i], pathLength, processState);
		if (varLoc.exist){
			//printf("path var %s\n", path->folders[i]);
			path->varIndexes[index] = i;

			unsigned int keysCount = getKeysCount(state, path->folders[i] + 1, pathLength - 2, 0, processState);
			//printf("%d\n", keysCount);
	
			struct KeyPos** keyPositions = getKeyPositions(keysCount, state, path->folders[i] + 1, pathLength - 2, 0, processState);

			struct KeyWord** keyWords = getKeyWords(keysCount, keyPositions, state, path->folders[i] + 1, pathLength - 2, processState);
			
			unsigned int increment = 0;
			struct Var* pathVar = getVarTypes(keyWords[0]->value, keyPositions, keyWords, keysCount, 0, &increment, processState);

			//printf("var name %s\n", pathVar->name);
			freeLines(keyPositions, keyWords, keysCount, processState);

			path->pathVars[index] = pathVar;
			
			index++;
		}
	}

	free(splitPath->nextPath);
	free(splitPath);

	return path;
}

int routeExists(struct State* state, struct Path* path, struct ProcessState* processState){
	return 0;
}

void addRoute(struct State* state, struct Var* function, struct Path* path, struct ProcessState* processState){
	if (routeExists(state, path, processState)){
		raiseError("route already exists\n", 1, processState);
	}

	struct Route** newRoutes = (struct Route**)realloc(state->routes->routes, (state->routes->numberOfRoutes+1)*sizeof(struct Route*));

	if (newRoutes == NULL){
		raiseError("memory error on routes\n", 1, processState);
	}

	struct Route* newRoute = (struct Route*)malloc(sizeof(struct Route));
	newRoute->function = function;
	newRoute->path = path;
	newRoute->isStatic = 0;

	state->routes->routes = newRoutes;
	state->routes->routes[state->routes->numberOfRoutes] = newRoute;
	state->routes->numberOfRoutes++;
}

void addStaticRoute(struct State* state, struct Var* function, struct Path* path, struct ProcessState* processState){
	if (routeExists(state, path, processState)){
		raiseError("route already exists\n", 1, processState);
	}

	struct Route** newRoutes = (struct Route**)realloc(state->routes->routes, (state->routes->numberOfRoutes+1)*sizeof(struct Route*));

	if (newRoutes == NULL){
		raiseError("memory error on routes\n", 1, processState);
	}

	struct Route* newRoute = (struct Route*)malloc(sizeof(struct Route));
	newRoute->function = function;
	newRoute->path = path;
	newRoute->isStatic = 1;

	state->routes->routes = newRoutes;
	state->routes->routes[state->routes->numberOfRoutes] = newRoute;
	state->routes->numberOfRoutes++;
}
// --------------