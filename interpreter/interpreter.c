#include "interpreter.h"

int interpret(FILE *file)
{

	struct File data = readFile(file);

	printf("compiling...\n");

	struct HeaderOptions headerOptions = getHeaderOptions(data);

	struct KeyChars keyChars = createKeyChars();

	printf("header compiled...\n");

	struct State* state = (struct State*)malloc(sizeof(struct State));
	state->keyChars = keyChars;
	state->builtins = createBuiltins();

	state->routes = (struct Routes*)malloc(sizeof(struct Routes));
	state->routes->routes = (struct Route**)malloc(0);
	state->routes->numberOfRoutes = 0;

	state->files = (struct Files*)malloc(sizeof(struct Files));
	state->files->files = (struct UserFile**)malloc(0);
	state->files->numberOfFiles = 0;

	struct Body* body = interpretBody(state, data, headerOptions.headerEnd, data.length);

	if (!body->hasMain) {
		raiseError("No entry point found (missing main function)", 1);
	}

	struct Var* mainFunction = getVarFromScope(state->globalScope, "main");

	mainFunction->function->varScope = createVarScope(mainFunction);
	struct State* copiedState = copyState(state);
	struct Var* statusCode = callFunction(mainFunction, copiedState);

	if (!strcmp(statusCode->value, "1")){
		raiseError("Main function returned with error code 1", 1);
	}

	printf("body compiled...\n");

	startHTTPServer(state, &headerOptions, &body);

	printf("server started...\n");

	printf("compiled\n");

	char buf[100];
	gets(buf);

	return 0;
}