#include "headerFunctions.h"



void add_function(struct FunctionMap* functionMap, int index, char*name, int(*function)(struct HeaderOptions* headerOptions, char*name, char**args, int argc, struct ProcessState* processState), struct ProcessState* processState){
	unsigned int nameLength = strlen(name);
	functionMap[index].name = (char*)malloc((nameLength+1)*sizeof(char));
	functionMap[index].nameLength = nameLength;
	for (int i = 0; i < nameLength; i++){
		functionMap[index].name[i] = name[i];
	}
	functionMap[index].name[nameLength] = '\0';
	functionMap[index].function = function;
}

int check_args_count(char*name, int expected, int given, struct ProcessState* processState){
	if (given != expected){
		printf("Function %s expected %d arguments, but %d were given!\n", name, expected, given);
		return 1;
	}
	return 0;
}



int _UseLocalHost(struct HeaderOptions* headerOptions, char*name, char**args, int argc, struct ProcessState* processState){
	if (check_args_count(name, 1, argc, processState)){
		return 1;
	}

	int Bool = string_to_bool(args[0], processState);
	if (Bool == -1){
		printf("invalid parameter\n");
	}
	else if (Bool){
		headerOptions->tcpOptions->localHost = 1;
	} else {
		headerOptions->tcpOptions->localHost = 0;
	}

	return 0;
}

int _UsePort(struct HeaderOptions* headerOptions, char*name, char**args, int argc, struct ProcessState* processState){
	if (check_args_count(name, 1, argc, processState)){
		return 1;
	}

	if (is_num(args[0], strlen(args[0]), processState)){
		printf("argument must be number\n");
		return 1;
	}

	int port = atoi(args[0]);
	headerOptions->tcpOptions->port = port;
	return 0;
}

int _DebugMode(struct HeaderOptions* headerOptions, char*name, char**args, int argc, struct ProcessState* processState){
	if (check_args_count(name, 1, argc, processState)){
		return 1;
	}

	int Bool = string_to_bool(args[0], processState);
	if (Bool == -1){
		printf("invalid parameter\n");
	}
	else if (Bool){
		headerOptions->tcpOptions->releaseMode = 0;
	} else {
		headerOptions->tcpOptions->releaseMode = 1;
	}

	return 0;
}

int _ForceSSL(struct HeaderOptions* headerOptions, char*name, char**args, int argc, struct ProcessState* processState){
	if (check_args_count(name, 1, argc, processState)){
		return 1;
	}

	int Bool = string_to_bool(args[0], processState);
	if (Bool == -1){
		printf("invalid parameter\n");
	}
	else if (Bool){
		headerOptions->tcpOptions->sslOptions->forceSSL = 1;
		headerOptions->tcpOptions->sslOptions->useSSL = 1;
	} else {
		headerOptions->tcpOptions->sslOptions->forceSSL = 0;
	}

	return 0;
}

int _UseSSL(struct HeaderOptions* headerOptions, char*name, char**args, int argc, struct ProcessState* processState){
	if (check_args_count(name, 2, argc, processState)){
		return 1;
	}

	headerOptions->tcpOptions->sslOptions->useSSL = 1;

	headerOptions->tcpOptions->sslOptions->sslCertificate = strdup(args[0]);
	headerOptions->tcpOptions->sslOptions->keyPath = strdup(args[1]);

	return 0;
}

int _MaxConnections(struct HeaderOptions* headerOptions, char*name, char**args, int argc, struct ProcessState* processState){
	if (check_args_count(name, 1, argc, processState)){
		return 1;
	}

	if (is_num(args[0], strlen(args[0]), processState)){
		printf("argument must be number\n");
		return 1;
	}

	int connections = atoi(args[0]);
	headerOptions->tcpOptions->connectionQueue = connections;

	return 0;
}

int _HTMLPath(struct HeaderOptions* headerOptions, char*name, char**args, int argc, struct ProcessState* processState){
	if (check_args_count(name, 1, argc, processState)){
		return 1;
	}
	return 0;
}

int _JSPath(struct HeaderOptions* headerOptions, char*name, char**args, int argc, struct ProcessState* processState){
	if (check_args_count(name, 1, argc, processState)){
		return 1;
	}
	return 0;
}

struct HeaderAtlas get_function_map(struct ProcessState* processState){
	struct HeaderAtlas headerAtlas;

	headerAtlas.functions = (struct FunctionMap*)malloc(NUMBER_OF_FUNCTIONS*sizeof(struct FunctionMap));

	for (int i = 0; i < NUMBER_OF_FUNCTIONS; i++){
		headerAtlas.functions[i].nameLength = 0;
	}

	add_function(headerAtlas.functions, 0, "USE_LOCAL_HOST", &_UseLocalHost, processState);
	add_function(headerAtlas.functions, 1, "USE_PORT", &_UsePort, processState);
	add_function(headerAtlas.functions, 2, "DEBUG_MODE", &_DebugMode, processState);
	add_function(headerAtlas.functions, 3, "FORCE_SSL", &_ForceSSL, processState);
	add_function(headerAtlas.functions, 4, "USE_SSL", &_UseSSL, processState);
	add_function(headerAtlas.functions, 5, "MAX_CONNECTIONS", &_MaxConnections, processState);
	add_function(headerAtlas.functions, 6, "HTML_PATH", &_HTMLPath, processState);
	add_function(headerAtlas.functions, 7, "JS_PATH", &_JSPath, processState);

	return headerAtlas;
}


void interprete_header_function(struct HeaderOptions* headerOptions, struct HeaderAtlas* headerAtlas, char*keyword, char**arguments, unsigned int kwLength, unsigned int argc, struct ProcessState* processState){
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
				headerAtlas->functions[i].function(headerOptions, headerAtlas->functions[i].name, arguments, argc, processState);
				return;
			}
		}
	}

	printf("syntax error, no function named %s found!\n", keyword);
}