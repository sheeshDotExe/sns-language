#include "route.h"

// Route ------------------
struct Var *route(struct Param *params, struct State *state, struct ProcessState* processState)
{
	struct Var *rawPath = params->inputVars[0];
	struct String* string = (struct String*)(get_type(String_c, rawPath, processState)->type);
	struct Path* path = interpret_path(state, string->cString, strlen(string->cString), processState);

	struct Param* routeVars = (struct Param*)malloc(sizeof(struct Param));
	routeVars->inputCount = path->varCount;
	routeVars->inputVars = (struct Var**)malloc(path->varCount*sizeof(struct Var*));
	for (int i = 0; i < path->varCount; i++){
		routeVars->inputVars[i] = copy_var(path->pathVars[i], processState);
	}
	int codes[1] = {String_c};

	routeVars->returnValue = generate_var(&codes, 1, "return", "", (struct Param*)NULL, processState);

	codes[0] = Function_c;

	struct Var* routeFunction = generate_var(&codes, 1, "route", "", routeVars, processState);

	routeFunction->hasParam = 1;
	routeFunction->assignable = 1;
	routeFunction->inheritScopes = 1;

	add_route(state, routeFunction, path, processState);

	return routeFunction;
}

struct Var*route_static(struct Param* params, struct State* state, struct ProcessState* processState)
{
	struct Var *rawPath = params->inputVars[0];
	struct String* string = (struct String*)(get_type(String_c, rawPath, processState)->type);
	struct Path* path = interpret_path(state, string->cString, strlen(string->cString), processState);

	struct Param* routeVars = (struct Param*)malloc(sizeof(struct Param));
	routeVars->inputCount = path->varCount;
	routeVars->inputVars = (struct Var**)malloc(path->varCount*sizeof(struct Var*));
	for (int i = 0; i < path->varCount; i++){
		routeVars->inputVars[i] = copy_var(path->pathVars[i], processState);
	}
	int codes[1] = {String_c};

	routeVars->returnValue = generate_var(&codes, 1, "return", "", (struct Param*)NULL, processState);

	codes[0] = Function_c;

	struct Var* routeFunction = generate_var(&codes, 1, "route", "", routeVars, processState);

	routeFunction->hasParam = 1;
	routeFunction->assignable = 1;
	routeFunction->inheritScopes = 1;

	add_static_route(state, routeFunction, path, processState);

	return routeFunction;
}

struct SplitPath* get_split_path(char* path, unsigned int length, struct ProcessState* processState){
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

struct VarLoc get_path_var(char* folder, unsigned int length, struct ProcessState* processState){
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

struct Path * interpret_path(struct State* state, char* rawPath, unsigned int length, struct ProcessState* processState){
	struct Path* path = (struct Path*)malloc(sizeof(struct Path));
	struct SplitPath* splitPath = get_split_path(rawPath, length, processState);

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
		struct VarLoc varLoc = get_path_var(path->folders[i], strlen(path->folders[i]), processState);
		varCount += varLoc.exist;
	}

	path->pathVars = (struct Var**)malloc(varCount*sizeof(struct Var*));
	path->varCount = varCount;
	path->varIndexes = (unsigned int*)malloc(varCount*sizeof(unsigned int));

	unsigned int index = 0;
	for (int i = 0; i < splitPath->length-1; i++){
		unsigned int pathLength = strlen(path->folders[i]);
		struct VarLoc varLoc = get_path_var(path->folders[i], pathLength, processState);
		if (varLoc.exist){
			//printf("path var %s\n", path->folders[i]);
			path->varIndexes[index] = i;

			unsigned int keysCount = get_keys_count(state, path->folders[i] + 1, pathLength - 2, 0, processState);
			//printf("%d\n", keysCount);
	
			struct KeyPos** keyPositions = get_key_positions(keysCount, state, path->folders[i] + 1, pathLength - 2, 0, processState);

			struct KeyWord** keyWords = get_key_words(keysCount, keyPositions, state, path->folders[i] + 1, pathLength - 2, processState);
			
			unsigned int increment = 0;
			struct Var* pathVar = get_var_types(keyWords[0]->value, keyPositions, keyWords, keysCount, 0, &increment, processState);

			//printf("var name %s\n", pathVar->name);
			free_lines(keyPositions, keyWords, keysCount, processState);

			path->pathVars[index] = pathVar;
			
			index++;
		}
	}

	free(splitPath->nextPath);
	free(splitPath);

	return path;
}

int route_exists(struct State* state, struct Path* path, struct ProcessState* processState){
	return 0;
}

void add_route(struct State* state, struct Var* function, struct Path* path, struct ProcessState* processState){
	if (route_exists(state, path, processState)){
		raise_error("route already exists\n", 1, processState);
	}

	struct Route** newRoutes = (struct Route**)realloc(state->routes->routes, (state->routes->numberOfRoutes+1)*sizeof(struct Route*));

	if (newRoutes == NULL){
		raise_error("memory error on routes\n", 1, processState);
	}

	struct Route* newRoute = (struct Route*)malloc(sizeof(struct Route));
	newRoute->function = function;
	newRoute->path = path;
	newRoute->isStatic = 0;

	state->routes->routes = newRoutes;
	state->routes->routes[state->routes->numberOfRoutes] = newRoute;
	state->routes->numberOfRoutes++;
}

void add_static_route(struct State* state, struct Var* function, struct Path* path, struct ProcessState* processState){
	if (route_exists(state, path, processState)){
		raise_error("route already exists\n", 1, processState);
	}

	struct Route** newRoutes = (struct Route**)realloc(state->routes->routes, (state->routes->numberOfRoutes+1)*sizeof(struct Route*));

	if (newRoutes == NULL){
		raise_error("memory error on routes\n", 1, processState);
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