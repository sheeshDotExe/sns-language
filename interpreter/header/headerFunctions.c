#include "headerFunctions.h"



void addFunction(struct FunctionMap* functionMap, int index, char*name, int(*function)(struct HeaderOptions* headerOptions, char*name, char**args, int argc)){
	unsigned int nameLength = strlen(name);
	functionMap[index].name = (char*)malloc((nameLength+1)*sizeof(char));
	functionMap[index].nameLength = nameLength;
	for (int i = 0; i < nameLength; i++){
		functionMap[index].name[i] = name[i];
	}
	functionMap[index].name[nameLength] = '\0';
	functionMap[index].function = function;
}

int checkArgsCount(char*name, int expected, int given){
	if (given != expected){
		printf("Function %s expected %d arguments, but %d were given!\n", name, expected, given);
		return 1;
	}
	return 0;
}



int _UseLocalHost(struct HeaderOptions* headerOptions, char*name, char**args, int argc){
	if (checkArgsCount(name, 1, argc)){
		return 1;
	}

	int Bool = stringToBool(args[0]);
	if (Bool == -1){
		printf("invalid parameter\n");
	}
	else if (Bool){
		headerOptions->tcpOptions.localHost = 1;
	} else {
		headerOptions->tcpOptions.localHost = 0;
	}

	return 0;
}

int _UsePort(struct HeaderOptions* headerOptions, char*name, char**args, int argc){
	if (checkArgsCount(name, 1, argc)){
		return 1;
	}

	if (isNum(args[0], strlen(args[0]))){
		printf("argument must be number\n");
		return 1;
	}

	int port = atoi(args[0]);
	headerOptions->tcpOptions.port = port;
	return 0;
}

int _DebugMode(struct HeaderOptions* headerOptions, char*name, char**args, int argc){
	if (checkArgsCount(name, 1, argc)){
		return 1;
	}

	int Bool = stringToBool(args[0]);
	if (Bool == -1){
		printf("invalid parameter\n");
	}
	else if (Bool){
		headerOptions->tcpOptions.releaseMode = 0;
	} else {
		headerOptions->tcpOptions.releaseMode = 1;
	}

	return 0;
}

int _ForceSSL(struct HeaderOptions* headerOptions, char*name, char**args, int argc){
	if (checkArgsCount(name, 1, argc)){
		return 1;
	}

	int Bool = stringToBool(args[0]);
	if (Bool == -1){
		printf("invalid parameter\n");
	}
	else if (Bool){
		headerOptions->tcpOptions.sslOptions.forceSSL = 1;
		headerOptions->tcpOptions.sslOptions.useSSL = 1;
	} else {
		headerOptions->tcpOptions.sslOptions.forceSSL = 0;
	}

	return 0;
}

int _UseSSL(struct HeaderOptions* headerOptions, char*name, char**args, int argc){
	if (checkArgsCount(name, 1, argc)){
		return 1;
	}

	int Bool = stringToBool(args[0]);
	if (Bool == -1){
		printf("invalid parameter\n");
	}
	else if (Bool){
		headerOptions->tcpOptions.sslOptions.useSSL = 1;
	} else {
		headerOptions->tcpOptions.sslOptions.useSSL = 0;
	}

	return 0;
}

int _MaxConnections(struct HeaderOptions* headerOptions, char*name, char**args, int argc){
	if (checkArgsCount(name, 1, argc)){
		return 1;
	}

	if (isNum(args[0], strlen(args[0]))){
		printf("argument must be number\n");
		return 1;
	}

	int connections = atoi(args[0]);
	headerOptions->tcpOptions.connectionQueue = connections;

	return 0;
}

int _HTMLPath(struct HeaderOptions* headerOptions, char*name, char**args, int argc){
	if (checkArgsCount(name, 1, argc)){
		return 1;
	}
	return 0;
}

int _JSPath(struct HeaderOptions* headerOptions, char*name, char**args, int argc){
	if (checkArgsCount(name, 1, argc)){
		return 1;
	}
	return 0;
}

struct HeaderAtlas getFunctionMap(){
	struct HeaderAtlas headerAtlas;

	headerAtlas.functions = (struct FunctionMap*)malloc(NUMBER_OF_FUNCTIONS*sizeof(struct FunctionMap));

	for (int i = 0; i < NUMBER_OF_FUNCTIONS; i++){
		headerAtlas.functions[i].nameLength = 0;
	}

	addFunction(headerAtlas.functions, 0, "USE_LOCAL_HOST", &_UseLocalHost);
	addFunction(headerAtlas.functions, 1, "USE_PORT", &_UsePort);
	addFunction(headerAtlas.functions, 2, "DEBUG_MODE", &_DebugMode);
	addFunction(headerAtlas.functions, 3, "FORCE_SSL", &_ForceSSL);
	addFunction(headerAtlas.functions, 4, "USE_SSL", &_UseSSL);
	addFunction(headerAtlas.functions, 5, "MAX_CONNECTIONS", &_MaxConnections);
	addFunction(headerAtlas.functions, 6, "HTML_PATH", &_HTMLPath);
	addFunction(headerAtlas.functions, 7, "JS_PATH", &_JSPath);

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
				headerAtlas->functions[i].function(headerOptions, headerAtlas->functions[i].name, arguments, argc);
				return;
			}
		}
	}

	printf("syntax error, no function named %s found!\n", keyword);
}