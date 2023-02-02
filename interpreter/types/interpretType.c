#include "interpretType.h"

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
		free(*value);
		*value = (char*)malloc(2*sizeof(char));
		itoa(Bool_v, *value, 10);

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

struct Var generateVarFromString(char*value, unsigned int length){

	//value will be eaten

	char**valueP = &value;

	struct CommonTypes types = getValidTypes(valueP, &length);

	struct Var var = generateVar(types.codes, types.length, "unnamed", *valueP);

	free(types.codes);
	free(*valueP);

	return var;
}