#ifndef FILE_READER_H
#define FILE_READER_H

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "../processState.h"

struct File
{
	unsigned char*mem;
	long unsigned int length;
};

struct File read_file(FILE*fileHandle, struct ProcessState* processState);

#endif