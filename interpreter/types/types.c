#include "types.h"

int stringToBool(char*string){
	if (!strcmp(string, "True")){
		return 1;
	} else if (!strcmp(string, "False")){
		return 0;
	}
	return -1;
}

int isNum(char*string, unsigned int length){
	for (int i = 0; i < length; i++){
		int isNumber = 0;
		for (int j = 0; j < 11; j++){
			if (string[i] == NUMBERS[j]){
				isNumber = 1;
			}
		}
		if (!isNumber){
			return 1;
		}
	}
	return 0;
}

void assignString(struct String* string, char* value, unsigned int length){
	if (string->size){
		free(string->cString);
	}
	string->size = length;
	string->cString = (char*)malloc(length*sizeof(char));
	memcpy(string->cString, value, length);
}

void assignInt(struct Int* int_s, char* value, unsigned int length){
	if (isNum(value, length)){
		printf("assignment to type int must be a number!\n");
	}
	int_s->value = atoi(value);
}

void assignFloat(struct Float* float_s, char* value, unsigned int length){
	if (isNum(value, length)){
		printf("assignment to type int must be a number!\n");
	}
	float_s->value = atof(value);
}

void assignBool(struct Bool* bool, char* value, unsigned int length){
	int bVal = stringToBool(value);
	if (bVal == -1){
		printf("assignment to type bool must be a bool!\n");
	}
	bool->value = bVal;
}

struct Type generateType(int code, char* value, unsigned int length){
	struct Type type;

	type.code = code;
	switch (code) {
		case String_c : {
			type.type = (struct String*)malloc(sizeof(struct String));
			((struct String*)type.type)->size = 0;
			assignString((struct String*)type.type, value, length);
		} break;
		case Int_c : {
			type.type = (struct Int*)malloc(sizeof(struct Int));
			assignInt((struct Int*)type.type, value, length);
		} break;
		case Float_c : {
			type.type = (struct Float*)malloc(sizeof(struct Float));
			assignFloat((struct Float*)type.type, value, length);
		} break;
	}

	return type;
}

struct Var generateVar(int* codes, unsigned int numberOfTypes, char* name, char* value){
	struct Var var;

	unsigned int nameLength = strlen(name);
	var.name = (char*)malloc((nameLength+1)*sizeof(char));
	memcpy(var.name, name, nameLength+1);

	unsigned int valueLength = strlen(value);
	var.value = (char*)malloc((valueLength+1)*sizeof(char));
	memcpy(var.value, value, valueLength+1);

	var.numberOfTypes = numberOfTypes;

	var.types = (struct Type*)malloc(numberOfTypes*sizeof(struct Type));

	for (int i = 0; i < numberOfTypes; i++){
		var.types[i] = generateType(codes[i], value, valueLength);
	}

	return var;
}

struct CommonTypes getCommonTypes(struct Var* first, struct Var* second){
	struct CommonTypes commonTypes;

	unsigned int numberOfTypes = 0;

	int types[NUMBER_OF_TYPES];

	for (int i = 0; i < first->numberOfTypes; i++){
		for (int j = 0; j < second->numberOfTypes; j++){
			if (first->types[i].code == second->types[j].code){
				types[numberOfTypes] = first->types[i].code;
				numberOfTypes++;
			}
		}
	}

	commonTypes.length = numberOfTypes;
	commonTypes.codes = (int*)malloc(numberOfTypes*sizeof(int));

	memcpy(commonTypes.codes, &types, numberOfTypes);

	return commonTypes;
}

void testVar(){

	int codes[3] = {String_c, Int_c, Float_c};
	struct Var var = generateVar((int*)codes, 3, "HelloWorld", "3.4");
	struct Var var2 = generateVar((int*)codes, 3, "HelloWorld2", "3.4");

	struct CommonTypes commonTypes = getCommonTypes(&var, &var2);
	printf("%d\n",commonTypes.length);
}