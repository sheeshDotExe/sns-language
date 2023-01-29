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
	if (isNum(value, length)){
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

}

struct Var addVars(struct Var* first, struct Var* second){
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

	struct Var var = generateVar(commonTypes.codes, commonTypes.length, "unnamed", newValue);

	return var;
}

struct Var subVars(struct Var* first, struct Var* second){
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

	struct Var var = generateVar(commonTypes.codes, commonTypes.length, "unnamed", newValue);

	return var;
}

struct Var divVars(struct Var* first, struct Var* second){
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

	struct Var var = generateVar(commonTypes.codes, commonTypes.length, "unnamed", newValue);

	return var;
}

struct Var mulVars(struct Var* first, struct Var* second){
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

	struct Var var = generateVar(commonTypes.codes, commonTypes.length, "unnamed", newValue);

	return var;
}

struct Var lessThan(struct Var* first, struct Var* second){
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

	struct Var var = generateVar((int*)codes, 5, "unnamed", newValue);

	return var;
}

struct Var greaterThan(struct Var* first, struct Var* second){
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

	struct Var var = generateVar((int*)codes, 5, "unnamed", newValue);

	return var;
}

struct Var equalTo(struct Var* first, struct Var* second){
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

	struct Var var = generateVar((int*)codes, 4, "unnamed", newValue);

	return var;
}

void testVar(){

	int codes[3] = {String_c, Int_c, Float_c};
	struct Var var = generateVar((int*)codes, 3, "HelloWorld", "1");

	int ccodes[2] = {String_c, Float_c};
	struct Var var1 = generateVar((int*)ccodes, 2, "HelloWorld2", "3.5");

	struct Var sum = greaterThan(&var, &var1);

	printf("%s\n", sum.value);
}