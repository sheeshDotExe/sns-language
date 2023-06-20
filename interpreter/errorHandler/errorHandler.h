#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#ifdef __unix__
#include <pthread.h>
#else
#include <windows.h>
#endif
#include "../processState.h"

void raise_error(char*errorMessage, int stopCompiling, struct ProcessState* processState);
#endif