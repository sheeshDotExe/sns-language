#include "interpreter.h"

int interpret(FILE*file){

	struct File data = readFile(file);

	struct HeaderOptions headerOptions = getHeaderOptions(data);

	//printf("%s", data);

	char buf[100];
	gets(buf);

	return 0;
}