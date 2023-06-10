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


struct DefinitionLines* getLines(char* mem, unsigned long int start, unsigned long int end, struct ProcessState* processState);
struct Body* interpretBody(struct State* state, struct File file, unsigned long int start, unsigned long int end, struct ProcessState* processState);
int interpretLine(struct State* state, char* line, unsigned int length, struct ProcessState* processState);
struct KeyWord** getKeyWords(unsigned int keysCount, struct KeyPos** keyPositions, struct State* state, char* line, unsigned int length, struct ProcessState* processState);
struct KeyPos** getKeyPositions(unsigned int keysCount, struct State* state, char* line, unsigned int length, unsigned int start, struct ProcessState* processState);
unsigned int getKeysCount(struct State* state, char* line, unsigned int length, unsigned int start, struct ProcessState* processState);
struct Var* getVarTypes(char* varName, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int length, unsigned int index, unsigned int* increment, struct ProcessState* processState);
#endif