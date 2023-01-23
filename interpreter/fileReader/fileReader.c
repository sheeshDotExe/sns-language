#include "fileReader.h"

#define CHUNK_SIZE 200

int _readChunk(FILE*fileHandle, unsigned char*buffer, long unsigned int*readBytes){
	unsigned int read = fread(buffer + *readBytes, sizeof(unsigned char), CHUNK_SIZE, fileHandle);
	if (read){
		*readBytes += read;
		return 1;
	} else {
		return 0;
	}
}

struct File readFile(FILE*fileHandle){
	struct File file;

	fseek(fileHandle, 0, SEEK_END);
	long unsigned int length = ftell(fileHandle);

	fseek(fileHandle, 0, SEEK_SET);

	unsigned char*fileData = (unsigned char*)malloc(length*sizeof(unsigned char));

	long unsigned int readBytes = 0;

	while(_readChunk(fileHandle, fileData, &readBytes)){
	}

	fileData[readBytes] = '\0';

	file.mem = fileData;
	file.length = length;

	return file;
};
