#include "if.h"

struct Var *if_implementation(struct Param *params, struct State *state, struct ProcessState* processState){
	struct Var* condition = params->inputVars[0];

	struct Param* param = (struct Param*)malloc(sizeof(struct Param));
	param->inputCount = 0;
	param->inputVars = (struct Var**)malloc(0);
	int codes[1] = {String_c};

	param->returnValue = generate_var(&codes, 1, "return", "", (struct Param*)NULL, processState);

	codes[1] = Function_c;

	struct Var* response = generate_var(&codes, 1, "if", "", param, processState);

	response->shouldExecute = is_true(condition, processState);
	response->assignable = 1;
	response->inheritScopes = 1;
	response->isBuiltin = 1;

	return response;
}

struct Var *else_implementation(struct Param *params, struct State *state, struct ProcessState* processState){

	struct Param* param = (struct Param*)malloc(sizeof(struct Param));
	param->inputCount = 0;
	param->inputVars = (struct Var**)malloc(0);
	int codes[1] = {String_c};

	param->returnValue = generate_var(&codes, 1, "return", "", (struct Param*)NULL, processState);

	codes[1] = Function_c;

	struct Var* response = generate_var(&codes, 1, "if", "", param, processState);

	response->shouldExecute = 2;
	response->assignable = 1;
	response->inheritScopes = 1;
	response->isBuiltin = 1;

	return response;
}