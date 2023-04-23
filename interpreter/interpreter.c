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

	struct Body body = interpretBody(state, data, headerOptions.headerEnd, data.length);

	printf("body compiled...\n");

	startHTTPServer(state, &headerOptions, &body);

	printf("server started...\n");

	printf("compiled\n");

	char buf[100];
	gets(buf);

	return 0;
}