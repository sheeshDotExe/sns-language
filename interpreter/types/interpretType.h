#ifndef INTERPRET_TYPE_H
#define INTERPRET_TYPE_H

#include "types.h"
#include "../filePatterns/patternReader.h"
#include "../processState.h"

struct CommonTypes get_valid_types(char**value, unsigned int* length, struct ProcessState* processState);
struct Var* generate_var_from_string(char*value, unsigned int length, struct ProcessState* processState);

#endif