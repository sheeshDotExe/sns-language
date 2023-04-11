#ifndef FUNCTION_LOGIC_H
#define FUNCTION_LOGIC_H

#include "../../types/types.h"
#include "../interpretBody.h"
#include "../bodyTypes.h"

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

struct KeyParam {
	unsigned int start;
	unsigned int end;
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

struct Var* evaluateExpression(struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index);

void getSetParams(struct Var* var, struct State* state, struct KeyPos** keyPositions, struct KeyWord** keyWords, unsigned int stop, unsigned int index);
struct VarScope* createVarScope(struct Var* var);
void freeVarScope(struct VarScope* varScope);
struct Var* callFunction(struct Var* var, struct State* state);
struct Function* getFunction(struct Var* var, struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index);

struct State* copyState(struct State* state);
#endif