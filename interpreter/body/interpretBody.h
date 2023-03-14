#ifndef INTERPRET_BODY_H
#define INTERPRET_BODY_H

#include "../types/types.h"
#include "../fileReader/fileReader.h"
#include "../filePatterns/patternReader.h"

struct Body {
	struct VarScope globalScope;

	int hasMain;
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

struct Values {
	struct Var** vars;
	unsigned int length;
};

struct Operator {
	int type;
	struct Var* leftVar;
	struct Var* rightVar;
	struct Operator* leftOperator;
	struct Operator* rightOperator;
};

struct DefinitionLines getLines(char* mem, unsigned long int start, unsigned long int end);

struct Var* evaluateExpression(struct VarScope* varScope, struct KeyPos* keyPosition, struct KeyWord* keyWords, unsigned int stop, unsigned int index);
struct Body interpretBody(struct KeyChars keyChars, struct File file, unsigned long int start, unsigned long int end);
int interpretLine(struct KeyChars keyChars, struct VarScope* varScope, char* line, unsigned int length);

#endif