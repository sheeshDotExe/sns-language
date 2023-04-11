#include "functionLogic.h"

struct Operator** sortOperators(struct Operator** operators, unsigned int length){
	struct Operator** newOperators = (struct Operator**)malloc(length*sizeof(struct Operator*));
	int i = 0;
	int order[NUMBER_OF_OPERATORS] = {Division_k, Multiplication_k, Subtract_k, Addition_k, Assign_k, FuncCallStart_k, SplitBySpace};

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

void freeOperator(struct Operator** operators, int length){
	for (int i = 0; i < length; i++){
		free(operators[i]);
	}
	free(operators);
}

void freeResults(struct Var** vars, int length){
	for (int i = 0; i < length; i++){
		freeVar(vars[i]);
		free(vars[i]);
	}
	free(vars);
}

void freeValues(struct Values* values){
	for (int i = 0; i < values->length; i++){
		freeVar(values->vars[i]);
		free(values->vars[i]);
	}
	free(values->vars);
}

unsigned int getParenthesesEnd(struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index){
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

struct Values getValues(struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index){
	int numberOfValues = 1;

	for (int i = index; i < stop; i++){
		numberOfValues++;
		if (keyPosition[i]->key == FuncCallStart_k){
			unsigned int next = getParenthesesEnd(keyPosition, keyWords, stop, i+1);
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


			unsigned int newStop = getParenthesesEnd(keyPosition, keyWords, stop, i+1);
			if (strlen(keyWords[i]->value) > 0){
				printf("function call %s\n", keyWords[i]->value);
				struct Var* var = getVarFromScope(state->localScope, keyWords[i]->value);
				printf("call var: %s\n", var->name);
				getSetParams(var, state, keyPosition, keyWords, stop, i);
				var->function->varScope = createVarScope(var);
				printf("call\n");
				struct State* copiedState = copyState(state);
				struct Var* returnValue = callFunction(var, copiedState); // key chars loop
				free(copiedState);
				printf("function return val %s\n", returnValue->value);

				values.vars[varIndex] = copyVar(returnValue);

				freeVar(returnValue);
				free(returnValue);

				freeVarScope(var->function->varScope);
			} else {
				values.vars[varIndex] = evaluateExpression(state, keyPosition, keyWords, newStop, i+1);
			}
			varIndex++;
			i = newStop+1;
			continue;
		}

		struct Var* result = generateVarFromString(keyWords[i]->value, strlen(keyWords[i]->value));
		if (!result->numberOfTypes){
			freeVar(result);
			free(result);
			result = copyVar(getVarFromScope(state->localScope, keyWords[i]->value));
		}
		values.vars[varIndex] = result;
		varIndex++;
	}
	if (i == stop && keyPosition[i-1]->key != FuncCallEnd_k){
		struct Var* result = generateVarFromString(keyWords[i]->value, strlen(keyWords[i]->value));
		if (!result->numberOfTypes){
			freeVar(result);
			free(result);
			result = copyVar(getVarFromScope(state->localScope, keyWords[i]->value));
		}
		values.vars[varIndex] = result;
	}

	return values;
};

struct Var* evaluateExpression(struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index){
	struct Var* result;

	unsigned int newIndex = index;

	struct Values values = getValues(state, keyPosition, keyWords, stop, index);

	unsigned int numberOfOperators = values.length - 1;

	if (!numberOfOperators){
		result = copyVar(values.vars[0]);
		freeValues(&values);
		return result;
	}

	struct Operator** operators = (struct Operator**)malloc(numberOfOperators*sizeof(struct Operator*));

	for (int i = 0; i < numberOfOperators; i++){
		struct KeyPos* keyPos = keyPosition[newIndex+i];
		struct Operator* operator = (struct Operator*)malloc(sizeof(struct Operator));

		if (keyPos->key == FuncCallStart_k){
			unsigned int end = getParenthesesEnd(keyPosition, keyWords, stop, newIndex+i+1);
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

	struct Operator** sortedOperators = sortOperators(operators, numberOfOperators);
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
				raiseError("Invalid operator", 1);
				result = copyVar(operator->rightVar);
				freeOperator(sortedOperators, numberOfOperators);
				freeValues(&values);
				free(results[numberOfOperators-1]);
				freeResults(results, numberOfOperators-1);
				return result;
			} break;

			case (Division_k): {
				res = divVars(operator->leftVar, operator->rightVar);
			} break;

			case (Multiplication_k): {
				res = mulVars(operator->leftVar, operator->rightVar);
			} break;

			case (Addition_k): {
				res = addVars(operator->leftVar, operator->rightVar);
			} break;

			case (Subtract_k): {
				res = subVars(operator->leftVar, operator->rightVar);
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
			result = copyVar(res);
		}
	}

	freeOperator(sortedOperators, numberOfOperators);
	freeValues(&values);
	freeResults(results, numberOfOperators);
	return result;
}

void getSetParams(struct Var* var, struct State* state, struct KeyPos** keyPositions, struct KeyWord** keyWords, unsigned int stop, unsigned int index){
	if (!var->hasParam){
		raiseError("no function found!", 1);
	}
	if (var->param->inputCount == 0){
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
		struct Var* result = evaluateExpression(state, keyPositions, keyWords, keyParams[i].end, keyParams[i].start+1);
		//printf("param result: %s\n", result->value);
		assignValue(var->param->inputVars[i], result);
		freeVar(result);
		free(result);
	}

	free(keyParams);

	if (numberOfParams != var->param->inputCount){
		raiseError("Incorrect number of params!", 1);
	}

	int paramCount = 0;
}

void freeVarScope(struct VarScope* varScope){
	for (int i = 0; i < varScope->numberOfVars; i++){
		freeVar(varScope->vars[i]);
		free(varScope->vars[i]);
	}
	free(varScope->vars);
	free(varScope);
}

struct VarScope* createVarScope(struct Var* var){
	struct Param* varParam = var->param;
	
	struct VarScope* varScope = (struct VarScope*)malloc(sizeof(struct VarScope));

	varScope->numberOfVars = varParam->inputCount+1; // add 1 for return value;
	varScope->vars = (struct Var**)malloc(varScope->numberOfVars*sizeof(struct Var*));

	int i = 0;
	for(i; i < varParam->inputCount; i++){
		varScope->vars[i] = copyVar(varParam->inputVars[i]);
		//printf("%d test %s\n", i, varParam->inputVars[i]->name);
	}
	varScope->vars[i] = copyVar(varParam->returnValue);

	return varScope;
}

struct Var* callFunction(struct Var* var, struct State* state){
	struct Function* function = var->function;

	freeVarScope(state->localScope);
	state->localScope = function->varScope;

	for (int i = 0; i < function->lines->length; i++){
		int code = interpretLine(state, function->lines->lines[i].value, function->lines->lines[i].length);
		if (code){
			break;
		}
	}
	return copyVar(getVarFromScope(function->varScope, "return"));
}

struct Function* getFunction(struct Var* var, struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index){
	struct Function* function = (struct Function*)malloc(sizeof(struct Function));

	if (strcmp(keyPosition[index+1]->name, "function")){
		raiseError("No function found\n", 1);
	}

	char* functionContent = keyWords[index+1]->value;

	struct DefinitionLines* lines = getLines(functionContent, 0, strlen(functionContent));
	function->lines = lines;

	return function;
}

struct State* copyState(struct State* state){
	struct State* newState = (struct State*)malloc(sizeof(struct State));
	newState->keyChars = state->keyChars;
	newState->builtins = state->builtins;
	newState->globalScope = state->globalScope;
	newState->localScope = copyVarScope(state->localScope);

	return newState;
}