#include "interpreter.h"

int interpret(FILE *file, struct ProcessState* processState)
{

	struct File data = readFile(file, processState);

	fclose(file);

	printf("compiling...\n");

	struct HeaderOptions* headerOptions = getHeaderOptions(data, processState);

	struct KeyChars keyChars = createKeyChars(processState);

	printf("header compiled...\n");

	struct State* state = (struct State*)malloc(sizeof(struct State));
	state->keyChars = keyChars;
	state->builtins = createBuiltins(processState);

	state->routes = (struct Routes*)malloc(sizeof(struct Routes));
	state->routes->routes = (struct Route**)malloc(0);
	state->routes->numberOfRoutes = 0;

	state->files = (struct Files*)malloc(sizeof(struct Files));
	state->files->files = (struct UserFile**)malloc(0);
	state->files->numberOfFiles = 0;

	state->inheritedVarscopes = (struct InheritedVarscopes*)malloc(sizeof(struct InheritedVarscopes));
	state->inheritedVarscopes->numberOfScopes = 0;
	state->useInheritence = 0;

	state->fileExtension = (char**)malloc(sizeof(char*));
	state->fileExtension[0] = strdup("\0");

	struct Body* body = interpretBody(state, data, headerOptions->headerEnd, data.length, processState);

	if (!body->hasMain) {
		raiseError("No entry point found (missing main function)", 1, processState);
	}

	struct Var* mainFunction = getVarFromScope(state->globalScope, "main", processState);

	mainFunction->function->varScope = createVarScope(mainFunction, processState);
	struct State* copiedState = copyState(state, processState);
	struct Var* statusCode = callFunction(mainFunction, copiedState, processState);

	if (!strcmp(statusCode->value, "1")){
		raiseError("Main function returned with error code 1", 1, processState);
	}

	printf("body compiled...\n");

	startHTTPServer(state, headerOptions, &body, processState);

	printf("server started...\n");

	printf("compiled\n");

	char buf[100];
	gets(buf);

	return 0;
}