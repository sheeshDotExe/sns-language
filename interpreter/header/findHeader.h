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
void setDefaultHeaderOptions(struct HeaderOptions* headerOptions);
void interpreteHeaderLine(struct HeaderOptions* headerOptions, struct HeaderAtlas* headerAtlas, char*command, unsigned int length);
struct HeaderLines readHeaderData(struct File file, unsigned long int start, unsigned long int end);
struct HeaderOptions getHeaderOptions(struct File file);

#endif