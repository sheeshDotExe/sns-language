#include "types.h"

int stringToBool(char*string, struct ProcessState* processState){
	if (!strcmp(string, "True")){
		return 1;
	} else if (!strcmp(string, "False")){
		return 0;
	}
	return -1;
}

int isFloat(char*string, unsigned int length, struct ProcessState* processState){
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

int isNum(char*string, unsigned int length, struct ProcessState* processState){
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

int isString(char*value, unsigned int length, struct ProcessState* processState){
	if (strlen(value)){
		if (value[0] == '"' || value[0] == '\'') return 1;
	}
	return 0;
}

void assignString(struct String* string, char* value, unsigned int length, struct ProcessState* processState){
	if (string->size){
		free(string->cString);
	}
	if (!isString(value, length, processState)){
		string->size = length;
		string->cString = (char*)malloc((length+1)*sizeof(char));
		memcpy(string->cString, value, (length+1)*sizeof(char));
	} else {
		string->size = length-2;
		string->cString = (char*)malloc((length-1)*sizeof(char));
		memcpy(string->cString, value+1, (length-2)*sizeof(char));
		string->cString[length-2] = '\0';
	}
}

void assignInt(struct Int* int_s, char* value, unsigned int length, struct ProcessState* processState){
	if (isNum(value, length, processState)){
		raiseError("assignment to type int must be a number!\n", 1, processState);
	}
	int_s->value = atoi(value);
}

void assignFloat(struct Float* float_s, char* value, unsigned int length, struct ProcessState* processState){
	if (isFloat(value, length, processState)){
		raiseError("assignment to type float must be a number!\n", 1, processState);
	}
	float_s->value = atof(value);
}

struct Type generateType(int code, char* value, unsigned int length, struct ProcessState* processState){
	struct Type type;

	type.code = code;

	int hasValue = 0;

	switch (code) {
		case String_c : {
			hasValue = 1;
			type.type = (struct String*)malloc(sizeof(struct String));
			((struct String*)type.type)->size = 0;
			assignString((struct String*)type.type, value, length, processState);
		} break;
		case Int_c : {
			hasValue = 1;
			type.type = (struct Int*)malloc(sizeof(struct Int));
			assignInt((struct Int*)type.type, value, length, processState);
		} break;
		case Float_c : {
			hasValue = 1;
			type.type = (struct Float*)malloc(sizeof(struct Float));
			assignFloat((struct Float*)type.type, value, length, processState);
		} break;
	}

	type.hasValue = hasValue;

	return type;
}


struct Var* generateVar(int* codes, unsigned int numberOfTypes, char* name, char* value, struct Param* param, struct ProcessState* processState){
	struct Var* var = (struct Var*)malloc(sizeof(struct Var));

	var->creationFlag = 1;
	var->hasParam = 0;
	var->shouldExecute = 0;
	var->inheritScopes = 0;
	var->returned = 0;
	var->isBuiltin = 0;

	unsigned int nameLength = strlen(name);
	var->name = (char*)malloc((nameLength+1)*sizeof(char));
	memcpy(var->name, name, (nameLength+1)*sizeof(char));

	unsigned int valueLength = strlen(value);
	var->value = (char*)malloc((valueLength+1)*sizeof(char));
	memcpy(var->value, value, (valueLength+1)*sizeof(char));

	var->numberOfTypes = numberOfTypes;

	var->types = (struct Type*)malloc(numberOfTypes*sizeof(struct Type));

	for (int i = 0; i < numberOfTypes; i++){
		var->types[i] = generateType(codes[i], value, valueLength, processState);
	}

	if (param){
		var->param = param;
		var->originalParam = copyParam(param, processState);
	}

	return var;
}

struct Var* copyVar(struct Var* instance, struct ProcessState* processState){
	struct Var* var = (struct Var*)malloc(sizeof(struct Var));

	var->creationFlag = 1;
	var->shouldExecute = instance->shouldExecute;
	var->inheritScopes = instance->inheritScopes;
	var->returned = instance->returned;
	var->isBuiltin = instance->isBuiltin;

	unsigned int nameLength = strlen(instance->name);
	var->name = (char*)malloc((nameLength+1)*sizeof(char));
	memcpy(var->name, instance->name, (nameLength+1)*sizeof(char));

	unsigned int valueLength = strlen(instance->value);
	var->value = (char*)malloc((valueLength+1)*sizeof(char));
	memcpy(var->value, instance->value, (valueLength+1)*sizeof(char));

	var->numberOfTypes = instance->numberOfTypes;
	var->types = (struct Type*)malloc(instance->numberOfTypes*sizeof(struct Type));

	for (int i = 0; i < instance->numberOfTypes; i++){
		var->types[i] = generateType(instance->types[i].code, instance->value, valueLength, processState);
	}

	return var;
}

int getSignificantType(struct CommonTypes* commonTypes, struct ProcessState* processState){
	int significant = -1;
	for (int i = 0; i < commonTypes->length; i++){
		if(commonTypes->codes[i] > significant){
			significant = commonTypes->codes[i];
		}
	}
	return significant;
}

struct CommonTypes getCommonTypes(struct Var* first, struct Var* second, struct ProcessState* processState){
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

struct Type* getType(int code, struct Var* var, struct ProcessState* processState){
	for (int i = 0; i < var->numberOfTypes; i++){
		if (var->types[i].code == code){
			return &var->types[i];
		}
	}
	return NULL;
}

void freeTypes(struct Type* types, unsigned int length, struct Var* var, struct ProcessState* processState){
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
		if (var->types[i].hasValue){
			free(var->types[i].type);
		}
	}
	free(var->types);
}

void freeVar(struct Var* var, struct ProcessState* processState){
	if (var->creationFlag){
		freeTypes(var->types, var->numberOfTypes, var, processState);
		free(var->name);
		free(var->value);
	}
}

void assignValue(struct Var* var, struct Var*other, struct ProcessState* processState){
	struct CommonTypes commonTypes = getCommonTypes(var, other, processState);

	if (commonTypes.length == 0){
		printf("cannot assign variable %s to variable %s\n", var->value, other->value);
		raiseError("", 1, processState);
	}

	char* value = other->value;
	unsigned int valueLength = strlen(value);

	freeTypes(var->types, var->numberOfTypes, var, processState);

	var->numberOfTypes = commonTypes.length;
	var->types = (struct Type*)malloc(commonTypes.length*sizeof(struct Type));

	for (int i = 0; i < commonTypes.length; i++){
		var->types[i] = generateType(commonTypes.codes[i], value, valueLength, processState);
	}

	free(var->value);
	var->value = (char*)malloc((valueLength+1)*sizeof(char));
	memcpy(var->value, value, (valueLength+1)*sizeof(char));

	free(commonTypes.codes);

}

struct Var* addVars(struct Var* first, struct Var* second, struct ProcessState* processState){
	struct CommonTypes commonTypes = getCommonTypes(first, second, processState);

	if (commonTypes.length == 0){
		printf("cannot add variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1, processState);
	}

	int code = getSignificantType(&commonTypes, processState);

	char* newValue;

	switch (code){
		case String_c : {
			struct String* string1 = (struct String*)getType(String_c, first, processState)->type;
			struct String* string2 = (struct String*)getType(String_c, second, processState)->type;

			unsigned int length = string1->size + string2->size + 1;
			newValue = (char*)malloc(length*sizeof(char));
			memcpy(newValue, string1->cString, string1->size*sizeof(char));
			memcpy(newValue+string1->size, string2->cString, (string2->size+1)*sizeof(char)); // copy null terminator
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first, processState)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second, processState)->type;

			long long new = int1->value + int2->value;
			newValue = (char*)malloc(20*sizeof(char)); // largest 64 bit signed integer
			sprintf(newValue, "%d", new);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first, processState)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second, processState)->type;

			float new = float1->value + float2->value;
			newValue = (char*)malloc(100*sizeof(char));
			gcvt(new, 100, newValue);
		}
	}

	struct Var* var = generateVar(commonTypes.codes, commonTypes.length, "unnamed", newValue, (struct Param*)NULL, processState);

	free(commonTypes.codes);
	free(newValue);

	return var;
}

struct Var* subVars(struct Var* first, struct Var* second, struct ProcessState* processState){
	struct CommonTypes commonTypes = getCommonTypes(first, second, processState);

	if (commonTypes.length == 0){
		printf("cannot sub variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1, processState);
	}

	int code = getSignificantType(&commonTypes, processState);

	char* newValue;

	switch (code){
		case String_c : {
			raiseError("string has no operator -\n", 1, processState);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first, processState)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second, processState)->type;

			long long new = int1->value - int2->value;
			newValue = (char*)malloc(20*sizeof(char)); // largest 64 bit signed integer
			sprintf(newValue, "%d", new);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first, processState)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second, processState)->type;

			float new = float1->value - float2->value;
			newValue = (char*)malloc(100*sizeof(char));
			gcvt(new, 100, newValue);
		}
	}

	struct Var* var = generateVar(commonTypes.codes, commonTypes.length, "unnamed", newValue, (struct Param*)NULL, processState);

	free(commonTypes.codes);
	free(newValue);

	return var;
}

struct Var* divVars(struct Var* first, struct Var* second, struct ProcessState* processState){
	struct CommonTypes commonTypes = getCommonTypes(first, second, processState);

	if (commonTypes.length == 0){
		printf("cannot divide variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1, processState);
	}

	int code = getSignificantType(&commonTypes, processState);

	char* newValue;

	switch (code){
		case String_c : {
			raiseError("string has no operator /\n", 1, processState);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first, processState)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second, processState)->type;

			long long new = int1->value / int2->value;
			newValue = (char*)malloc(20*sizeof(char)); // largest 64 bit signed integer
			sprintf(newValue, "%d", new);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first, processState)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second, processState)->type;

			float new = float1->value / float2->value;
			newValue = (char*)malloc(100*sizeof(char));
			gcvt(new, 100, newValue);
		}
	}

	struct Var* var = generateVarFromString(newValue, strlen(newValue), processState);
	free(commonTypes.codes);
	free(newValue);

	return var;
}

struct Var* mulVars(struct Var* first, struct Var* second, struct ProcessState* processState){
	struct CommonTypes commonTypes = getCommonTypes(first, second, processState);

	if (commonTypes.length == 0){
		printf("cannot multiply variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1, processState);
	}

	int code = getSignificantType(&commonTypes, processState);

	char* newValue;

	switch (code){
		case String_c : {
			raiseError("string has no operator *\n", 1, processState);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first, processState)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second, processState)->type;

			long long new = int1->value * int2->value;
			newValue = (char*)malloc(20*sizeof(char)); // largest 64 bit signed integer
			sprintf(newValue, "%d", new);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first, processState)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second, processState)->type;

			float new = float1->value * float2->value;
			newValue = (char*)malloc(100*sizeof(char));
			gcvt(new, 100, newValue);
		}
	}
	struct Var* var = generateVar(commonTypes.codes, commonTypes.length, "unnamed", newValue, (struct Param*)NULL, processState);

	free(commonTypes.codes);
	free(newValue);

	return var;
}

struct Var* lessThan(struct Var* first, struct Var* second, struct ProcessState* processState){
	struct CommonTypes commonTypes = getCommonTypes(first, second, processState);

	if (commonTypes.length == 0){
		printf("cannot compare variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1, processState);
	}

	int code = getSignificantType(&commonTypes, processState);

	char* newValue;

	switch (code){
		case String_c : {
			struct String* string1 = (struct String*)getType(String_c, first, processState)->type;
			struct String* string2 = (struct String*)getType(String_c, second, processState)->type;

			unsigned int result = string1->size < string2->size;
			newValue = (char*)malloc(1*sizeof(char));
			sprintf(newValue, "%d", result);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first, processState)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second, processState)->type;

			long long new = int1->value < int2->value;
			newValue = (char*)malloc(1*sizeof(char)); // largest 64 bit signed integer
			sprintf(newValue, "%d", new);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first, processState)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second, processState)->type;

			float new = float1->value < float2->value;
			newValue = (char*)malloc(100*sizeof(char));
			gcvt(new, 100, newValue);
		}
	}

	int codes[4] = {Int_c, Float_c, String_c, Char_c};

	struct Var* var = generateVar((int*)codes, 5, "unnamed", newValue, (struct Param*)NULL, processState);

	free(commonTypes.codes);

	return var;
}

struct Var* greaterThan(struct Var* first, struct Var* second, struct ProcessState* processState){
	struct CommonTypes commonTypes = getCommonTypes(first, second, processState);

	if (commonTypes.length == 0){
		printf("cannot compare variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1, processState);
	}

	int code = getSignificantType(&commonTypes, processState);

	char* newValue;

	switch (code){
		case String_c : {
			struct String* string1 = (struct String*)getType(String_c, first, processState)->type;
			struct String* string2 = (struct String*)getType(String_c, second, processState)->type;

			unsigned int result = string1->size > string2->size;
			newValue = (char*)malloc(1*sizeof(char));
			sprintf(newValue, "%d", result);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first, processState)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second, processState)->type;

			long long new = int1->value > int2->value;
			newValue = (char*)malloc(1*sizeof(char)); // largest 64 bit signed integer
			sprintf(newValue, "%d", new);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first, processState)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second, processState)->type;

			float new = float1->value > float2->value;
			newValue = (char*)malloc(100*sizeof(char));
			gcvt(new, 100, newValue);
		}
	}

	int codes[4] = {Int_c, Float_c, String_c, Char_c};

	struct Var* var = generateVar((int*)codes, 5, "unnamed", newValue, (struct Param*)NULL, processState);

	free(commonTypes.codes);

	return var;
}

struct Var* equalTo(struct Var* first, struct Var* second, struct ProcessState* processState){
	struct CommonTypes commonTypes = getCommonTypes(first, second, processState);

	if (commonTypes.length == 0){
		printf("cannot compare variable %s to variable %s\n", first->value, second->value);
		raiseError("", 1, processState);
	}

	int code = getSignificantType(&commonTypes, processState);

	char* newValue;

	switch (code){
		case String_c : {
			struct String* string1 = (struct String*)getType(String_c, first, processState)->type;
			struct String* string2 = (struct String*)getType(String_c, second, processState)->type;

			unsigned int result = 0;
			if (!strcmp(string1->cString, string2->cString)){
				result = 1;
			}
			newValue = (char*)malloc(1*sizeof(char));
			sprintf(newValue, "%d", result);
		} break;

		case Int_c : {
			struct Int* int1 = (struct Int*)getType(Int_c, first, processState)->type;
			struct Int* int2 = (struct Int*)getType(Int_c, second, processState)->type;

			long long new = int1->value == int2->value;
			newValue = (char*)malloc(1*sizeof(char)); // largest 64 bit signed integer
			sprintf(newValue, "%d", new);
		} break;

		case Float_c : {
			struct Float* float1 = (struct Float*)getType(Float_c, first, processState)->type;
			struct Float* float2 = (struct Float*)getType(Float_c, second, processState)->type;

			float new = float1->value == float2->value;
			newValue = (char*)malloc(100*sizeof(char));
			gcvt(new, 100, newValue);
		}
	}

	int codes[4] = {Int_c, Float_c, String_c, Char_c};

	struct Var* var = generateVar((int*)codes, 4, "unnamed", newValue, (struct Param*)NULL, processState);

	free(commonTypes.codes);

	return var;
}

int isTrue(struct Var* var, struct ProcessState* processState){
	struct CommonTypes* commonTypes = (struct CommonTypes*)malloc(sizeof(struct CommonTypes));
	commonTypes->length = var->numberOfTypes;
	commonTypes->codes = (int*)malloc(commonTypes->length*sizeof(int));

	for (int i = 0; i < var->numberOfTypes; i++){
		commonTypes->codes[i] = var->types[i].code;
	}

	int code = getSignificantType(commonTypes, processState);

	switch (code){
		case String_c : {
			struct String* string = (struct String*)getType(String_c, var, processState)->type;
			return strlen(string->cString);
		} break;

		case Int_c : {
			struct Int* intT = (struct Int*)getType(Int_c, var, processState)->type;
			return intT->value;
		} break;

		case Float_c : {
			struct Float* floatT = (struct Float*)getType(Float_c, var, processState)->type;
			return floatT->value;
		}
	}

	return 0;
}

void addKey(struct Key* keyP, char* name, unsigned int length, int key, struct ProcessState* processState){

	keyP->name = (char*)malloc(length*sizeof(char));
	memcpy(keyP->name, name, length*sizeof(char));

	keyP->key = key;
	keyP->length = length;
}

struct KeyChars createKeyChars(struct ProcessState* processState){
	struct KeyChars keyChars;

	keyChars.length = NUMBER_OF_KEYS;
	keyChars.keys = (struct Key*)malloc(NUMBER_OF_KEYS*sizeof(struct Key));

	char* keys[] = {"-=", "+=", "/=", "*=", "::", "<", ">", "==", ":", "?", ",", "=",
					"->", "{", "}", "[", "]",
					"(", ")", "-", "+", "/", "*"
					};

	for (int i = 0; i < NUMBER_OF_KEYS; i++){
		addKey(&keyChars.keys[i], keys[i], strlen(keys[i])+1, i, processState);
	}

	return keyChars;
}

void freeParam(struct Param* param, struct ProcessState* processState){
	for (int i = 0; i < param->inputCount; i++){
		freeVar(param->inputVars[i], processState);
		free(param->inputVars[i]);
	}
	free(param->inputVars);
	freeVar(param->returnValue, processState);
	free(param->returnValue);
	free(param);
}

struct Param* copyParam(struct Param* param, struct ProcessState* processState){
	struct Param* newParam = (struct Param*)malloc(sizeof(struct Param));

	newParam->inputCount = param->inputCount;
	newParam->inputVars = (struct Var**)malloc(param->inputCount*sizeof(struct Var*));
	for (int i = 0; i < param->inputCount; i++){
		newParam->inputVars[i] = copyVar(param->inputVars[i], processState);
	}
	newParam->returnValue = copyVar(param->returnValue, processState);

	return newParam;
}

struct VarScope* copyVarScope(struct VarScope* varScope, struct ProcessState* processState){
	struct VarScope* newVarScope = (struct VarScope*)malloc(sizeof(struct VarScope));
	newVarScope->numberOfVars = varScope->numberOfVars;
	newVarScope->isTrue = varScope->isTrue;
	newVarScope->vars = (struct Var**)malloc(sizeof(struct Var*) * varScope->numberOfVars);
	for (int i = 0; i < varScope->numberOfVars; i++){
		newVarScope->vars[i] = copyVar(varScope->vars[i], processState);
	}
	return newVarScope;
}

struct InheritedVarscopes* hardcopyInheritedVarscope(struct InheritedVarscopes* inheritedVarscopes, struct ProcessState* processState){
	struct InheritedVarscopes* newScopes = (struct InheritedVarscopes*)malloc(sizeof(struct InheritedVarscopes));

	newScopes->numberOfScopes = inheritedVarscopes->numberOfScopes;
	newScopes->scopes = (struct InheritedVarscopes**)malloc(newScopes->numberOfScopes*sizeof(struct InheritedVarscopes*));

	for (int i = 0; i < inheritedVarscopes->numberOfScopes; i++){
		newScopes->scopes[i] = copyVarScope(inheritedVarscopes->scopes[i], processState);
	}

	return newScopes;
}

struct InheritedVarscopes* copyInheritedVarscope(struct InheritedVarscopes* inheritedVarscopes, struct ProcessState* processState){
	struct InheritedVarscopes* newScopes = (struct InheritedVarscopes*)malloc(sizeof(struct InheritedVarscopes));

	newScopes->numberOfScopes = inheritedVarscopes->numberOfScopes;
	newScopes->scopes = (struct InheritedVarscopes**)malloc(newScopes->numberOfScopes*sizeof(struct InheritedVarscopes*));

	for (int i = 0; i < inheritedVarscopes->numberOfScopes; i++){
		newScopes->scopes[i] = inheritedVarscopes->scopes[i];
	}

	return newScopes;
}

struct Var* getVarFromInheritedScopes(struct InheritedVarscopes* inheritedVarscopes, char* varName, struct ProcessState* processState){
	for (int i = 0; i < inheritedVarscopes->numberOfScopes; i++){
		if (varExistsInScope(inheritedVarscopes->scopes[i], varName, processState)){
			return getVarFromScope(inheritedVarscopes->scopes[i], varName, processState);
		}
	}
	return NULL;
}

struct Var* getVarFromScopes(struct VarScope* localScope, struct VarScope* globalScope, char* varName, struct ProcessState* processState){
	if (varExistsInScope(localScope, varName, processState)) return getVarFromScope(localScope, varName, processState);
	if (varExistsInScope(globalScope, varName, processState)) return getVarFromScope(globalScope, varName, processState);

	printf("no variable named %s", varName);
	raiseError("", 1, processState);
	return NULL;
}

struct Var* getVarFromScope(struct VarScope* scope, char* varName, struct ProcessState* processState){
	for (int i = 0; i < scope->numberOfVars; i++){
		struct Var* var = scope->vars[i];
		//printf("check %s\n", var->name);
		if (!strcmp(varName, var->name)){
			return var;
		}
	}
	printf("no variable named %s", varName);
	raiseError("", 1, processState);
	return (struct Var*)NULL;
}

int varExistsInScope(struct VarScope* scope, char* varName, struct ProcessState* processState){
	for (int i = 0; i < scope->numberOfVars; i++){
		struct Var* var = scope->vars[i];
		if (!strcmp(varName, var->name)){
			return 1;
		}
	}
	return 0;
}

void addVarToScope(struct VarScope* scope, struct Var* var, struct ProcessState* processState){

	if (varExistsInScope(scope, var->name, processState)){
		printf("redefinition of variable %s\n", var->name);
		raiseError("", 1, processState);
	}

	scope->numberOfVars++;
	struct Var** newVars = (struct Var**)realloc(scope->vars, scope->numberOfVars*sizeof(struct Var*));
	if (newVars == NULL){
		raiseError("memory error", 1, processState);
	} else {
		scope->vars = newVars;
		scope->vars[scope->numberOfVars-1] = var;
	}
}

void addVarScope(struct InheritedVarscopes* scopes, struct VarScope* varScope, struct ProcessState* processState){
	scopes->numberOfScopes++;
	struct VarScope** newScopes = (struct VarScope**)realloc(scopes->scopes, scopes->numberOfScopes*sizeof(struct VarScope*));
	if (newScopes == NULL){
		raiseError("memory error on scope inheriting", 1, processState);
	} else {
		scopes->scopes = newScopes;
		scopes->scopes[scopes->numberOfScopes-1] = varScope;
	}
}