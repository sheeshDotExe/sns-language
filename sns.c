#include "interpreter/interpreter.h"
#include <stdio.h>
#include <wchar.h>
#include <locale.h>

FILE* get_file(char*path, struct ProcessState* processState){
	return fopen(path, "r");
}

int main(int argc, char **argv)
{

	if (argc < 1){
		printf("incorrect input\n");
		return 1;
	}


	setlocale(LC_ALL, ".UTF8");

	//char* path = argv[1];

	struct ProcessState* processState = (struct ProcessState*)malloc(sizeof(struct ProcessState));
	processState->running = 1;

	FILE* file = get_file("./demo.sns", processState);

	if (interpret(file, processState)){
		printf("error error\n");
		return 1;
	}

	return 0;
}