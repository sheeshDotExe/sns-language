#include "interpreter/interpreter.h"
#include <stdio.h>

FILE* getFile(char*path){
	return fopen(path, "r");
}

int main(int argc, char **argv)
{
	if (argc < 2){
		printf("incorrect input\n");
		return 1;
	}

	char* path = argv[1];

	FILE* file = getFile(path);

	if (interpret(file)){
		printf("error error\n");
		return 1;
	}

	return 0;
}