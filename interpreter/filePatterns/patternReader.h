#ifndef PATTERN_READER_H
#define PATTERN_READER_H

#include "../fileReader/fileReader.h"
#include <string.h>

struct PatternRange{
	unsigned long int start;
	unsigned long int end;
};

unsigned int findNextLine(struct File file, unsigned long int start);
unsigned long int findNext(char*data, unsigned long int end, char*key, unsigned int keyLength);
struct PatternRange getPatternByKey(struct File file, unsigned long int start, char*key);

#endif