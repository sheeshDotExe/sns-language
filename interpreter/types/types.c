#include "types.h"

struct Type generateType(int code){
	struct Type type;

	type.code = code;
	switch (code) {
		case String_c : {
			type.type = (struct String*)malloc(sizeof(struct String));
		} break;
		case Int_c : {
			type.type = (struct Int*)malloc(sizeof(struct Int));
		} break;
		case Float_c : {
			type.type = (struct Float*)malloc(sizeof(struct Float));
		} break;
	}

	return type;
}

struct Var generateVar(int* codes, unsigned int numberOfTypes, char* name){
	struct Var var;

	unsigned int nameLength = strlen(name);
	var.name = (char*)malloc((nameLength+1)*sizeof(char));
	memcpy(var.name, name, nameLength+1);

	var.types = (struct Type*)malloc(numberOfTypes*sizeof(struct Type));

	for (int i = 0; i < numberOfTypes; i++){
		var.types[i] = generateType(codes[i]);
	}

	return var;
}

void testVar(){

	int codes[3] = {String_c, Int_c, Float_c};
	struct Var var = generateVar((int*)codes, 3, "HelloWorld");

	printf("%s\n",var.name);
}