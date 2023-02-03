#include "interpretBody.h"

struct DefinitionLines getLines(struct File file, unsigned long int start, unsigned long int end){
	struct DefinitionLines lines;
	unsigned int newLine = start;

	for (int i=start; i < end; i++){
		printf("%c",file.mem[i]);
	}
	/*
	while (1){
		unsigned int nextLine = findNextLine(file, newLine);

		printf("%d\n", newLine);

		if (nextLine == end){
			break;
		}

		newLine = newLine+nextLine+1;
	}
	*/
	return lines;
}

struct Body interpretBody(struct File file, unsigned long int start, unsigned long int end){

	struct Body body;

	struct DefinitionLines lines = getLines(file, start, end);

	return body;
}