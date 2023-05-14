#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdio.h>
#include <stdlib.h>

void raiseError(char*errorMessage, int stopCompiling);
void pause();

#endif