#ifndef INTERPRET_TYPE_H
#define INTERPRET_TYPE_H

#include "types.h"
#include "../filePatterns/patternReader.h"
#include "../processState.h"

struct CommonTypes getValidTypes(char**value, unsigned int* length, struct ProcessState* processState);
struct Var* generateVarFromString(char*value, unsigned int length, struct ProcessState* processState);

#endif