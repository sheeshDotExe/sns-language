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

struct Var* evaluateExpression(struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index, struct ProcessState* processState);

void getSetParams(struct Param* param, struct State* state, struct KeyPos** keyPositions, struct KeyWord** keyWords, unsigned int stop, unsigned int index, struct ProcessState* processState);
struct VarScope* createVarScope(struct Var* var, struct ProcessState* processState);
void freeVarScope(struct VarScope* varScope, struct ProcessState* processState);
struct Var* callFunction(struct Var* var, struct State* state, struct ProcessState* processState);
struct Function* getFunction(struct Var* var, struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index, struct ProcessState* processState);

struct State* copyState(struct State* state, struct ProcessState* processState);
struct State* hardcopyState(struct State* state, struct ProcessState* processState);
#endif