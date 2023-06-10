#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../processState.h"

void raiseError(char*errorMessage, int stopCompiling, struct ProcessState* processState);
#endif