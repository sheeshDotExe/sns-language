#ifndef INTERPRET_BODY_H
#define INTERPRET_BODY_H

#include "../types/types.h"
#include "../fileReader/fileReader.h"
#include "../filePatterns/patternReader.h"
#include "./functionLogic/functionLogic.h"
#include "./bodyTypes.h"
#include "./eval/eval.h"

struct Body {
	struct VarScope globalScope;

	int hasMain;
};


struct DefinitionLines* get_lines(char* mem, unsigned long int start, unsigned long int end, struct ProcessState* processState);
struct Body* interpret_body(struct State* state, struct File file, unsigned long int start, unsigned long int end, struct ProcessState* processState);
int interpret_line(struct State* state, char* line, unsigned int length, struct ProcessState* processState);
struct KeyWord** get_key_words(unsigned int keysCount, struct KeyPos** keyPositions, struct State* state, char* line, unsigned int length, struct ProcessState* processState);
struct KeyPos** get_key_positions(unsigned int keysCount, struct State* state, char* line, unsigned int length, unsigned int start, struct ProcessState* processState);
unsigned int get_keys_count(struct State* state, char* line, unsigned int length, unsigned int start, struct ProcessState* processState);
struct Var* get_var_types(char* varName, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int length, unsigned int index, unsigned int* increment, struct ProcessState* processState);
#endif