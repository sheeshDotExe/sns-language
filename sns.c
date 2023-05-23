#include "interpreter/interpreter.h"
#include <stdio.h>
#include <wchar.h>
#include <locale.h>

FILE* getFile(char*path){
	return fopen(path, "r");
}

int main(int argc, char **argv)
{

	if (argc < 1){
		printf("incorrect input\n");
		return 1;
	}

	//char* path = argv[1];

	FILE* file = getFile("./demo.sns");

	if (interpret(file)){
		printf("error error\n");
		return 1;
	}

	return 0;
}