#ifndef FILE_READER_H
#define FILE_READER_H

#include <stdio.h>
#include <stdlib.h>

struct File
{
	char*mem;
	long unsigned int length;
};

struct File readFile(FILE*fileHandle);

#endif