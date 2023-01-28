#ifndef HEADER_FUNCTIONS_H
#define HEADER_FUNCTIONS_H

#define NUMBER_OF_FUNCTIONS 8

#include "headerStructs.h"
#include "../types/types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct FunctionMap{
	char * name;
	unsigned int nameLength;
	int(*function)(struct HeaderOptions* headerOptions, char*name, char**args, int argc);
};

struct HeaderAtlas{
	struct FunctionMap* functions;
};

void addFunction(struct FunctionMap* functionMap, int index, char*name, int(*function)(struct HeaderOptions* headerOptions, char*name, char**args, int argc));
void interpreteHeaderFunction(struct HeaderOptions* headerOptions, struct HeaderAtlas* headerAtlas, char*keyword, char**arguments, unsigned int kwLength, unsigned int argc);
struct HeaderAtlas getFunctionMap();

#endif