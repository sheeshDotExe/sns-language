#ifndef PATTERN_READER_H
#define PATTERN_READER_H

#include "../fileReader/fileReader.h"
#include <string.h>

struct PatternRange{
	unsigned long int start;
	unsigned long int end;
};

unsigned long int count_char_in_range(char*string, unsigned int start, unsigned int stop, char key, struct ProcessState* processState);
unsigned int find_next_char(char*string, char* keys, unsigned int length, unsigned int keyLength, struct ProcessState* processState);
int contains(char*string, char key, unsigned int length, struct ProcessState* processState);
unsigned int find_next_line(struct File file, unsigned long int start, struct ProcessState* processState);
unsigned long int find_next(char*data, unsigned long int end, char*key, unsigned int keyLength, struct ProcessState* processState);
struct PatternRange get_pattern_by_key(struct File file, unsigned long int start, char*key, struct ProcessState* processState);

#endif