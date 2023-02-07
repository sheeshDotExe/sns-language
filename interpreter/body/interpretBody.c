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
		if (line[i] != ' ' && line[i] != '\t'){
			charCount++;
		}
	}

	keyWord.value = (char*)malloc((charCount+1)*sizeof(char));

	unsigned int count = 0;
	for (int i = start; i < stop; i++){
		if (line[i] != ' ' && line[i] != '\t'){
			keyWord.value[count] = line[i];
			count++;
		}
	}

	keyWord.value[charCount] = '\0';
	keyWord.length = charCount;

	return keyWord;
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
		keyPos.before = (struct KeyWord*)malloc(sizeof(struct KeyWord));
		keyPos.after = (struct KeyWord*)malloc(sizeof(struct KeyWord));
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

	// now parse the keywords with the keys
	for (int i = 0; i < keysCount; i++){
		struct KeyPos* key = &keyPositions[i];
		key->before = &keyWords[i];
		key->after = &keyWords[i+1];

		printf("%d %s %s\n", key->key, key->before->value, key->after->value);
	}


	for (int i = 0; i < currentKeyWord; i++){
		free(keyWords[i].value);
	}
	free(keyWords);

	for (int i = 0; i < keysCount; i++){
		free(keyPositions[i].before);
		free(keyPositions[i].after);
	}
	free(keyPositions);

	printf("%s\n", line);
	return 0;
}

struct Body interpretBody(struct KeyChars keyChars, struct File file, unsigned long int start, unsigned long int end){

	struct Body body;

	struct DefinitionLines lines = getLines(file, start, end);

	for (int i = 0; i < lines.length; i++){
		interpretLine(keyChars, &body, lines.lines[i].value, lines.lines[i].length);
		free(lines.lines[i].value);
	}

	free(lines.lines);

	return body;
}