#include "types.h"

int stringToBool(char*string){
	if (!strcmp(string, "True")){
		return 1;
	} else if (!strcmp(string, "False")){
		return 0;
	}
	return -1;
}

int isFloat(char*string, unsigned int length){
	for (int i = 0; i < length; i++){
		int isNumber = 0;
		for (int j = 0; j < 12; j++){
			if (string[i] == NUMBERS_WITH_DOT[j]){
				isNumber = 1;
			}
		}
		if (!isNumber){
			return 1;
		}
	}
	return 0;
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

int isString(char*value, unsigned int length){
	if (contains(value, '"', length) || contains(value, '\'', length)){
		return 1;
	}
	return 0;
}

void fillTypes(struct CommonTypes* cTypes, int* types, unsigned int length){
	cTypes->length = length;
	cTypes->codes = (int*)malloc(length*sizeof(int));
	for (int i = 0; i < length; i++){
		cTypes->codes[i] = types[i];
	}
}

struct CommonTypes getValidTypes(char**value, unsigned int* lengthP){
	struct CommonTypes types;

	int validTypes[NUMBER_OF_TYPES];
	unsigned int length = 0;

	if (*lengthP == 1){
		validTypes[length] = Char_c;
		length++;
	}

	validTypes[length] = String_c;
	length++;
	if (isString(*value, *lengthP)){
		if (*lengthP == 3){
			validTypes[length] = Char_c;
			length++;
		}
		fillTypes(&types, validTypes, length);
		return types;
	}

	int Bool_v = stringToBool(*value);
	if (Bool_v != -1){
		*lengthP = 1;

		validTypes[length] = Int_c;
		length++;
		validTypes[length] = Float_c;
		length++;

		fillTypes(&types, validTypes, length);
		return types;
	}

	if (!isNum(*value, *lengthP)){
		validTypes[length] = Int_c;
		length++;
	}

	if (!isFloat(*value, *lengthP)){
		validTypes[length] = Float_c;
		length++;
	}

	fillTypes(&types, validTypes, length);
	return types;
}

struct Var* generateVarFromString(char*value, unsigned int length){
	char**valueP = &value;

	struct CommonTypes types = getValidTypes(valueP, &length);

	struct Var* ret = generateVar(types.codes, types.length, "unnamed", *valueP, (struct Param*)NULL);

	free(types.codes);

	return ret;
}

void assignString(struct String* string, char* value, unsigned int length){
	if (string->size){
		free(string->cString);
	}
	string->size = length;
	string->cString = (char*)malloc((length+1)*sizeof(char));
	memcpy(string->cString, value, length+1);
}

void assignInt(struct Int* int_s, char* value, unsigned int length){
	if (isNum(value, length)){
		raiseError("assignment to type int must be a number!\n", 1);
	}
	int_s->value = atoi(value);
}

void assignFloat(struct Float* float_s, char* value, unsigned int length){
	if (isFloat(value, length)){
		raiseError("assignment to type float must be a number!\n", 1);
	}
	float_s->value = atof(value);
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


struct Var* generateVar(int* codes, unsigned int numberOfTypes, char* name, char* value, struct Param* param){
	struct Var* var = (struct Var*)malloc(sizeof(struct Var));

	var->creationFlag = 1;

	unsigned int nameLength = strlen(name);
	var->name = (char*)malloc((nameLength+1)*sizeof(char));
	memcpy(var->name, name, nameLength+1);

	unsigned int valueLength = strlen(value);
	var->value = (char*)malloc((valueLength+1)*sizeof(char));
	memcpy(var->value, value, valueLength+1);

	var->numberOfTypes = numberOfTypes;

	var->types = (struct Type*)malloc(numberOfTypes*sizeof(struct Type));

	for (int i = 0; i < numberOfTypes; i++){
		var->types[i] = generateType(codes[i], value, valueLength);
	}

	return var;
}

struct Var* copyVar(struct Var* instance){
	struct Var* var = (struct Var*)malloc(sizeof(struct Var));

	var->creationFlag = 1;

	unsigned int nameLength = strlen(instance->name);
	var->name = (char*)malloc((nameLength+1)*sizeof(char));
	memcpy(var->name, instance->name, nameLength+1);

	unsigned int valueLength = strlen(instance->value);
	var->value = (char*)malloc((valueLength+1)*sizeof(char));
	memcpy(var->value, instance->value, valueLength+1);

	var->numberOfTypes = instance->numberOfTypes;
	var->types = (struct Type*)malloc(instance->numberOfTypes*sizeof(struct Type));

	for (int i = 0; i < instance->numberOfTypes; i++){
		var->types[i] = generateType(instance->types[i].code, instance->value, valueLength);
	}

	return var;
}

int getSignificantType(struct CommonTypes* commonTypes){
	int significant = -1;
	for (int i = 0; i < commonTypes->length; i++){
		if(commonTypes->codes[i] > significant){
			significant = commonTypes->codes[i];
		}
	}
	return significant;
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

	for (int i = 0; i < numberOfTypes; i++){
		commonTypes.codes[i] = types[i];
	}

	return commonTypes;
}

struct Type* getType(int code, struct Var* var){
	for (int i = 0; i < var->numberOfTypes; i++){
		if (var->types[i].code == code){
			return &var->types[i];
		}
	}
	return NULL;
}

void freeTypes(struct Type* types, unsigned int length, struct Var* var){
	for (int i = 0; i < length; i++){
		int code = types[i].code;
		switch (code){
			case String_c : {
				struct String* string1 = (struct String*)types[i].type;
				free(string1->cString);
			} break;

			case Int_c : {
			} break;

			case Float_c : {
			}
		}
		free(var->types[i].type);
	}
	free(var->types);
}

void freeVar(struct Var* var){
	if (var->creationFlag){
		freeTypes(var->types, var->numberOfTypes, var);
		free(var->name);
		free(var->value);
	}
}

void assignValue(struct Var* var, struct Var*other){
	struct CommonTypes commonTypes = getCommonTypes(var, other);

	if (commonTypes.length == 0){
		printf("cannot assign variable %s to variable %s\n", var->value, other->value);
		raiseError("", 1);
	}

	char* value = other->value;
	unsigned int valueLength = strlen(value);

	freeTypes(var->types, var->numberOfTypes, var);

	var->numberOfTypes = commonTypes.length;
	var->types = (struct Type*)malloc(commonTypes.length*sizeof(struct Type));

	for (int i = 0; i < commonTypes.length; i++){
		var->types[i] = generateType(commonTypes.codes[i], value, valueLength);
	}

	free(var->value);
	var->value = (char*)malloc((valueLength+1)*sizeof(char));
	memcpy(var->value, value, valueLength+1);

	free(commonTypes.codes);

}

struct Var* addVars(struct Var* first, struct Var* second){
	struct CommonTypes commonTypes = getCommonTypes(first, second);

	if (commonTypes.length == 0){
		printf("cannot add variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1);
	}

	int code = getSignificantType(&commonTypes);

	char* newValue;

	switch (code){
		case String_c : {
			struct String* string1 = (struct String*)getType(String_c, first)->type;
			struct String* string2 = (struct String*)getType(String_c, second)->type;

			unsigned int length = string1->size + string2->size + 1;
			newValue = (char*)malloc(length*sizeof(char));
			memcpy(newValue, string1->cString, string1->size);
			memcpy(newValue+string1->size, string2->cString, string2->size+1); // copy null terminator
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second)->type;

			long long new = int1->value + int2->value;
			newValue = (char*)malloc(20*sizeof(char)); // largest 64 bit signed integer
			itoa(new, newValue, 10);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second)->type;

			float new = float1->value + float2->value;
			newValue = (char*)malloc(100*sizeof(char));
			gcvt(new, 100, newValue);
		}
	}

	struct Var* var = generateVar(commonTypes.codes, commonTypes.length, "unnamed", newValue, (struct Param*)NULL);

	free(commonTypes.codes);
	free(newValue);

	return var;
}

struct Var* subVars(struct Var* first, struct Var* second){
	struct CommonTypes commonTypes = getCommonTypes(first, second);

	if (commonTypes.length == 0){
		printf("cannot sub variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1);
	}

	int code = getSignificantType(&commonTypes);

	char* newValue;

	switch (code){
		case String_c : {
			raiseError("string has no operator -\n", 1);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second)->type;

			long long new = int1->value - int2->value;
			newValue = (char*)malloc(20*sizeof(char)); // largest 64 bit signed integer
			itoa(new, newValue, 10);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second)->type;

			float new = float1->value - float2->value;
			newValue = (char*)malloc(100*sizeof(char));
			gcvt(new, 100, newValue);
		}
	}

	struct Var* var = generateVar(commonTypes.codes, commonTypes.length, "unnamed", newValue, (struct Param*)NULL);

	free(commonTypes.codes);
	free(newValue);

	return var;
}

struct Var* divVars(struct Var* first, struct Var* second){
	struct CommonTypes commonTypes = getCommonTypes(first, second);

	if (commonTypes.length == 0){
		printf("cannot divide variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1);
	}

	int code = getSignificantType(&commonTypes);

	char* newValue;

	switch (code){
		case String_c : {
			raiseError("string has no operator /\n", 1);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second)->type;

			long long new = int1->value / int2->value;
			newValue = (char*)malloc(20*sizeof(char)); // largest 64 bit signed integer
			itoa(new, newValue, 10);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second)->type;

			float new = float1->value / float2->value;
			newValue = (char*)malloc(100*sizeof(char));
			gcvt(new, 100, newValue);
		}
	}

	struct Var* var = generateVarFromString(newValue, strlen(newValue));
	free(commonTypes.codes);
	free(newValue);

	return var;
}

struct Var* mulVars(struct Var* first, struct Var* second){
	struct CommonTypes commonTypes = getCommonTypes(first, second);

	if (commonTypes.length == 0){
		printf("cannot multiply variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1);
	}

	int code = getSignificantType(&commonTypes);

	char* newValue;

	switch (code){
		case String_c : {
			raiseError("string has no operator *\n", 1);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second)->type;

			long long new = int1->value * int2->value;
			newValue = (char*)malloc(20*sizeof(char)); // largest 64 bit signed integer
			itoa(new, newValue, 10);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second)->type;

			float new = float1->value * float2->value;
			newValue = (char*)malloc(100*sizeof(char));
			gcvt(new, 100, newValue);
		}
	}
	struct Var* var = generateVar(commonTypes.codes, commonTypes.length, "unnamed", newValue, (struct Param*)NULL);

	free(commonTypes.codes);
	free(newValue);

	return var;
}

struct Var* lessThan(struct Var* first, struct Var* second){
	struct CommonTypes commonTypes = getCommonTypes(first, second);

	if (commonTypes.length == 0){
		printf("cannot compare variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1);
	}

	int code = getSignificantType(&commonTypes);

	char* newValue;

	switch (code){
		case String_c : {
			struct String* string1 = (struct String*)getType(String_c, first)->type;
			struct String* string2 = (struct String*)getType(String_c, second)->type;

			unsigned int result = string1->size < string2->size;
			newValue = (char*)malloc(1*sizeof(char));
			itoa(result, newValue, 10);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second)->type;

			long long new = int1->value < int2->value;
			newValue = (char*)malloc(1*sizeof(char)); // largest 64 bit signed integer
			itoa(new, newValue, 10);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second)->type;

			float new = float1->value < float2->value;
			newValue = (char*)malloc(1*sizeof(char));
			gcvt(new, 1, newValue);
		}
	}

	int codes[4] = {Int_c, Float_c, String_c, Char_c};

	struct Var* var = generateVar((int*)codes, 5, "unnamed", newValue, (struct Param*)NULL);

	free(commonTypes.codes);

	return var;
}

struct Var* greaterThan(struct Var* first, struct Var* second){
	struct CommonTypes commonTypes = getCommonTypes(first, second);

	if (commonTypes.length == 0){
		printf("cannot compare variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1);
	}

	int code = getSignificantType(&commonTypes);

	char* newValue;

	switch (code){
		case String_c : {
			struct String* string1 = (struct String*)getType(String_c, first)->type;
			struct String* string2 = (struct String*)getType(String_c, second)->type;

			unsigned int result = string1->size > string2->size;
			newValue = (char*)malloc(1*sizeof(char));
			itoa(result, newValue, 10);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second)->type;

			long long new = int1->value > int2->value;
			newValue = (char*)malloc(1*sizeof(char)); // largest 64 bit signed integer
			itoa(new, newValue, 10);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second)->type;

			float new = float1->value > float2->value;
			newValue = (char*)malloc(1*sizeof(char));
			gcvt(new, 1, newValue);
		}
	}

	int codes[4] = {Int_c, Float_c, String_c, Char_c};

	struct Var* var = generateVar((int*)codes, 5, "unnamed", newValue, (struct Param*)NULL);

	free(commonTypes.codes);

	return var;
}

struct Var* equalTo(struct Var* first, struct Var* second){
	struct CommonTypes commonTypes = getCommonTypes(first, second);

	if (commonTypes.length == 0){
		printf("cannot compare variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1);
	}

	int code = getSignificantType(&commonTypes);

	char* newValue;

	switch (code){
		case String_c : {
			struct String* string1 = (struct String*)getType(String_c, first)->type;
			struct String* string2 = (struct String*)getType(String_c, second)->type;

			unsigned int result = 0;
			if (!strcmp(string1->cString, string2->cString)){
				result = 1;
			}
			newValue = (char*)malloc(1*sizeof(char));
			itoa(result, newValue, 10);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second)->type;

			long long new = int1->value == int2->value;
			newValue = (char*)malloc(1*sizeof(char)); // largest 64 bit signed integer
			itoa(new, newValue, 10);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second)->type;

			float new = float1->value == float2->value;
			newValue = (char*)malloc(1*sizeof(char));
			gcvt(new, 1, newValue);
		}
	}

	int codes[4] = {Int_c, Float_c, String_c, Char_c};

	struct Var* var = generateVar((int*)codes, 4, "unnamed", newValue, (struct Param*)NULL);

	free(commonTypes.codes);

	return var;
}

void addKey(struct Key* keyP, char* name, unsigned int length, int key){

	keyP->name = (char*)malloc(length*sizeof(char));
	memcpy(keyP->name, name, length);

	keyP->key = key;
	keyP->length = length;
}

struct KeyChars createKeyChars(){
	struct KeyChars keyChars;

	keyChars.length = NUMBER_OF_KEYS;
	keyChars.keys = (struct Key*)malloc(NUMBER_OF_KEYS*sizeof(struct Key));

	char* keys[] = {"-=", "+=", "/=", "*=", "::", ":", "?",",", "=", "->", "{", "}", "[", "]",
					"(", ")", "-", "+", "/", "*"
					};

	for (int i = 0; i < NUMBER_OF_KEYS; i++){
		addKey(&keyChars.keys[i], keys[i], strlen(keys[i])+1, i);
	}

	return keyChars;
}

struct Var* getVarFromScope(struct VarScope* scope, char* varName){
	for (int i = 0; i < scope->numberOfVars; i++){
		struct Var* var = &scope->vars[i];
		if (!strcmp(varName, var->name)){
			return var;
		}
	}
	printf("no variable named %s", varName);
	raiseError("", 1);
	return (struct Var*)NULL;
}

void addVarToScope(struct VarScope* scope, struct Var* var){
	scope->numberOfVars++;
	struct Var* newVars = (struct Var*)realloc(scope->vars, scope->numberOfVars*sizeof(struct Var));
	if (newVars == NULL){
		raiseError("memory error", 1);
	} else {
		scope->vars = newVars;
		scope->vars[scope->numberOfVars-1] = *var;
	}
}