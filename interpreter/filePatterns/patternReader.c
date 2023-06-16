#include "patternReader.h"

// expensive opperation when used improperly
unsigned long int find_next(char*data, unsigned long int end, char*key, unsigned int keyLength, struct ProcessState* processState){

	for (unsigned long int i = 0; i < end - keyLength; i++){
		int match = 1;
		for (int j = 0; j < keyLength; j++){
			//wprintf(L"compairing %lc with %lc\n", data[i+j], key[j]);
			if (data[i+j] != key[j]){
				match = 0;
			}
			//if (match) wprintf(L"they were equal\n");
		}
		if (match){
			return i;
		}
	}

	return 0;
}

unsigned long int count_char_in_range(char*string, unsigned int start, unsigned int stop, char key, struct ProcessState* processState){
	unsigned long int count = 0;
	for (unsigned int i = start; i < stop; i++){
		if (string[i] == key){
			count++;
		}
	}
	return count;
}

int contains(char*string, char key, unsigned int length, struct ProcessState* processState){
	for (int i = 0; i < length; i++){
		if (string[i] == key){
			return 1;
		}
	}
	return 0;
}

unsigned int find_next_char(char*string, char* keys, unsigned int length, unsigned int keyLength, struct ProcessState* processState){
	for (unsigned int i = 0; i < length; i++){
		for (int j = 0; j < keyLength; j++){
			if (string[i] == keys[j]){
				return i;
			}
		}
	}
	return 0;
}

struct PatternRange get_pattern_by_key(struct File file, unsigned long int start, char*key, struct ProcessState* processState){
	struct PatternRange patternRange;

	char*data = file.mem + start;
	
	unsigned int keyLength = strlen(key);

	patternRange.start = find_next(data, file.length - start, key, keyLength, processState) + start;

	patternRange.end = find_next(data + (patternRange.start + keyLength), file.length - (patternRange.start + keyLength) - start, key, keyLength, processState) + start + patternRange.start + keyLength*2;

	return patternRange;

}

unsigned int find_next_line(struct File file, unsigned long int start, struct ProcessState* processState){
	unsigned int length = 0;
	while (1){
		if (file.mem[start + length] == '\n'){
			return length;
		}
		length++;
	}

	return 0;
}