#include "html.h"

// load html document/ data

void addUserFile(struct State* state, struct UserFile* userFile){
	struct UserFile** newUserFiles = (struct Route**)realloc(state->files->files, (state->files->numberOfFiles+1)*sizeof(struct UserFile*));

	if (newUserFiles == NULL){
		raiseError("memory error on files\n", 1);
	}

	state->files->files = newUserFiles;
	state->files->files[state->files->numberOfFiles] = userFile;
	state->files->numberOfFiles++;
}

struct UserFile* createUserFile(struct State* state, char* path){
		
	//wchar_t* unicodePath = utf8ToUtf16(path); "C:\\Users\\Viktor\\Documents\\cpp\\sns-language\\public\\ååå.txt"

	FILE* fileH = fopen(path, "r");
	if (fileH == NULL){
		printf("found no file: %s\n", path);
		raiseError("", 1);
	}

	struct File file = readFile(fileH);

	fclose(fileH);

	struct UserFile* userFile = (struct UserFile*)malloc(sizeof(struct UserFile));
	userFile->data = file.mem;
	userFile->length = file.length;
	userFile->path = strdup(path);


	return userFile;
}

struct UserFile* getUserFile(struct State* state, char* path){
	struct Files* files = state->files;

	for (int i = 0; i < files->numberOfFiles; i++){
		struct UserFile* file = files->files[i];

		if (!strcmp(file->path, path)) return file;
	}

	return NULL;
}

char* safePath(char* path){
	char* newPath = (char*)malloc((strlen(path)+1)*sizeof(char));
	int i = 0;
	for (i; i < strlen(path); i++){
		if (path[i] == '/') newPath[i] = '\\';
		else newPath[i] = path[i];
	}
	newPath[i] = '\0';

	return newPath;
}

int validHex(char hex){
	int hexVal = (int)hex;

	return ((hexVal > 64 && hexVal < 71) || (hexVal > 47 && hexVal < 58));
}

int validateHexChar(char* hex){
	for (int i = 0; i < strlen(hex); i++){
		if (!validHex(hex[i])) return 0;
	}
	return 1;
}

char* urlEncodingToUtf8(char* url){
	int numberOfSpecialChars = 0;

	for (int i = 0; i < strlen(url); i++){
		if (url[i] == '%') numberOfSpecialChars++;
	}

	if (numberOfSpecialChars*2 > strlen(url)) return strdup(url);

	char* newUrl = (char*)malloc((strlen(url)-numberOfSpecialChars*2+1)*sizeof(char));

	int newUrlIndex = 0;
	for (int i = 0; i < strlen(url); i++){
		if (url[i] != '%'){
			newUrl[newUrlIndex] = url[i];
		} else {
			char* hexChar = (char*)malloc(3*sizeof(char));
			memcpy(hexChar, url+i+1, 2*sizeof(char));
			hexChar[2] = '\0';

			if (!validateHexChar(hexChar)) {
				free(newUrl);
				free(hexChar);
				printf("invalid hex\n");
				return strdup(url);
			}

			int hexValue = (int)strtol(hexChar, NULL, 16);

			newUrl[newUrlIndex] = (char)hexValue;

			free(hexChar);

			i+=2;
		}
		newUrlIndex++;
	}
	newUrl[newUrlIndex] = '\0';

	return newUrl;
}

char* getExtension(char* path){
	for (int i = strlen(path); i > 0; i--){
		if (path[i] == '.'){
			return strdup(path+i);
		}
	}
}

struct Var *html(struct Param *params, struct State *state){
	struct Var* rawPath = params->inputVars[0];
	struct String* string = (struct String*)(getType(String_c, rawPath)->type);
	char* path = string->cString;

	char* utf8Url = urlEncodingToUtf8(path);

	char* relPath = safePath(utf8Url);

	#ifndef __unix__
	char* fullPath = _fullpath(NULL, relPath, 0);
	#else
	char* fullPath = realpath(utf8Url, NULL);
	#endif

	free(state->fileExtension[0]);
	state->fileExtension[0] = getExtension(fullPath);

	struct UserFile* file = getUserFile(state, fullPath);

	if (file != NULL){
		return generateVarFromString(file->data, file->length);
	}

	file = createUserFile(state, fullPath);

	addUserFile(state, file);

	free(utf8Url);
	free(relPath);
	free(fullPath);

	struct Var* retVal = generateVarFromString(file->data, file->length);
	retVal->assignable = 0;

	return retVal;
}

// ------------------------