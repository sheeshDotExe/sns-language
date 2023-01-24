#include "findHeader.h"

struct HeaderLines readHeaderData(struct File file, unsigned long int start, unsigned long int end){

	struct HeaderLines headerLines;

	unsigned int numberOfLines = 0;
	unsigned long int readLines = 0;

	while (1){
		unsigned int lineLength = findNextLine(file, start + readLines);
		readLines += lineLength+1;

		if (lineLength){
			numberOfLines += 1;
		}

		if (start + readLines >= end){
			break;
		}
	}

	headerLines.lines = (char**)malloc(numberOfLines*sizeof(char*));
	headerLines.numberOfLines = numberOfLines;

	unsigned int lineNumber = 0;
	readLines = 0;

	while (1){
		unsigned int lineLength = findNextLine(file, start + readLines);
		readLines += lineLength+1;

		if (lineLength){

			headerLines.lines[lineNumber] = (char*)malloc((lineLength+1)*sizeof(char));
			unsigned int j = 0;
			for (unsigned int i = start + readLines - (lineLength+1); i < start + readLines - 1; i++){
				headerLines.lines[lineNumber][j] = file.mem[i];
				j++;
			}
			headerLines.lines[lineNumber][j] = '\0';
			lineNumber++;
		}

		if (start + readLines >= end){
			break;
		}
	}

	return headerLines;
}


void interpreteHeaderLine(struct HeaderOptions* headerOptions, struct HeaderAtlas* headerAtlas, char*command, unsigned int length){
	// get type
	if (contains(command, '(', length)){
		unsigned long int keywordEnd = findNextChar(command, " (", length, 2);
		if (keywordEnd){
			char*keyword = (char*)malloc(keywordEnd*sizeof(char));
			for (int i = 0; i < keywordEnd; i++){
				keyword[i] = command[i];
			}
			keyword[keywordEnd]='\0';

			// get arguments

			interpreteHeaderFunction(headerOptions, headerAtlas, keyword, (char**)NULL, keywordEnd, 0);
		}
	} //....
}

struct HeaderOptions getHeaderOptions(struct File file){
	struct HeaderOptions headerOptions;

	struct PatternRange headerLocation = getPatternByKey(file, 0, "***");

	headerOptions.headerStart = headerLocation.start;
	headerOptions.headerEnd = headerLocation.end;

	struct HeaderLines headerLines = readHeaderData(file, headerLocation.start + 3, headerLocation.end - 3);

	struct HeaderAtlas headerAtlas = getFunctionMap();

	for (int i = 0; i < headerLines.numberOfLines; i++){
		unsigned int length = strlen(headerLines.lines[i]);
		interpreteHeaderLine(&headerOptions, &headerAtlas, headerLines.lines[i], length);
	}

	return headerOptions;
};