#ifndef INTERPRET_BODY_H
#define INTERPRET_BODY_H

#include "../types/types.h"
#include "../types/interpretType.h"
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
	char* name;
	int key;
};

struct KeyWord {
	char* value;
	unsigned int length;
};

struct Operator {
	int type;
	struct Var* leftVar;
	struct Var* rightVar;
	struct Operator* leftOperator;
	struct Operator* rightOperator;
};

struct DefinitionLines getLines(struct File file, unsigned long int start, unsigned long int end);

struct Body interpretBody(struct KeyChars keyChars, struct File file, unsigned long int start, unsigned long int end);

#endif