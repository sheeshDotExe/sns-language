#include "headerFunctions.h"

void addFunction(struct FunctionMap* functionMap, int index, char*name, int(*function)(struct HeaderOptions* headerOptions, char**args, int argc)){
	unsigned int nameLength = strlen(name);
	functionMap[index].name = (char*)malloc(nameLength*sizeof(char));
	functionMap[index].nameLength = nameLength;
	for (int i = 0; i < nameLength; i++){
		functionMap[index].name[i] = name[i];
	}
	functionMap[index].function = function;
}

int _UseLocalHost(struct HeaderOptions* headerOptions, char**args, int argc){
	return 0;
}

int _UsePort(struct HeaderOptions* headerOptions, char**args, int argc){
	return 0;
}

int _DebugMode(struct HeaderOptions* headerOptions, char**args, int argc){
	return 0;
}

int _ForceSSL(struct HeaderOptions* headerOptions, char**args, int argc){
	return 0;
}

int _UseSSL(struct HeaderOptions* headerOptions, char**args, int argc){
	return 0;
}

int _MaxConnections(struct HeaderOptions* headerOptions, char**args, int argc){
	return 0;
}

int _HTMLPath(struct HeaderOptions* headerOptions, char**args, int argc){
	return 0;
}

int _JSPath(struct HeaderOptions* headerOptions, char**args, int argc){
	return 0;
}

struct HeaderAtlas getFunctionMap(){
	struct HeaderAtlas headerAtlas;

	headerAtlas.functions = (struct FunctionMap*)malloc(NUMBER_OF_FUNCTIONS*sizeof(struct FunctionMap));

	for (int i = 0; i < NUMBER_OF_FUNCTIONS; i++){
		headerAtlas.functions[i].nameLength = 0;
	}

	addFunction(headerAtlas.functions, 0, "USE_LOCAL_HOST", &_UseLocalHost);

	return headerAtlas;
}


void interpreteHeaderFunction(struct HeaderOptions* headerOptions, struct HeaderAtlas* headerAtlas, char*keyword, char**arguments, unsigned int kwLength, unsigned int argc){
	for (int i = 0; i < NUMBER_OF_FUNCTIONS; i++){
		unsigned int length = headerAtlas->functions[i].nameLength;
		if (length == kwLength){
			int shouldCall = 1;
			for (int j = 0; j < length; j++){
				if (headerAtlas->functions[i].name[j] != keyword[j]){
					shouldCall = 0;
				}
			}
			if (shouldCall){
				headerAtlas->functions[i].function(headerOptions, arguments, argc);
				return;
			}
		}
	}

	printf("syntax error, no function named %s found!\n", keyword);
}