#ifndef INTERPRET_BODY_H
#define INTERPRET_BODY_H

#include "../types/types.h"
#include "../fileReader/fileReader.h"
#include "../filePatterns/patternReader.h"

struct Body {
	struct VarScope globalScope;

	int hasMain;
};

struct DefinitionLine {
	char* value;
	unsigned int length;
};

struct DefinitionLines {
	struct DefinitionLine* lines;
	unsigned int length;
};

struct KeyPos {
	unsigned int pos;
	unsigned int endPos;
	int key;
};

struct KeyWord {
	char* value;
	unsigned int length;
};

struct DefinitionLines getLines(struct File file, unsigned long int start, unsigned long int end);

struct Body interpretBody(struct KeyChars keyChars, struct File file, unsigned long int start, unsigned long int end);

#endif