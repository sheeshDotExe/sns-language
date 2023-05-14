#include "builtins.h"

// Route ------------------
struct Var *route(struct Param *params, struct State *state)
{
	struct Var *rawPath = params->inputVars[0];
	struct String* string = (struct String*)(getType(String_c, rawPath)->type);
	struct Path* path = interpretPath(state, string->cString, strlen(string->cString));

	struct Param* routeVars = (struct Param*)malloc(sizeof(struct Param));
	routeVars->inputCount = path->varCount;
	routeVars->inputVars = (struct Var**)malloc(path->varCount*sizeof(struct Var*));
	for (int i = 0; i < path->varCount; i++){
		routeVars->inputVars[i] = copyVar(path->pathVars[i]);
	}
	int codes[1] = {String_c};

	routeVars->returnValue = generateVar(&codes, 1, "return", "", (struct Param*)NULL);

	codes[0] = Function_c;

	struct Var* routeFunction = generateVar(&codes, 1, "route", "", routeVars);

	routeFunction->hasParam = 1;

	addRoute(state, routeFunction, path);

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
			struct Var* pathVar = getVarTypes(keyWords[0]->value, keyPositions, keyWords, keysCount, 0, &increment);

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

int routeExists(struct State* state, struct Path* path){
	return 0;
}

void addRoute(struct State* state, struct Var* function, struct Path* path){
	if (routeExists(state, path)){
		raiseError("route already exists\n", 1);
	}

	struct Route** newRoutes = (struct Route**)realloc(state->routes->routes, (state->routes->numberOfRoutes+1)*sizeof(struct Route*));

	if (newRoutes == NULL){
		raiseError("memory error on routes\n", 1);
	}

	struct Route* newRoute = (struct Route*)malloc(sizeof(struct Route));
	newRoute->function = function;
	newRoute->path = path;

	state->routes->routes = newRoutes;
	state->routes->routes[state->routes->numberOfRoutes] = newRoute;
	state->routes->numberOfRoutes++;
}
// --------------


// load html document/ data

void addUserFile(struct State* state, struct UserFile* userFile){
	struct UserFile** newUserFiles = (struct Route**)realloc(state->files->files, (state->files->numberOfFiles+1)*sizeof(struct UserFile*));

	if (newUserFiles == NULL){
		raiseError("memory error on files\n", 1);
	}

	state->files->files = newUserFiles;
	state->files->files[state->files->numberOfFiles] = userFile;
	state->files->numberOfFiles++;
}

struct UserFile* createUserFile(struct State* state, char* path){

	FILE* fileH = fopen(path, "r");
	if (fileH == NULL){
		printf("found no file: %s\n", path);
		raiseError("", 1);
	}

	struct File file = readFile(fileH);

	struct UserFile* userFile = (struct UserFile*)malloc(sizeof(struct UserFile));
	userFile->data = file.mem;
	userFile->length = file.length;
	userFile->path = strdup(path);


	return userFile;
}

struct UserFile* getUserFile(struct State* state, char* path){
	struct Files* files = state->files;

	for (int i = 0; i < files->numberOfFiles; i++){
		struct UserFile* file = files->files[i];

		if (!strcmp(file->path, path)) return file;
	}

	return NULL;
}

char* safePath(char* path){
	char* newPath = (char*)malloc(strlen(path)+1);
	int i = 0;
	for (i; i < strlen(path); i++){
		if (path[i] == '/') newPath[i] = '\\';
		else newPath[i] = path[i];
	}
	newPath[i] = '\0';

	return newPath;
}

struct Var *html(struct Param *params, struct State *state){
	struct Var* rawPath = params->inputVars[0];
	struct String* string = (struct String*)(getType(String_c, rawPath)->type);
	char* path = string->cString;

	char* relPath = safePath(path);
	char* fullPath = _fullpath(NULL, relPath, 0);

	struct UserFile* file = getUserFile(state, fullPath);

	if (file != NULL){
		return generateVarFromString(file->data, file->length);
	}

	file = createUserFile(state, fullPath);

	free(relPath);
	free(fullPath);

	return generateVarFromString(file->data, file->length);
}

// ------------------------

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
	builtins->numberOfFunctions = 2;
	builtins->functions = (struct BuiltinFunction*)malloc(sizeof(struct BuiltinFunction) * builtins->numberOfFunctions);

	struct Var** inputVars;
	struct Var* returnValue;
	int types[NUMBER_OF_TYPES];

	// route builtin
	inputVars = (struct Var**)malloc(sizeof(struct Var*) * 1);
	types[0] = String_c;
	inputVars[0] = generateVar(&types, 1, (char*)"path", "", NULL);
	returnValue = generateVar(NULL, 0, "return", "", NULL);
	addBuiltin(builtins, 0, strdup("route"), 1, inputVars, returnValue, &route);

	// html builtin
	inputVars = (struct Var**)malloc(sizeof(struct Var*) * 1);
	types[0] = String_c;
	inputVars[0] = generateVar(&types, 1, "path", "", NULL);
	returnValue = generateVar(NULL, 0, "return", "", NULL);
	addBuiltin(builtins, 1, strdup("html"), 1, inputVars, returnValue, &html);

	return builtins;
}