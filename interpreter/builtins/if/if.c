#include "if.h"

struct Var *if_implementation(struct Param *params, struct State *state){
	struct Var* condition = params->inputVars[0];

	struct Param* param = (struct Param*)malloc(sizeof(struct Param));
	param->inputCount = 0;
	param->inputVars = (struct Var**)malloc(0);
	int codes[1] = {String_c};

	param->returnValue = generateVar(&codes, 1, "return", "", (struct Param*)NULL);

	codes[1] = Function_c;

	struct Var* response = generateVar(&codes, 1, "if", "", param);

	response->shouldExecute = isTrue(condition);
	response->assignable = 1;
	response->inheritScopes = 1;
	response->isBuiltin = 1;

	return response;
}

struct Var *else_implementation(struct Param *params, struct State *state){

	struct Param* param = (struct Param*)malloc(sizeof(struct Param));
	param->inputCount = 0;
	param->inputVars = (struct Var**)malloc(0);
	int codes[1] = {String_c};

	param->returnValue = generateVar(&codes, 1, "return", "", (struct Param*)NULL);

	codes[1] = Function_c;

	struct Var* response = generateVar(&codes, 1, "if", "", param);

	response->shouldExecute = 2;
	response->assignable = 1;
	response->inheritScopes = 1;
	response->isBuiltin = 1;

	return response;
}