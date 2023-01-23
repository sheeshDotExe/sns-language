#include "interpreter.h"

#define CHUNK_SIZE 200

int interpret(FILE*file){

	fseek(file, 0, SEEK_END);
	long unsigned int length = ftell(file);

	fseek(file, 0, SEEK_SET);

	unsigned char*fileData = (unsigned char*)malloc(length*sizeof(unsigned char));

	long unsigned int readBytes = 0;

	while(1){
		unsigned int read = fread(fileData + readBytes, sizeof(unsigned char), CHUNK_SIZE, file);
		if (read){
			readBytes += read;
		} else {
			break;
		}
	}

	fileData[readBytes] = '\0';


	printf("%s", fileData);

	char buf[100];
	gets(buf);

	return 0;
}