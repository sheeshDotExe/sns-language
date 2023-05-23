#include "findHeader.h"

struct HeaderLines readHeaderData(struct File file, unsigned long int start, unsigned long int end){

	struct HeaderLines headerLines;

	unsigned int numberOfLines = 0;
	unsigned long int readLines = 0;

	while (1){
		unsigned int lineLength = findNextLine(file, start + readLines);
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
		unsigned int lineLength = findNextLine(file, start + readLines);
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

struct Arguments getArgs(char*string, unsigned long int start, unsigned long int stop){
	struct Arguments arguments;

	unsigned long int whiteSpace = countCharInRange(string, start, stop, ' ');
	unsigned long int numberOfArguments = countCharInRange(string, start, stop, ',') + 1;

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
		unsigned long int newArgStart = lastArgStart + findNextChar(string + lastArgStart, ",", newStop, 1);

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

void interpreteHeaderLine(struct HeaderOptions* headerOptions, struct HeaderAtlas* headerAtlas, char*command, unsigned int length){
	// get type
	if (contains(command, '(', length)){
		unsigned long int keywordEnd = findNextChar(command, " (", length, 2);
		char*keyword = (char*)malloc((keywordEnd+1)*sizeof(char));
		
		for (int i = 0; i < keywordEnd; i++){
			keyword[i] = command[i];
		}

		keyword[keywordEnd]= '\0';

		unsigned long int argumentStart = findNextChar(command, "(", length, 1);
		unsigned long int agrumentEnd = findNextChar(command, ")", length, 1);
		
		struct Arguments args = getArgs(command, argumentStart, agrumentEnd);

		interpreteHeaderFunction(headerOptions, headerAtlas, keyword, args.argv, keywordEnd, args.argc);

		for (int i=0; i < args.argc; i++){
			free(args.argv[i]);
		}
		free(keyword);
	} //....
}

void setDefaultHeaderOptions(struct HeaderOptions* headerOptions){
	headerOptions->headerStart = 0;
	headerOptions->headerEnd = 0;
	headerOptions->formattedFiles = 0;

	headerOptions->tcpOptions.localHost = 1; // use localhost by default
	headerOptions->tcpOptions.hostAddress = (char*)malloc(10*sizeof(char));
	memcpy(headerOptions->tcpOptions.hostAddress, "127.0.0.1", 10*sizeof(char)); // copy nullterminator
	headerOptions->tcpOptions.port = 8080;
	headerOptions->tcpOptions.releaseMode = 0;
	headerOptions->tcpOptions.connectionQueue = 10;
	headerOptions->tcpOptions.connectionTimeoutWait = 5000;

	headerOptions->tcpOptions.sslOptions.forceSSL = 0;
	headerOptions->tcpOptions.sslOptions.useSSL = 0;
	headerOptions->tcpOptions.sslOptions.hasCertificate = 0;
}

struct HeaderOptions getHeaderOptions(struct File file){
	struct HeaderOptions headerOptions;

	setDefaultHeaderOptions(&headerOptions);

	struct PatternRange headerLocation = getPatternByKey(file, 0, "***");

	headerOptions.headerStart = headerLocation.start;
	headerOptions.headerEnd = headerLocation.end;

	struct HeaderLines headerLines = readHeaderData(file, headerLocation.start + 3, headerLocation.end - 3);

	struct HeaderAtlas headerAtlas = getFunctionMap();

	for (int i = 0; i < headerLines.numberOfLines; i++){
		unsigned int length = strlen(headerLines.lines[i]);
		interpreteHeaderLine(&headerOptions, &headerAtlas, headerLines.lines[i], length);

		free(headerLines.lines[i]);
	}

	free(headerLines.lines);

	return headerOptions;
};