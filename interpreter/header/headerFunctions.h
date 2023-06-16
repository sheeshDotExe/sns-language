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
	int(*function)(struct HeaderOptions* headerOptions, char*name, char**args, int argc, struct ProcessState* processState);
};

struct HeaderAtlas{
	struct FunctionMap* functions;
};

void add_function(struct FunctionMap* functionMap, int index, char*name, int(*function)(struct HeaderOptions* headerOptions, char*name, char**args, int argc, struct ProcessState* processState), struct ProcessState* processState);
void interprete_header_function(struct HeaderOptions* headerOptions, struct HeaderAtlas* headerAtlas, char*keyword, char**arguments, unsigned int kwLength, unsigned int argc, struct ProcessState* processState);
struct HeaderAtlas get_function_map(struct ProcessState* processState);

#endif