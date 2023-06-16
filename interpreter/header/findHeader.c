#include "findHeader.h"

struct HeaderLines read_header_data(struct File file, unsigned long int start, unsigned long int end, struct ProcessState* processState){

	struct HeaderLines headerLines;

	unsigned int numberOfLines = 0;
	unsigned long int readLines = 0;

	while (1){
		unsigned int lineLength = find_next_line(file, start + readLines, processState);
		readLines += lineLength+1;

		if (lineLength){
			numberOfLines += 1;
		}

		if (start + readLines >= end){
			break;
		}
	}

	headerLines.lines = (char**)malloc(numberOfLines*sizeof(char*));
	headerLines.numberOfLines = numberOfLines;

	unsigned int lineNumber = 0;
	readLines = 0;

	while (1){
		unsigned int lineLength = find_next_line(file, start + readLines, processState);
		readLines += lineLength+1;

		if (lineLength){

			headerLines.lines[lineNumber] = (char*)malloc((lineLength+1)*sizeof(char));
			unsigned int j = 0;
			for (unsigned int i = start + readLines - (lineLength+1); i < start + readLines - 1; i++){
				headerLines.lines[lineNumber][j] = file.mem[i];
				j++;
			}
			headerLines.lines[lineNumber][j] = '\0';
			lineNumber++;
		}

		if (start + readLines >= end){
			break;
		}
	}

	return headerLines;
}

struct Arguments get_args(char*string, unsigned long int start, unsigned long int stop, struct ProcessState* processState){
	struct Arguments arguments;

	unsigned long int whiteSpace = count_char_in_range(string, start, stop, ' ', processState);
	unsigned long int numberOfArguments = count_char_in_range(string, start, stop, ',', processState) + 1;

	arguments.argc = numberOfArguments;
	arguments.argv = (char**)malloc(numberOfArguments*sizeof(char*));

	char*newString = (char*)malloc((stop-start-whiteSpace)*sizeof(char));

	unsigned int count = 0;
	for (int i = start; i < stop; i++){
		if (string[i] != ' '){
			newString[count] = string[i];
			count++;
		}
	}
	newString[stop-start-whiteSpace] = '\0';

	string = newString;

	unsigned long int lastArgStart = 1;
	unsigned long int newStop = stop-start-whiteSpace;

	for (int i=0; i < numberOfArguments; i++){
		unsigned long int newArgStart = lastArgStart + find_next_char(string + lastArgStart, ",", newStop, 1, processState);

		unsigned long int length = 0;

		if (newArgStart == lastArgStart){
			length = newStop - lastArgStart;
		} else {
			length = newArgStart - lastArgStart;
		}
		
		arguments.argv[i] = (char*)malloc((length+1)*sizeof(char));

		int count = 0;
		for (int j=lastArgStart; j < lastArgStart+length; j++){
			arguments.argv[i][count] = string[j];
			count++;
		}
		arguments.argv[i][length] = '\0';

		lastArgStart = newArgStart+1;
	}

	return arguments;
}

void interprete_header_line(struct HeaderOptions* headerOptions, struct HeaderAtlas* headerAtlas, char*command, unsigned int length, struct ProcessState* processState){
	// get type
	if (contains(command, '(', length, processState)){
		unsigned long int keywordEnd = find_next_char(command, " (", length, 2, processState);
		char*keyword = (char*)malloc((keywordEnd+1)*sizeof(char));
		
		for (int i = 0; i < keywordEnd; i++){
			keyword[i] = command[i];
		}

		keyword[keywordEnd]= '\0';

		unsigned long int argumentStart = find_next_char(command, "(", length, 1, processState);
		unsigned long int agrumentEnd = find_next_char(command, ")", length, 1, processState);
		
		struct Arguments args = get_args(command, argumentStart, agrumentEnd, processState);

		interprete_header_function(headerOptions, headerAtlas, keyword, args.argv, keywordEnd, args.argc, processState);

		for (int i=0; i < args.argc; i++){
			free(args.argv[i]);
		}
		free(keyword);
	} //....
}

void set_default_header_options(struct HeaderOptions* headerOptions, struct ProcessState* processState){
	headerOptions->headerStart = 0;
	headerOptions->headerEnd = 0;
	headerOptions->formattedFiles = 0;

	headerOptions->tcpOptions = (struct TcpOptions*)malloc(sizeof(struct TcpOptions));

	headerOptions->tcpOptions->localHost = 1; // use localhost by default
	headerOptions->tcpOptions->hostAddress = (char*)malloc(10*sizeof(char));
	memcpy(headerOptions->tcpOptions->hostAddress, "127.0.0.1", 10*sizeof(char)); // copy nullterminator
	headerOptions->tcpOptions->port = 8080;
	headerOptions->tcpOptions->releaseMode = 0;
	headerOptions->tcpOptions->connectionQueue = 10;
	headerOptions->tcpOptions->connectionTimeoutWait = 5000;

	headerOptions->tcpOptions->sslOptions = (struct SSLOptions*)malloc(sizeof(struct SSLOptions));

	headerOptions->tcpOptions->sslOptions->forceSSL = 0;
	headerOptions->tcpOptions->sslOptions->useSSL = 0;
	headerOptions->tcpOptions->sslOptions->hasCertificate = 0;
}

struct HeaderOptions* get_header_options(struct File file, struct ProcessState* processState){
	struct HeaderOptions* headerOptions = (struct HeaderOptions*)malloc(sizeof(struct HeaderOptions));

	set_default_header_options(headerOptions, processState);

	struct PatternRange headerLocation = get_pattern_by_key(file, 0, "***", processState);

	headerOptions->headerStart = headerLocation.start;
	headerOptions->headerEnd = headerLocation.end;

	struct HeaderLines headerLines = read_header_data(file, headerLocation.start + 3, headerLocation.end - 3, processState);

	struct HeaderAtlas headerAtlas = get_function_map(processState);

	for (int i = 0; i < headerLines.numberOfLines; i++){
		unsigned int length = strlen(headerLines.lines[i]);
		interprete_header_line(headerOptions, &headerAtlas, headerLines.lines[i], length, processState);

		free(headerLines.lines[i]);
	}

	free(headerLines.lines);

	return headerOptions;
};