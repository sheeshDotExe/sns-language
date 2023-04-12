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

	state->builtins->functions[0].function(NULL, state);

	// printf("%s", data);
	// testVar();

	/*
	struct Var var = generateVarFromString("\"100\"", 5);

	printf("%d\n", var.numberOfTypes);

	freeVar(&var);
	*/

	printf("compiled\n");

	char buf[100];
	gets(buf);

	return 0;
}