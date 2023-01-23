#include "findHeader.h"

int readHeaderData(struct File file, unsigned long int start, unsigned long int end){

	while (1){
		unsigned int lineLength = findNextLine(file, start);
		printf("%d\n", lineLength);
		start += lineLength+1;

		if (start >= end){
			break;
		}
	}

	return 0;
}

struct HeaderOptions getHeaderOptions(struct File file){
	struct HeaderOptions headerOptions;

	struct PatternRange headerLocation = getPatternByKey(file, 0, "***");

	headerOptions.headerStart = headerLocation.start;
	headerOptions.headerEnd = headerLocation.end;

	readHeaderData(file, headerLocation.start + 3, headerLocation.end - 3);

	return headerOptions;
};