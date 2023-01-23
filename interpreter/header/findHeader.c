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

struct HeaderOptions getHeaderOptions(struct File file){
	struct HeaderOptions headerOptions;

	struct PatternRange headerLocation = getPatternByKey(file, 0, "***");

	headerOptions.headerStart = headerLocation.start;
	headerOptions.headerEnd = headerLocation.end;

	struct HeaderLines headerLines = readHeaderData(file, headerLocation.start + 3, headerLocation.end - 3);

	return headerOptions;
};