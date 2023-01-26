#include "interpreter.h"

int interpret(FILE*file){

	struct File data = readFile(file);

	printf("compiling...\n");

	struct HeaderOptions headerOptions = getHeaderOptions(data);

	//printf("%s", data);
	testVar();

	printf("compiled\n");

	char buf[100];
	gets(buf);

	return 0;
}