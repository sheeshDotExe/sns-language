#include "errorHandler.h"

void raiseError(char*errorMessage, int stopCompiling, struct ProcessState* processState){
	printf(errorMessage);

	int exit = 1;

	if (stopCompiling){
		processState->running = 0;
		pthread_exit((void*)&exit);
	}
}