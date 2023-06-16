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

struct Var* evaluate_expression(struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index, struct ProcessState* processState);

void get_set_params(struct Param* param, struct State* state, struct KeyPos** keyPositions, struct KeyWord** keyWords, unsigned int stop, unsigned int index, struct ProcessState* processState);
struct VarScope* create_var_scope(struct Var* var, struct ProcessState* processState);
void free_var_scope(struct VarScope* varScope, struct ProcessState* processState);
struct Var* call_function(struct Var* var, struct State* state, struct ProcessState* processState);
struct Function* get_function(struct Var* var, struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index, struct ProcessState* processState);

struct State* copy_state(struct State* state, struct ProcessState* processState);
struct State* hardcopy_state(struct State* state, struct ProcessState* processState);
#endif