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

struct LineInfo getLineInfo(char* line, unsigned int length){
	
}

struct Body interpretBody(struct File file, unsigned long int start, unsigned long int end){

	struct Body body;

	struct DefinitionLines lines = getLines(file, start, end);

	for (int i = 0; i < lines.length; i++){
		printf("%s\n", lines.lines[i]);
	}

	return body;
}