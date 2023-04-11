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


struct DefinitionLines* getLines(char* mem, unsigned long int start, unsigned long int end);
struct Body interpretBody(struct State* state, struct File file, unsigned long int start, unsigned long int end);
int interpretLine(struct State* state, char* line, unsigned int length);
#endif