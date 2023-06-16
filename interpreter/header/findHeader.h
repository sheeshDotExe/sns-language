#ifndef FIND_HEADER_H
#define FIND_HEADER_H

#include "../fileReader/fileReader.h"
#include "../filePatterns/patternReader.h"
#include "./headerFunctions.h"
#include "headerStructs.h"

struct HeaderLines{
	char**lines;
	unsigned int numberOfLines;
};


struct Arguments {
	char**argv;
	unsigned int argc;
};


// get the options of the file between *** ***
void set_default_header_options(struct HeaderOptions* headerOptions, struct ProcessState* processState);
void interprete_header_line(struct HeaderOptions* headerOptions, struct HeaderAtlas* headerAtlas, char*command, unsigned int length, struct ProcessState* processState);
struct HeaderLines read_header_data(struct File file, unsigned long int start, unsigned long int end, struct ProcessState* processState);
struct HeaderOptions* get_header_options(struct File file, struct ProcessState* processState);

#endif