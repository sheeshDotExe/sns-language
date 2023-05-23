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

	char* relPath = safePath(path);
	char* fullPath = _fullpath(NULL, relPath, 0);

	free(state->fileExtension[0]);
	state->fileExtension[0] = getExtension(fullPath);

	struct UserFile* file = getUserFile(state, fullPath);

	if (file != NULL){
		return generateVarFromString(file->data, file->length);
	}

	file = createUserFile(state, fullPath);

	addUserFile(state, file);

	free(relPath);
	free(fullPath);

	struct Var* retVal = generateVarFromString(file->data, file->length);
	retVal->assignable = 0;

	return retVal;
}

// ------------------------