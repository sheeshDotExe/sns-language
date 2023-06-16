#include "interpreter.h"

int interpret(FILE *file, struct ProcessState* processState)
{

	struct File data = read_file(file, processState);

	fclose(file);

	printf("compiling...\n");

	struct HeaderOptions* headerOptions = get_header_options(data, processState);

	struct KeyChars keyChars = create_key_chars(processState);

	printf("header compiled...\n");

	struct State* state = (struct State*)malloc(sizeof(struct State));
	state->keyChars = keyChars;
	state->builtins = create_builtins(processState);

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

	struct Body* body = interpret_body(state, data, headerOptions->headerEnd, data.length, processState);

	if (!body->hasMain) {
		raise_error("No entry point found (missing main function)", 1, processState);
	}

	struct Var* mainFunction = get_var_from_scope(state->globalScope, "main", processState);

	mainFunction->function->varScope = create_var_scope(mainFunction, processState);
	struct State* copiedState = copy_state(state, processState);
	struct Var* statusCode = call_function(mainFunction, copiedState, processState);

	if (!strcmp(statusCode->value, "1")){
		raise_error("Main function returned with error code 1", 1, processState);
	}

	printf("body compiled...\n");

	start_HTTP_server(state, headerOptions, &body, processState);

	printf("server started...\n");

	printf("compiled\n");

	char buf[100];
	gets(buf);

	return 0;
}