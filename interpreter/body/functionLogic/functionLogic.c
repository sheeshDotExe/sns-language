#include "functionLogic.h"

struct Operator** sort_operators(struct Operator** operators, unsigned int length, struct ProcessState* processState){
	struct Operator** newOperators = (struct Operator**)malloc(length*sizeof(struct Operator*));
	int i = 0;
	int order[NUMBER_OF_OPERATORS] = {Division_k, Multiplication_k, Subtract_k, Addition_k, Assign_k, LessThan_k, GreaterThan_k, EqualTo_k, FuncCallStart_k, SplitBySpace};

	for (int j = 0; j < NUMBER_OF_OPERATORS; j++){
		int operator = order[j];
		for (int z = 0; z < length; z++){
			if (operators[z]->type == operator){
				newOperators[i] = operators[z];
				i++;
			}
		}
	}

	return newOperators;
}

void free_operator(struct Operator** operators, int length, struct ProcessState* processState){
	for (int i = 0; i < length; i++){
		free(operators[i]);
	}
	free(operators);
}

void free_results(struct Var** vars, int length, struct ProcessState* processState){
	for (int i = 0; i < length; i++){
		free_var(vars[i], processState);
		free(vars[i]);
	}
	free(vars);
}

void free_values(struct Values* values, struct ProcessState* processState){
	for (int i = 0; i < values->length; i++){
		free_var(values->vars[i], processState);
		free(values->vars[i]);
	}
	free(values->vars);
}

unsigned int get_parentheses_end(struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index, struct ProcessState* processState){
	int parentLength = 1;
	for (int i = index; i < stop; i++){
		if (keyPosition[i]->key == FuncCallEnd_k){
			parentLength--;
		} else if (keyPosition[i]->key == FuncCallStart_k){
			parentLength++;
		}

		if (!parentLength){
			return i;
		}
	}
	return -1;
}

struct Values get_values(struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index, struct ProcessState* processState){
	int numberOfValues = 1;

	for (int i = index; i < stop; i++){
		numberOfValues++;
		if (keyPosition[i]->key == FuncCallStart_k){
			unsigned int next = get_parentheses_end(keyPosition, keyWords, stop, i+1, processState);
			i = next;
			numberOfValues--;
		}
	}

	struct Values values;

	values.vars = (struct Var**)malloc(numberOfValues*sizeof(struct Var*));
	values.length = numberOfValues;

	
	int varIndex = 0;
	int i = index;
	for (i; i < stop; i++){
		if (keyPosition[i]->key == FuncCallStart_k){
			//printf("keyword for func %s\n", keyWords[i]->value);


			unsigned int newStop = get_parentheses_end(keyPosition, keyWords, stop, i+1, processState);
			if (strlen(keyWords[i]->value) > 0){

				int builtinPos = is_builtin(state->builtins, keyWords[i]->value, processState);
				if (builtinPos){
					struct BuiltinFunction* function = get_builtin(state->builtins, builtinPos, processState);
					get_set_params(function->params, state, keyPosition, keyWords, stop, i, processState);
					struct Var* returnValue = function->function(function->params, state, processState);

					values.vars[varIndex] = copy_var(returnValue, processState);
					free_var(returnValue, processState);
					free(returnValue);

					free_param(function->params, processState);
					function->params = copy_param(function->originalParams, processState);
				} else {
					struct Var* var = NULL;
					if (state->useInheritence){
						var = get_var_from_inherited_scopes(state->inheritedVarscopes, keyWords[i]->value, processState);
					}
					if (var == NULL){
						var = get_var_from_scopes(state->localScope, state->globalScope, keyWords[i]->value, processState);
					}
					//printf("call var: %s\n", var->name);
					get_set_params(var->param, state, keyPosition, keyWords, stop, i, processState);
					var->function->varScope = create_var_scope(var, processState);
					//printf("call\n");
					struct State* copiedState = copy_state(state, processState);
					struct Var* returnValue = call_function(var, copiedState, processState); // key chars loop

					free_param(var->param, processState);
					var->param = copy_param(var->originalParam, processState);
					free(copiedState);
					//printf("function return val %s\n", returnValue->value);

					values.vars[varIndex] = copy_var(returnValue, processState);

					free_var(returnValue, processState);
					free(returnValue);

					free_var_scope(var->function->varScope, processState);
				}
			} else {
				values.vars[varIndex] = evaluate_expression(state, keyPosition, keyWords, newStop, i+1, processState);
			}
			varIndex++;
			i = newStop+1;
			continue;
		}

		struct Var* result = generate_var_from_string(keyWords[i]->value, strlen(keyWords[i]->value), processState);
		if (result->numberOfTypes == 1 && result->types[0].code == String_c && !is_string(keyWords[i]->value, strlen(keyWords[i]->value), processState)){
			free_var(result, processState);
			free(result);
			struct Var* var = NULL;
			if (state->useInheritence){
				var = get_var_from_inherited_scopes(state->inheritedVarscopes, keyWords[i]->value, processState);
			}
			if (var == NULL){
				var = get_var_from_scopes(state->localScope, state->globalScope, keyWords[i]->value, processState);
			}
			result = copy_var(var, processState);
		}
		values.vars[varIndex] = result;
		varIndex++;
	}
	if (i == stop && keyPosition[i-1]->key != FuncCallEnd_k){
		struct Var* result = generate_var_from_string(keyWords[i]->value, strlen(keyWords[i]->value), processState);
		if (result->numberOfTypes == 1 && result->types[0].code == String_c && !is_string(keyWords[i]->value, strlen(keyWords[i]->value), processState)){
			free_var(result, processState);
			free(result);
			struct Var* var = NULL;
			if (state->useInheritence){
				var = get_var_from_inherited_scopes(state->inheritedVarscopes, keyWords[i]->value, processState);
			}
			if (var == NULL){
				var = get_var_from_scopes(state->localScope, state->globalScope, keyWords[i]->value, processState);
			}
			result = copy_var(var, processState);
		}
		values.vars[varIndex] = result;
	}

	return values;
};

struct Var* evaluate_expression(struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index, struct ProcessState* processState){
	struct Var* result;

	unsigned int newIndex = index;

	struct Values values = get_values(state, keyPosition, keyWords, stop, index, processState);

	unsigned int numberOfOperators = values.length - 1;

	if (!numberOfOperators){
		result = copy_var(values.vars[0], processState);
		free_values(&values, processState);
		return result;
	}

	struct Operator** operators = (struct Operator**)malloc(numberOfOperators*sizeof(struct Operator*));

	for (int i = 0; i < numberOfOperators; i++){
		struct KeyPos* keyPos = keyPosition[newIndex+i];
		struct Operator* operator = (struct Operator*)malloc(sizeof(struct Operator));

		if (keyPos->key == FuncCallStart_k){
			unsigned int end = get_parentheses_end(keyPosition, keyWords, stop, newIndex+i+1, processState);
			newIndex = end-i+1;
			keyPos = keyPosition[newIndex+i];
		}

		operator->type = keyPos->key;

		operator->leftVar = values.vars[i];
		operator->rightVar = values.vars[i+1];

		operators[i] = operator;
	}

	for (int i = 0; i < numberOfOperators; i++){
		struct Operator* operator = operators[i];
		if (i){
			operator->leftOperator = operators[i-1];
		} else {
			operator->leftOperator = NULL;
		}
		if (i < numberOfOperators-1){
			operator->rightOperator = operators[i+1];
		} else {
			operator->rightOperator = NULL;
		}
	}

	struct Operator** sortedOperators = sort_operators(operators, numberOfOperators, processState);
	free(operators);

	struct Var** results = (struct Var**)malloc(numberOfOperators*sizeof(struct Var*));

	for (int i = 0; i < numberOfOperators; i++){
		struct Operator* operator = sortedOperators[i];

		struct Var* res;
		
		switch (operator->type){
			case (FuncCallStart_k):
			case (Assign_k):
			case (SubtractAssign_k):
			case (AdditionAssign_k):
			case (DivisionAssign_k):
			case (MultiplicationAssign_k):
			case (SplitBySpace):{
				raise_error("Invalid operator", 1, processState);
				result = copy_var(operator->rightVar, processState);
				free_operator(sortedOperators, numberOfOperators, processState);
				free_values(&values, processState);
				free(results[numberOfOperators-1]);
				free_results(results, numberOfOperators-1, processState);
				return result;
			} break;

			case (Division_k): {
				res = div_vars(operator->leftVar, operator->rightVar, processState);
			} break;

			case (Multiplication_k): {
				res = mul_vars(operator->leftVar, operator->rightVar, processState);
			} break;

			case (Addition_k): {
				res = add_vars(operator->leftVar, operator->rightVar, processState);
			} break;

			case (Subtract_k): {
				res = sub_vars(operator->leftVar, operator->rightVar, processState);
			} break;

			case (GreaterThan_k): {
				res = greater_than(operator->leftVar, operator->rightVar, processState);
			} break;

			case (LessThan_k): {
				res = less_than(operator->leftVar, operator->rightVar, processState);
			} break;

			case (EqualTo_k): {
				res = equal_to(operator->leftVar, operator->rightVar, processState);
			} break;
		}

		results[i] = res;
		
		if (operator->leftOperator != NULL){
			operator->leftOperator->rightVar = res;
			if (operator->rightOperator != NULL){
				operator->leftOperator->rightOperator = operator->rightOperator;
			}
		}
		if (operator->rightOperator != NULL){
			operator->rightOperator->leftVar = res;
			if (operator->leftOperator != NULL){
				operator->rightOperator->leftOperator = operator->leftOperator;
			}
		}

		if (i == numberOfOperators-1){
			result = copy_var(res, processState);
		}
	}

	free_operator(sortedOperators, numberOfOperators, processState);
	free_values(&values, processState);
	free_results(results, numberOfOperators, processState);
	return result;
}

void get_set_params(struct Param* param, struct State* state, struct KeyPos** keyPositions, struct KeyWord** keyWords, unsigned int stop, unsigned int index, struct ProcessState* processState){

	if (param->inputCount == 0){
		return;
	}

	int numberOfParams = 1;
	
	for (int i = index; i < stop; i++){
		struct KeyPos* keyPos = keyPositions[i];
		if (keyPos->key == Param_k){
			numberOfParams++;
		}
		if (keyPos->key == FuncCallEnd_k){
			break;
		}
	}

	struct KeyParam* keyParams = (struct KeyParam*)malloc(numberOfParams*sizeof(struct KeyParam));

	//printf("params: %d\n", numberOfParams);

	unsigned int start = index;
	unsigned int paramIndex = 0;
	for (int i = index; i < stop; i++){
		struct KeyPos* keyPos = keyPositions[i];
		if (keyPos->key == Param_k){
			keyParams[paramIndex].start = start;
			keyParams[paramIndex].end = i;
			paramIndex++;
			start = i;
		}
		if (keyPos->key == FuncCallEnd_k){
			keyParams[paramIndex].start = start;
			keyParams[paramIndex].end = i;
			break;
		}
	}

	//printf("set param vars: %d\n", var->param->inputCount);

	for (int i = 0; i < numberOfParams; i++){
		struct Var* result = evaluate_expression(state, keyPositions, keyWords, keyParams[i].end, keyParams[i].start+1, processState);
		//printf("param result: %s\n", result->value);
		assign_value(param->inputVars[i], result, processState);
		free_var(result, processState);
		free(result);
	}

	free(keyParams);

	int paramCount = 0;
}

void free_var_scope(struct VarScope* varScope, struct ProcessState* processState){
	for (int i = 0; i < varScope->numberOfVars; i++){
		free_var(varScope->vars[i], processState);
		free(varScope->vars[i]);
	}
	free(varScope->vars);
	free(varScope);
}

struct VarScope* create_var_scope(struct Var* var, struct ProcessState* processState){
	struct Param* varParam = var->param;
	
	struct VarScope* varScope = (struct VarScope*)malloc(sizeof(struct VarScope));

	varScope->isTrue = 0;

	varScope->numberOfVars = varParam->inputCount+1; // add 1 for return value;
	varScope->vars = (struct Var**)malloc(varScope->numberOfVars*sizeof(struct Var*));

	int i = 0;
	for(i; i < varParam->inputCount; i++){
		varScope->vars[i] = copy_var(varParam->inputVars[i], processState);
		//printf("%d test %s\n", i, varParam->inputVars[i]->name);
	}
	varScope->vars[i] = copy_var(varParam->returnValue, processState);

	return varScope;
}

struct Var* call_function(struct Var* var, struct State* state, struct ProcessState* processState){

	struct Function* function = var->function;
	free_var_scope(state->localScope, processState);

	state->localScope = function->varScope;
	int returned = 0;

	for (int i = 0; i < function->lines->length; i++){
		int code = interpret_line(state, function->lines->lines[i].value, function->lines->lines[i].length, processState);
		if (code){
			returned = 1;
			break;
		}
	}

	struct Var* returnValue = get_var_from_scope(function->varScope, "return", processState);
	returnValue->returned = returned;

	return copy_var(returnValue, processState);
}

struct Function* get_function(struct Var* var, struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index, struct ProcessState* processState){
	struct Function* function = (struct Function*)malloc(sizeof(struct Function));

	if (strcmp(keyPosition[index+1]->name, "function")){
		raise_error("No function found\n", 1, processState);
	}

	char* functionContent = keyWords[index+1]->value;

	struct DefinitionLines* lines = get_lines(functionContent, 0, strlen(functionContent), processState);
	function->lines = lines;

	return function;
}

struct State* hardcopy_state(struct State* state, struct ProcessState* processState){
	struct State* newState = (struct State*)malloc(sizeof(struct State));
	newState->keyChars = state->keyChars;
	newState->builtins = state->builtins;
	newState->routes = state->routes;
	newState->files = state->files;
	newState->globalScope = copy_var_scope(state->globalScope, processState);
	newState->fileExtension = (char**)malloc(sizeof(char*));
	newState->fileExtension[0] = strdup("");

	newState->inheritedVarscopes = hardcopy_inherited_varscope(state->inheritedVarscopes, processState);

	newState->useInheritence = state->useInheritence;
	newState->localScope = copy_var_scope(state->localScope, processState);

	return newState;
};

struct State* copy_state(struct State* state, struct ProcessState* processState){
	struct State* newState = (struct State*)malloc(sizeof(struct State));
	newState->keyChars = state->keyChars;
	newState->builtins = state->builtins;
	newState->routes = state->routes;
	newState->files = state->files;
	newState->globalScope = state->globalScope;
	newState->fileExtension = state->fileExtension;

	newState->inheritedVarscopes = copy_inherited_varscope(state->inheritedVarscopes, processState);

	newState->useInheritence = state->useInheritence;
	newState->localScope = copy_var_scope(state->localScope, processState);

	return newState;
}