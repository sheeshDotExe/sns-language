#include "errorHandler.h"

void raiseError(char*errorMessage, int stopCompiling){
	printf(errorMessage);

	if (stopCompiling){
		char temp[50];
		gets(temp);
		exit(1);
	}
}