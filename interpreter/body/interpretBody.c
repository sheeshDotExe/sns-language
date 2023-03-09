#include "interpretBody.h"

// sussy algorithm
unsigned int getNewline(char* mem, unsigned long int* start, unsigned long int end){
	unsigned int length = 0;
	unsigned int brackets = 0;
	while (*start < end){
		if (mem[*start] == '{'){
			brackets++;
		} else if (mem[*start] == '}'){
			brackets--;
		}
		if ((mem[*start] == '\n' || *start == end-1) && !brackets){
			if (length){
				int shouldAdd = 0;
				for (int i = *start-length; i < *start; i++){
					if (mem[i] != ' ' && mem[i] != '\n'){
						shouldAdd = 1;
					}
				}
				if (shouldAdd){
					return length;
				}
			}
			return 0;
		}
		length++;
		(*start)++;
	}
	return length;
}

struct DefinitionLines getLines(char* mem, unsigned long int start, unsigned long int end){
	struct DefinitionLines lines;

	char* line;
	unsigned int length = 0;

	unsigned int numberOfLines = 0;

	unsigned long int newStart = start;
	
	while (newStart < end){
		if (getNewline(mem, &newStart, end)){
			numberOfLines++;
		}
		newStart++;
	}

	lines.length = numberOfLines;
	lines.lines = (struct DefinitionLine*)malloc(numberOfLines*sizeof(struct DefinitionLine));

	unsigned int lineCount = 0;
	newStart = start;

	while (newStart < end){
		length = getNewline(mem, &newStart, end);
		if (length){
			lines.lines[lineCount].length = length+2;
			lines.lines[lineCount].value = (char*)malloc((length+2)*sizeof(char));
			memcpy(lines.lines[lineCount].value, mem + newStart-length, length+1);
			lines.lines[lineCount].value[length+1] = '\0';
			lineCount++;
		}
		newStart++;
	}
	
	return lines;
}

struct KeyPos getNextKey(struct KeyChars keyChars, char* line, unsigned int length, unsigned int start){

	struct KeyPos keyPos;

	int isString = 0;
	int hasFunction = 0;
	int bracketCount = 0;

	for (int index = start; index < length; index++){

		if (line[index] == '{'){
			bracketCount++;
			hasFunction = 1;
			continue;
		} else if (line[index] == '}'){
			bracketCount--;
			continue;
		}

		if (bracketCount){
			continue;
		}

		if (line[index] == '"'){
			isString = !isString;
		}

		if (hasFunction){
			keyPos.pos = index;
			keyPos.endPos = index;
			keyPos.key = FuncStart_k;
			keyPos.name = (char*)"function";
			return keyPos;
		}

		for (int i = 0; i < NUMBER_OF_KEYS; i++){
			struct Key* key = &keyChars.keys[i];
			int isValid = 1;
			for (int j = 0; j < key->length-1; j++){ // -1 to get rid of '\0'
				if (index + j < length){
					if (line[index+j] != key->name[j]){
						isValid = 0; // 6 indentations:>)
					}
				} else {
						isValid = 0;
				}
			}

			if (isValid && !isString) {
				keyPos.pos = index;
				keyPos.endPos = index + (key->length-1);
				keyPos.key = key->key;
				keyPos.name = key->name;
				return keyPos;
			}
		}
		
	}

	keyPos.pos = start;
	keyPos.endPos = start;
	keyPos.key = -1;
	return keyPos;
}

struct KeyWord getKeyword(char* line, unsigned int start, unsigned int stop){
	struct KeyWord keyWord;

	unsigned int charCount = 0;
	int isFunction = 0;

	for (int i = start; i < stop; i++){
		int isString = -1;
		if (line[i] == '"' || line[i] == '\''){
			isString = isString*-1;
		}

		if (isString == -1){
			if (line[i] == '{'){
				isFunction = 1;
				start = i+1;
				stop--;
				break;
			}
		}
	}

	for (int i = start; i < stop; i++){
		if (isFunction || (line[i] != ' ' && line[i] != '\t' && line[i] != '\n')){
			charCount++;
		}
	}

	keyWord.value = (char*)malloc((charCount+1)*sizeof(char));

	unsigned int count = 0;
	for (int i = start; i < stop; i++){
		if (isFunction || (line[i] != ' ' && line[i] != '\t' && line[i] != '\n')){
			keyWord.value[count] = line[i];
			count++;
		}
	}

	keyWord.value[charCount] = '\0';
	keyWord.length = charCount;

	return keyWord;
}

int typeFromString(char* value, unsigned int length){
	if (!strcmp(value, "float")){
		return Float_c;
	} else if (!strcmp(value, "int")){
		return Int_c;
	} else if (!strcmp(value, "string")){
		return String_c;
	} else if (!strcmp(value, "function")){
		return Function_c;
	} else if (!strcmp(value, "array")){
		return Array_c;
	} else if (!strcmp(value, "bool")){
		return Int_c;
	}
	printf("no type named %s\n", value);
	raiseError("", 1);
	return -1;
}

struct Var* getVarTypes(char* varName, struct KeyPos* keyPosition, struct KeyWord* keyWords, unsigned int length, unsigned int index, unsigned int* increment){
	int count = 0;

	struct Var* var;

	unsigned int newIndex = index + 1;
	while (newIndex < length){ // itterate keys
		struct KeyPos* keyPos = &keyPosition[newIndex];
		struct KeyWord* keyWord = &keyWords[newIndex];
		count++;
		if (keyPos->key != Type_k){
			break;
		}
		newIndex++;
	}

	int* codes = (int*)malloc((count)*sizeof(int));

	struct Param* param = NULL;
	int hasParam = 0;

	unsigned int i = 0;
	newIndex = index + 1;
	(*increment)++;
	while (newIndex <= length){ // itterate keys

		if (newIndex == length){
			struct KeyWord* keyWord = &keyWords[newIndex];
			codes[i] = typeFromString(keyWord->value, keyWord->length);
			break;
		}

		struct KeyPos* keyPos = &keyPosition[newIndex];
		struct KeyWord* keyWord = &keyWords[newIndex];
		codes[i] = typeFromString(keyWord->value, keyWord->length);

		if (codes[i] == Function_c){
			int paramCount = 1;
			struct KeyPos* currentKey = keyPos;
			int nCount = 0;
			while (currentKey->key != FuncTypeEnd_k){
				if (currentKey->key == Param_k){
					paramCount++;
				}
				nCount++;
				if (newIndex + nCount >= length){
					raiseError("no closing tag in types (missing ])", 1);
				}
				currentKey = &keyPosition[newIndex+nCount];
			}

			param = (struct Param*)malloc(sizeof(struct Param));
			param->inputVars = (struct Var**)malloc(sizeof(struct Var*) * (paramCount-1));
			param->inputCount = paramCount-1;

			unsigned int paramStart = newIndex;
			unsigned int paramEnd = newIndex;
			unsigned int paramCounter = 0;

			currentKey = keyPos;
			nCount = 0;
			unsigned int wordCount = 0;

			struct KeyWord* currentWord = &keyWords[paramStart];

			while (currentKey->key != FuncTypeEnd_k){
				if (currentKey->key == Param_k){
					paramCount--;
					if (paramCount){
						param->inputVars[paramCounter] = getVarTypes(keyWords[paramEnd+1].value, keyPosition, keyWords, length, paramEnd+1, increment);
					} else {
						break;
					}
					paramEnd = paramStart;
				}
				paramStart++;
				currentKey = &keyPosition[paramStart];
				currentWord = &keyWords[paramStart];
			}
			param->returnValue = getVarTypes("return", keyPosition, keyWords, length, paramEnd, increment);
			hasParam = 1;
		}
		i++;
		if (keyPos->key != Type_k){
			break;
		}
		newIndex++;
		(*increment)++;
	}
	printf("variable: %d\n", count);
	for (int i = 0; i < count; i++){
		printf("code: %d\n", codes[i]);
	}
	var = generateVar(codes, count, varName, "", param);
	var->hasParam = hasParam;
	return var;
}

struct Operator** sortOperators(struct Operator** operators, unsigned int length){
	struct Operator** newOperators = (struct Operator**)malloc(length*sizeof(struct Operator*));
	int i = 0;
	int order[NUMBER_OF_OPERATORS] = {Division_k, Multiplication_k, Subtract_k, Addition_k, Assign_k, FuncCallStart_k};

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
		if (vars[i]->creationFlag){
			freeVar(vars[i]);
		}
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

unsigned int getParenthesesEnd(struct KeyPos* keyPosition, struct KeyWord* keyWords, unsigned int stop, unsigned int index){
	int parentLength = 1;
	for (int i = index; i < stop; i++){
		if (keyPosition[i].key == FuncCallEnd_k){
			parentLength--;
		} else if (keyPosition[i].key == FuncCallStart_k){
			parentLength++;
		}

		if (!parentLength){
			return i;
		}
	}
	return -1;
}

struct Values getValues(struct VarScope* varScope, struct KeyPos* keyPosition, struct KeyWord* keyWords, unsigned int stop, unsigned int index){
	int numberOfValues = 0;

	for (int i = index; i < stop; i++){
		numberOfValues++;
		if (i >= stop-1){
			continue;
		}
		if (keyPosition[i+1].key == FuncCallStart_k){
			unsigned int next = getParenthesesEnd(keyPosition, keyWords, stop, i+2);
			i = next;
		}
	}

	struct Values values;

	values.vars = (struct Var**)malloc(numberOfValues*sizeof(struct Var*));
	values.length = numberOfValues;

	
	int varIndex = 0;
	for (int i = index; i < stop; i++){
		if (i < stop-1){
			if (keyPosition[i+1].key == FuncCallStart_k){
				unsigned int newStop = getParenthesesEnd(keyPosition, keyWords, stop, i+2);
				values.vars[varIndex] = evaluateExpression(varScope, keyPosition, keyWords, newStop, i+1);
				varIndex++;
				i = newStop;
				continue;
			}
		}

		struct Var* result = generateVarFromString(keyWords[i+1].value, strlen(keyWords[i+1].value));
		if (!result->numberOfTypes){
			freeVar(result);
			free(result);
			result = copyVar(getVarFromScope(varScope, keyWords[i+1].value));
		}
		values.vars[varIndex] = result;
		varIndex++;
	}

	return values;
};

struct Var* evaluateExpression(struct VarScope* varScope, struct KeyPos* keyPosition, struct KeyWord* keyWords, unsigned int stop, unsigned int index){
	struct Var* result;

	unsigned int newIndex = index;

	struct Values values = getValues(varScope, keyPosition, keyWords, stop, index);

	unsigned int numberOfOperators = values.length;

	struct Operator** operators = (struct Operator**)malloc(numberOfOperators*sizeof(struct Operator*));

	for (int i = 0; i < numberOfOperators; i++){
		struct KeyPos* keyPos = &keyPosition[newIndex+i];
		struct Operator* operator = (struct Operator*)malloc(sizeof(struct Operator));

		if (i){
			if (keyPos->key == FuncCallStart_k){
				unsigned int end = getParenthesesEnd(keyPosition, keyWords, stop, newIndex+i+1);
				newIndex = end-i+1;
				keyPos = &keyPosition[newIndex+i];
			}
		}
		operator->type = keyPos->key;

		if (i){
			operator->leftVar = values.vars[i-1];
		} else {
			operator->leftVar = NULL;
		}
		operator->rightVar = values.vars[i];

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
			case (MultiplicationAssign_k): {
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
	}

	freeOperator(sortedOperators, numberOfOperators);
	freeValues(&values);
	freeResults(results, numberOfOperators);
	return result;
}

struct Function* getFunction(struct KeyPos* keyPosition, struct KeyWord* keyWords, unsigned int stop, unsigned int index){
	struct Function* function = (struct Function*)malloc(sizeof(struct Function));

	if (!strcmp(keyPosition[index+1].name, "function")){
		raiseError("No function found\n", 1);
	}

	return function;
}

int interpretLine(struct KeyChars keyChars, struct Body* body, char* line, unsigned int length){

	// find keys

	unsigned int keysCount = 0;
	int start = 0;
	while (start < length){
		struct KeyPos keyPos = getNextKey(keyChars, line, length, start);
		if (keyPos.key == -1){
			break;
		}

		keysCount++;
		start = keyPos.endPos;
	}
	
	struct KeyPos* keyPositions = (struct KeyPos*)malloc(keysCount*sizeof(struct KeyPos));

	start = 0;
	int currentKey = 0;
	while (start < length){
		struct KeyPos keyPos = getNextKey(keyChars, line, length, start);
		if (keyPos.key == -1){
			break;
		}
		keyPositions[currentKey] = keyPos;
		currentKey++;
		start = keyPos.endPos;
	}

	// find keywords

	struct KeyWord* keyWords = (struct KeyWord*)malloc((keysCount+1)*sizeof(struct KeyWord));

	unsigned int currentKeyWord = 0;
	unsigned int lastPosition = 0;
	for (int i = 0; i < keysCount; i++){
		struct KeyPos* position = &keyPositions[i];
		struct KeyWord keyWord = getKeyword(line, lastPosition, position->pos);
		keyWords[currentKeyWord] = keyWord;
		currentKeyWord++;
		lastPosition = position->endPos;
	}
	struct KeyWord keyWord = getKeyword(line, lastPosition, length);
	keyWords[currentKeyWord] = keyWord;

	unsigned int keyLoc = 0;
	unsigned int wordLoc = 0;

	unsigned int newVar = 0;
	struct Var* newVarP;

	// now parse the keywords with the keys

	for (int i = 0; i < keysCount; i++){
		struct KeyPos* key = &keyPositions[i];
		struct KeyWord* keyWord = &keyWords[i];

		switch (key->key){
			case (NewVar_k):{
				//allocate new var in the scope

				char* varName = keyWord->value;

				unsigned int increment = 0;

				struct Var* var = getVarTypes(varName, keyPositions, keyWords, keysCount, i, &increment);
				i += increment-1;

				newVar = 1;
				newVarP = var;

				printf("new var %s %d\n", var->name, var->hasParam);

				if (var->hasParam){
					printf("function %d %d\n", var->param->inputCount, var->param->returnValue->numberOfTypes);
				}
				
				addVarToScope(&body->globalScope, var); 
			}break;

			case (Assign_k): {
				
				struct Var* value = evaluateExpression(&body->globalScope, keyPositions, keyWords, keysCount, i);
				printf("result %s\n", value->value);

				//assign value to current variable
				if (newVar){
					assignValue(newVarP, value);
					//printf("assign %s\n", keyWord->value);
				} else {
					//get var from scope
					struct Var* leftVar = getVarFromScope(&body->globalScope, keyWord->value);
					assignValue(leftVar, value);
				}

				freeVar(value);
				free(value);
			}break;

			case (Function_k): {
				// get function
				struct Function* function = getFunction(keyPositions, keyWords, keysCount, i);
			}
		}
	}

	//printf("%s\n", body->globalScope.currentVar->name);

	for (int i = 0; i < currentKeyWord; i++){
		free(keyWords[i].value);
	}
	free(keyWords);

	free(keyPositions);

	printf("%s\n", line);
	return 0;
}

struct Body interpretBody(struct KeyChars keyChars, struct File file, unsigned long int start, unsigned long int end){

	struct Body body;

	body.globalScope.hasCurrentVar = 0;
	body.globalScope.numberOfVars = 0;
	body.globalScope.vars = (struct Var**)malloc(sizeof(struct Var*));

	struct DefinitionLines lines = getLines(file.mem, start, end);

	for (int i = 0; i < lines.length; i++){
		interpretLine(keyChars, &body, lines.lines[i].value, lines.lines[i].length);
		free(lines.lines[i].value);
	}

	free(lines.lines);

	return body;
}