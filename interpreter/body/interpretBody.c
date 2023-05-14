#include "interpretBody.h"

// sussy algorithm
unsigned int getNewline(char* mem, unsigned long int* start, unsigned long int end){
	unsigned int length = 0;
	unsigned int brackets = 0;
	int isString = 0;
	while (*start < end){
		if (mem[*start] == '"' || mem[*start] == '\'') {
			isString = !isString;
		}
		if (mem[*start] == '{' && !isString){
			brackets++;
		} else if (mem[*start] == '}' && !isString){
			brackets--;
		}
		if ((mem[*start] == '\n' || *start == end-1) && !brackets && !isString){
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

struct DefinitionLines* getLines(char* mem, unsigned long int start, unsigned long int end){
	struct DefinitionLines* lines = (struct DefinitionLines*)malloc(sizeof(struct DefinitionLines));

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

	lines->length = numberOfLines;
	lines->lines = (struct DefinitionLine*)malloc(numberOfLines*sizeof(struct DefinitionLine));

	unsigned int lineCount = 0;
	newStart = start;

	while (newStart < end){
		length = getNewline(mem, &newStart, end);
		if (length){
			lines->lines[lineCount].length = length + 2;
			lines->lines[lineCount].value = (char*)malloc((length+2)*sizeof(char));
			memcpy(lines->lines[lineCount].value, mem + newStart-length, length+1);
			lines->lines[lineCount].value[length+1] = '\0';
			lineCount++;
		}
		newStart++;
	}
	
	return lines;
}

unsigned int getNextChar(char* string, unsigned int stop, unsigned int start){
	for (int i = start; i < stop; i++){
		if (string[i] == ' ' || string[i] == '\t'){
			continue;
		}
		return i;
	}
	return stop;
}

struct KeyPos* getNextKey(struct KeyChars keyChars, char* line, unsigned int length, unsigned int start){

	struct KeyPos* keyPos = (struct KeyPos*)malloc(sizeof(struct KeyPos));

	int isString = 0;
	int hasFunction = 0;
	int bracketCount = 0;
	int hasSpace = 0;
	int stopAt = length;
	int startAt = getNextChar(line, length, 0);
	int foundAt = 0;

	for (int index = start; index < length; index++){

		if (line[index] == '{' && !isString){
			bracketCount++;
			hasFunction = 1;
			continue;
		} else if (line[index] == '}' && !isString){
			bracketCount--;
			continue;
		}

		if (bracketCount){
			continue;
		}
		
		if (line[index] == '"' || line[index] == '\''){
			isString = !isString;
		}

		if ((line[index] == ' ' || line[index] == '\t') && index > startAt && !isString){
			hasSpace = 1;
			foundAt = index;
			stopAt = getNextChar(line, length, index);
			//printf("space: %d\n", index);
		}

		if (hasFunction){
			keyPos->pos = index;
			keyPos->endPos = index;
			keyPos->key = FuncStart_k;
			keyPos->name = strdup("function");
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
				keyPos->pos = index;
				unsigned int nextStart = getNextChar(line, length, index + (key->length-1));
				//printf("key found: %d\n", nextStart);
				keyPos->endPos = nextStart;
				keyPos->key = key->key;
				keyPos->name = (char*)malloc((strlen(key->name)+1)*sizeof(char));
				memcpy(keyPos->name, key->name, strlen(key->name)+1);
				return keyPos;
			}
		}

		if (index == stopAt){
			break;
		}
		
	}

	if (hasSpace && !isString) {
		keyPos->pos = foundAt;
		keyPos->endPos = stopAt;
		keyPos->key = SplitBySpace;
		keyPos->name = strdup("none");
		//printf("space split %d\n", keyPos.endPos);
		return keyPos;
	}

	keyPos->pos = start;
	keyPos->endPos = start;
	keyPos->name = strdup("error");
	keyPos->key = -1;
	return keyPos;
}

struct KeyWord* getKeyword(char* line, unsigned int start, unsigned int stop){
	//printf("new\n");
	struct KeyWord* keyWord = (struct KeyWord*)malloc(sizeof(struct KeyWord));

	unsigned int charCount = 0;
	int isFunction = 0;
	int isString = -1;

	for (int i = start; i < stop; i++){
		if (line[i] == '"' || line[i] == '\''){
			isString = isString*-1;
		}
		//printf("%c %d\n", line[i], isString);

		if (isString == -1){
			if (line[i] == '{'){
				isFunction = 1;
				start = i+1;
				stop--;
				break;
			}
		}
	}

	isString = 0;

	for (int i = start; i < stop; i++){
		if (line[i] == '"' || line[i] == '\'') isString = !isString;

		if (isFunction || (line[i] != ' ' && line[i] != '\t' && line[i] != '\n') || isString){
			charCount++;
		}
	}

	keyWord->value = (char*)malloc((charCount+1)*sizeof(char));

	isString = 0;

	unsigned int count = 0;
	for (int i = start; i < stop; i++){
		if (line[i] == '"' || line[i] == '\'') isString = !isString;

		if (isFunction || (line[i] != ' ' && line[i] != '\t' && line[i] != '\n') || isString){
			keyWord->value[count] = line[i];
			count++;
		}
	}

	keyWord->value[charCount] = '\0';
	keyWord->length = charCount;

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

struct Var* getVarTypes(char* varName, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int length, unsigned int index, unsigned int* increment){
	int count = 1;

	struct Var* var;

	unsigned int newIndex = index + 1;
	while (newIndex < length){ // itterate keys
		struct KeyPos* keyPos = keyPosition[newIndex];
		struct KeyWord* keyWord = keyWords[newIndex];
		if (keyPos->key != Type_k){
			break;
		}
		count++;
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
			struct KeyWord* keyWord = keyWords[newIndex];
			codes[i] = typeFromString(keyWord->value, keyWord->length);
			break;
		}

		struct KeyPos* keyPos = keyPosition[newIndex];
		struct KeyWord* keyWord = keyWords[newIndex];
		codes[i] = typeFromString(keyWord->value, keyWord->length);

		if (codes[i] == Function_c){
			(*increment)++;
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
				currentKey = keyPosition[newIndex+nCount];
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

			struct KeyWord* currentWord = keyWords[paramStart];

			while (currentKey->key != FuncTypeEnd_k){
				if (currentKey->key == Param_k){
					paramCount--;
					if (paramCount){
						param->inputVars[paramCounter] = getVarTypes(keyWords[paramEnd+1]->value, keyPosition, keyWords, length, paramEnd+1, increment);
						paramCounter++;
					} else {
						break;
					}
					paramEnd = paramStart;
				}
				paramStart++;
				currentKey = keyPosition[paramStart];
				currentWord = keyWords[paramStart];
			}
			param->returnValue = getVarTypes(strdup("return"), keyPosition, keyWords, length, paramEnd, increment);
			hasParam = 1;
		}
		i++;
		if (keyPos->key != Type_k){
			break;
		}
		newIndex++;
		(*increment)++;
	}
	var = generateVar(codes, count, varName, "", param);
	var->hasParam = hasParam;
	free(codes);
	return var;
}


int interpretKeyWord(struct State* state, struct KeyPos** keyPosition, struct KeyWord** keyWords, unsigned int stop, unsigned int index){
	struct KeyWord* keyWord = keyWords[index];

	char* key = keyWord->value;

	if (!strcmp(key, "return")){
		struct Var* value = evaluateExpression(state, keyPosition, keyWords, stop, index+1);
		struct Var* returnValue = getVarFromScope(state->localScope, "return");
		assignValue(returnValue, value);
		freeVar(value);
		free(value);
		return Return_s;
	}

	printf("Invalid syntax: %s\n", key);
	raiseError("", 1);
	return 1;
}

void freeLines(struct KeyPos** keyPositions, struct KeyWord** keyWords, unsigned int count){
	for (int i = 0; i < count+1; i++){
		free(keyWords[i]->value);
		free(keyWords[i]);
	}
	for (int i = 0; i < count; i++){
		free(keyPositions[i]->name);
		free(keyPositions[i]);
	}
	free(keyWords);
	free(keyPositions);
}

unsigned int getKeysCount(struct State* state, char* line, unsigned int length, unsigned int start){
	unsigned int keysCount = 0;
	while (start < length){
		struct KeyPos* keyPos = getNextKey(state->keyChars, line, length, start);
		free(keyPos->name);
		if (keyPos->key == -1){
			free(keyPos);
			break;
		}

		keysCount++;
		start = keyPos->endPos;
		free(keyPos);
	}
	return keysCount;
}

struct KeyPos** getKeyPositions(unsigned int keysCount, struct State* state, char* line, unsigned int length, unsigned int start){
	struct KeyPos** keyPositions = (struct KeyPos**)malloc(keysCount*sizeof(struct KeyPos*));
	struct KeyPos* keyPos;
	int currentKey = 0;
	while (start < length){
		keyPos = getNextKey(state->keyChars, line, length, start);
		if (keyPos->key == -1){
			free(keyPos->name);
			free(keyPos);
			break;
		}

		keyPositions[currentKey] = keyPos;
		currentKey++;
		start = keyPos->endPos;
	}
	return keyPositions;
}

struct KeyWord** getKeyWords(unsigned int keysCount, struct KeyPos** keyPositions, struct State* state, char* line, unsigned int length){
	struct KeyWord** keyWords = (struct KeyWord**)malloc((keysCount+1)*sizeof(struct KeyWord*));

	unsigned int currentKeyWord = 0;
	unsigned int lastPosition = 0;
	for (int i = 0; i < keysCount; i++){
		struct KeyPos* position = keyPositions[i];
		struct KeyWord* keyWord = getKeyword(line, lastPosition, position->pos);

		keyWords[currentKeyWord] = keyWord;
		currentKeyWord++;
		lastPosition = position->endPos;
	}
	struct KeyWord* keyWord = getKeyword(line, lastPosition, length);
	keyWords[currentKeyWord] = keyWord;
	return keyWords;
}

int interpretLine(struct State* state, char* line, unsigned int length){

	unsigned int keysCount = getKeysCount(state, line, length, 0);
	
	struct KeyPos** keyPositions = getKeyPositions(keysCount, state, line, length, 0);

	// find keywords

	struct KeyWord** keyWords = getKeyWords(keysCount, keyPositions, state, line, length);

	unsigned int keyLoc = 0;
	unsigned int wordLoc = 0;

	unsigned int newVar = 0;
	struct Var* newVarP;

	// now parse the keywords with the keys

	for (int i = 0; i < keysCount; i++){
		struct KeyPos* key = keyPositions[i];
		struct KeyWord* keyWord = keyWords[i];

		switch (key->key){
			case (NewVar_k):{
				//allocate new var in the scope

				char* varName = keyWord->value;

				unsigned int increment = 0;

				struct Var* var = getVarTypes(varName, keyPositions, keyWords, keysCount, i, &increment);
				i += increment-1;

				newVar = 1;
				newVarP = var;
				
				addVarToScope(state->localScope, var); 
			}break;

			case (Assign_k): {
				struct Var* value = evaluateExpression(state, keyPositions, keyWords, keysCount, i+1);

				//assign value to current variable
				if (newVar){
					assignValue(newVarP, value);
					//printf("assign %s\n", keyWord->value);
				} else {
					//get var from scope
					struct Var* leftVar = getVarFromScopes(state->localScope, state->globalScope, keyWord->value);
					assignValue(leftVar, value);
				}

				freeVar(value);
				free(value);

				i = keysCount;
			}break;

			case (Function_k): {
				// get function
				if (newVar){
					struct Function* function = getFunction(newVarP, state, keyPositions, keyWords, keysCount, i);
					newVarP->function = function;
					newVarP->hasFunction = 1;
				}
			} break;

			case (FuncCallStart_k): {
				//printf("function call %s\n", keyWord->value);

				int builtinPos = isBuiltin(state->builtins, keyWord->value);
				if (builtinPos){
					struct BuiltinFunction* function = getBuiltin(state->builtins, builtinPos);
					getSetParams(function->params, state, keyPositions, keyWords, keysCount, i);
					struct Var* returnValue = function->function(function->params, state);

					freeParam(function->params);
					function->params = copyParam(function->originalParams);

					if (!strcmp(function->name, "route")){
						newVar = 1;
						newVarP = returnValue;
					}
					break;
				}
				struct Var* var = getVarFromScopes(state->localScope, state->globalScope, keyWord->value);
				getSetParams(var->param, state, keyPositions, keyWords, keysCount, i);
				var->function->varScope = createVarScope(var);
				struct State* copiedState = copyState(state);
				struct Var* returnValue = callFunction(var, copiedState);

				freeParam(var->param);
				var->param = copyParam(var->originalParam);

				free(copiedState);

				freeVar(returnValue);
				free(returnValue);

				freeVarScope(var->function->varScope);
				
			} break;
			
			case (SplitBySpace): {
				//abstract abstract abstract abstract abstrct ab scrtr asctrat
				int code = interpretKeyWord(state, keyPositions, keyWords, keysCount, i);
				if (code == Return_s){
					freeLines(keyPositions, keyWords, keysCount);
					return 1;
				}
			}
			
		}
	}

	freeLines(keyPositions, keyWords, keysCount);

	return 0;
}

struct Body* interpretBody(struct State* state, struct File file, unsigned long int start, unsigned long int end){

	struct Body* body = (struct Body*)malloc(sizeof(struct Body));

	body->hasMain = 0;

	body->globalScope.hasCurrentVar = 0;
	body->globalScope.numberOfVars = 0;
	body->globalScope.vars = (struct Var**)malloc(sizeof(struct Var*));

	struct DefinitionLines* lines = getLines(file.mem, start, end);

	state->globalScope = &body->globalScope;
	state->localScope = &body->globalScope;

	for (int i = 0; i < lines->length; i++){
		interpretLine(state, lines->lines[i].value, lines->lines[i].length);
		free(lines->lines[i].value);
	}

	free(lines->lines);
	free(lines);

	if (varExistsInScope(state->globalScope, "main")){
		struct Var* main = getVarFromScope(state->globalScope, "main");
		if (main->hasFunction) body->hasMain = 1;
	}

	return body;
}