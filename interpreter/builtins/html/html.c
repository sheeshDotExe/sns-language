#include "html.h"

// load html document/ data

void add_user_file(struct State* state, struct UserFile* userFile, struct ProcessState* processState){
	struct UserFile** newUserFiles = (struct Route**)realloc(state->files->files, (state->files->numberOfFiles+1)*sizeof(struct UserFile*));

	if (newUserFiles == NULL){
		raise_error("memory error on files\n", 1, processState);
	}

	state->files->files = newUserFiles;
	state->files->files[state->files->numberOfFiles] = userFile;
	state->files->numberOfFiles++;
}

struct UserFile* create_user_file(struct State* state, char* path, struct ProcessState* processState){

	FILE* fileH = fopen(path, "r");
	if (fileH == NULL){
		printf("found no file: %s\n", path);
		raise_error("", 1, processState);
	}

	struct File file = read_file(fileH, processState);

	fclose(fileH);

	struct UserFile* userFile = (struct UserFile*)malloc(sizeof(struct UserFile));
	userFile->data = file.mem;
	userFile->length = file.length;
	userFile->path = strdup(path);


	return userFile;
}

struct UserFile* get_user_file(struct State* state, char* path, struct ProcessState* processState){
	struct Files* files = state->files;

	for (int i = 0; i < files->numberOfFiles; i++){
		struct UserFile* file = files->files[i];

		if (!strcmp(file->path, path)) return file;
	}

	return NULL;
}

char* safe_path(char* path, struct ProcessState* processState){
	char* newPath = (char*)malloc((strlen(path)+1)*sizeof(char));
	int i = 0;
	for (i; i < strlen(path); i++){
		if (path[i] == '/') newPath[i] = '\\';
		else newPath[i] = path[i];
	}
	newPath[i] = '\0';

	return newPath;
}

int valid_hex(char hex, struct ProcessState* processState){
	int hexVal = (int)hex;

	return ((hexVal > 64 && hexVal < 71) || (hexVal > 47 && hexVal < 58));
}

int validate_hex_char(char* hex, struct ProcessState* processState){
	for (int i = 0; i < strlen(hex); i++){
		if (!valid_hex(hex[i], processState)) return 0;
	}
	return 1;
}

char* url_encoding_to_utf8(char* url, struct ProcessState* processState){
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

			if (!validate_hex_char(hexChar, processState)) {
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

char* get_extension(char* path, struct ProcessState* processState){
	for (int i = strlen(path); i > 0; i--){
		if (path[i] == '.'){
			return strdup(path+i);
		}
	}
	return strdup("");
}

char* cut_excess(char* path, struct ProcessState* processState){
	int size = strlen(path)+1;
	int newSize = 0;

	for (int i = 0; i < size-1; i++){
		if ((path[i] == '/' || path[i] == '\\')&&(path[i+1] == '/' || path[i+1] == '\\')) continue;
		if ((path[i] == '.' && (path[i+1] == '.' || path[i+1] == '/' || path[i+1] == '\\'))||(i && path[i]=='.'&&path[i-1]=='.')) continue;
		newSize++;
	}

	char* newPath = (char*)malloc((newSize+1)*sizeof(char));

	int newPathIndex = 0;
	for (int i = 0; i < size-1; i++){
		if ((path[i] == '/' || path[i] == '\\')&&(path[i+1] == '/' || path[i+1] == '\\')) continue;
		if ((path[i] == '.' && (path[i+1] == '.' || path[i+1] == '/' || path[i+1] == '\\'))||(i && path[i]=='.'&&path[i-1]=='.')) continue;
		#ifdef __unix__
		if (path[i] == '\\') newPath[newPathIndex] = '/';
		else newPath[newPathIndex] = path[i];
		#else
		newPath[newPathIndex] = path[i];
		#endif
		newPathIndex++;
	}

	newPath[newSize] = '\0';

	return newPath;
}

char* get_full_path(char* relPath, struct ProcessState* processState){
	char* currentDir = getcwd(NULL, 0);
	char* rel = cut_excess(relPath, processState);

	char* newPath = (char*)malloc((strlen(currentDir)+strlen(rel)+1));
	strcpy(newPath, currentDir);
	strcat(newPath, rel);
	newPath[strlen(currentDir)+strlen(rel)] = '\0';

	free(currentDir);
	free(rel);
	return newPath;
}

struct Var *html(struct Param *params, struct State *state, struct ProcessState* processState){
	struct Var* rawPath = params->inputVars[0];
	struct String* string = (struct String*)(get_type(String_c, rawPath, processState)->type);
	char* path = string->cString;

	char* utf8Url = url_encoding_to_utf8(path, processState);

	//char* relPath = safe_path(utf8Url, processState);


	char* fullPath = get_full_path(utf8Url, processState);
	/*
	#ifndef __unix__
	char* fullPath = _fullpath(NULL, relPath, 0);
	#else
	char* fullPath = realpath(utf8Url, NULL);
	#endif
	*/

	free(state->fileExtension[0]);
	state->fileExtension[0] = get_extension(fullPath, processState);

	struct UserFile* file = get_user_file(state, fullPath, processState);

	if (file != NULL){
		return generate_var_from_string(file->data, file->length, processState);
	}

	file = create_user_file(state, fullPath, processState);

	add_user_file(state, file, processState);

	free(utf8Url);
	//free(relPath);
	free(fullPath);

	struct Var* retVal = generate_var_from_string(file->data, file->length, processState);
	retVal->assignable = 0;

	return retVal;
}

// ------------------------