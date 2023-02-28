#ifndef INTERPRET_TYPE_H
#define INTERPRET_TYPE_H

#include "types.h"
#include "../filePatterns/patternReader.h"

int isString(char*value, unsigned int length);
struct CommonTypes getValidTypes(char**value, unsigned int* length);
struct Var* generateVarFromString(char*value, unsigned int length);

#endif