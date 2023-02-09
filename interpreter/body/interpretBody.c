#include "interpretBody.h"

// sussy algorithm
unsigned int getNewline(struct File file, unsigned long int* start, unsigned long int end){
	unsigned int length = 0;
	unsigned int brackets = 0;
	while (*start < end){
		if (file.mem[*start] == '{'){
			brackets++;
		} else if (file.mem[*start] == '}'){
			brackets--;
		}
		if ((file.mem[*start] == '\n' || *start == end-1) && !brackets){
			if (length){
				int shouldAdd = 0;
				for (int i = *start-length; i < *start; i++){
					if (file.mem[i] != ' ' && file.mem[i] != '\n'){
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

struct DefinitionLines getLines(struct File file, unsigned long int start, unsigned long int end){
	struct DefinitionLines lines;

	char* line;
	unsigned int length = 0;

	unsigned int numberOfLines = 0;

	unsigned long int newStart = start;
	
	while (newStart < end){
		if (getNewline(file, &newStart, end)){
			numberOfLines++;
		}
		newStart++;
	}

	lines.length = numberOfLines;
	lines.lines = (struct DefinitionLine*)malloc(numberOfLines*sizeof(struct DefinitionLine));

	unsigned int lineCount = 0;
	newStart = start;

	while (newStart < end){
		length = getNewline(file, &newStart, end);
		if (length){
			lines.lines[lineCount].length = length+2;
			lines.lines[lineCount].value = (char*)malloc((length+2)*sizeof(char));
			memcpy(lines.lines[lineCount].value, file.mem + newStart-length, length+1);
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

	for (int index = start; index < length; index++){

		if (line[index] == '"'){
			isString = !isString;
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
	for (int i = start; i < stop; i++){
		if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n'){
			charCount++;
		}
	}

	keyWord.value = (char*)malloc((charCount+1)*sizeof(char));

	unsigned int count = 0;
	for (int i = start; i < stop; i++){
		if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n'){
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
	}
	printf("no type named %s\n", value);
	raiseError("", 1);
	return -1;
}

struct Var* getVarTypes(char* varName, struct KeyPos* keyPosition, struct KeyWord* keyWords, unsigned int length, unsigned int index){
	int count = 0;

	struct Var* var = (struct Var*)malloc(sizeof(struct Var));

	unsigned int newIndex = index+1;
	while (newIndex < length){ // itterate keys
		struct KeyPos* keyPos = &keyPosition[newIndex];
		struct KeyWord* keyWord = &keyWords[newIndex];
		count++;
		if (keyPos->key != Type_k && keyPos->key != FuncTypeStart_k && keyPos->key != FuncTypeEnd_k){
			break;
		}
		newIndex++;
	}

	int* codes = (int*)malloc((count)*sizeof(int));

	char* param;

	unsigned int i = 0;
	newIndex = index+1;
	while (newIndex < length){ // itterate keys
		struct KeyPos* keyPos = &keyPosition[newIndex];
		struct KeyWord* keyWord = &keyWords[newIndex];
		codes[i] = typeFromString(keyWord->value, keyWord->length);

		printf("%d %d types\n", keyPos->key, codes[i]);

		if (codes[i] == Function_k){
			
		}

		i++;
		if (keyPos->key != Type_k){
			break;
		}
		newIndex++;
	}

	return var;
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

	// now parse the keywords with the keys
	for (int i = 0; i < keysCount; i++){
		struct KeyPos* key = &keyPositions[keyLoc];
		struct KeyWord* keyWord = &keyWords[wordLoc];

		switch (key->key){
			case (NewVar_k):{
				//allocate new var in the scope

				char* varName = keyWord->value;
				struct Var* var = getVarTypes(varName, keyPositions, keyWords, keysCount, i);

				unsigned int newVar = 1;
				
				addVarToScope(&body->globalScope, var); 
			}break;

			case (Assign_k): {
				//assign value to current variable
			}break;

			case (FuncCallEnd_k): {
			}

			case (FuncTypeEnd_k): {
				wordLoc--;
			}
		}

		keyLoc++;
		wordLoc++;
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
	body.globalScope.vars = (struct Var*)malloc(sizeof(struct Var));

	struct DefinitionLines lines = getLines(file, start, end);

	for (int i = 0; i < lines.length; i++){
		interpretLine(keyChars, &body, lines.lines[i].value, lines.lines[i].length);
		free(lines.lines[i].value);
	}

	free(lines.lines);

	return body;
}