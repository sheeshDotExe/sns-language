#include "interpreter.h"

int interpret(FILE *file)
{

	struct File data = readFile(file);

	printf("compiling...\n");

	struct HeaderOptions headerOptions = getHeaderOptions(data);

	struct KeyChars keyChars = createKeyChars();

	struct Body body = interpretBody(keyChars, data, headerOptions.headerEnd, data.length);

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