#include "fileReader.h"

#define CHUNK_SIZE 200

int _read_chunk(FILE*fileHandle, unsigned char*buffer, long unsigned int*readBytes, struct ProcessState* processState){
	unsigned int read = fread(buffer + *readBytes, sizeof(unsigned char), CHUNK_SIZE, fileHandle);
	if (read){
		*readBytes += read;
		return 1;
	} else {
		return 0;
	}
}

struct File read_file(FILE*fileHandle, struct ProcessState* processState){
	struct File file;

	fseek(fileHandle, 0, SEEK_END);
	long unsigned int length = ftell(fileHandle);

	fseek(fileHandle, 0, SEEK_SET);

	unsigned char* fileData = (unsigned char*)malloc((length+1)*sizeof(unsigned char));

	long unsigned int readBytes = 0;

	while(_read_chunk(fileHandle, fileData, &readBytes, processState)){
	}

	fileData[readBytes] = '\0';

	file.mem = fileData;
	file.length = readBytes;

	return file;
};
