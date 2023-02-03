#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../errorHandler/errorHandler.h"

enum TypeCodes {
	String_c,
	Char_c,
	Bool_c,
	Int_c,
	Float_c,
	Array_c,
	User_c,
	Function_c,
	Empty_c,
	Any_c
};

#define NUMBER_OF_TYPES 9
#define NUMBERS "0123456789"
#define NUMBERS_WITH_DOT "0123456789."

struct CustomType {
	char* name;
	struct Type* types;
};

struct String {
	char* cString;
	unsigned int size;
};

struct Int {
	long long value;
};

struct Float {
	float value;
};

struct Char {
	char value;
};

struct User {

};

struct Empty {

};

struct Any {

};

struct Type {
	int code;
	void* type;
};
struct Type generateType(int code, char* value, unsigned int length);

struct Var {
	char* name;
	char* value;
	unsigned int numberOfTypes;
	struct Type* types;
};
struct Var generateVar(int* codes, unsigned int numberOfTypes, char* name, char* value);

struct Instruction {
	void* function;
	void** args;
	int* argType;
	unsigned int argc;
};

struct Function {
	struct VarScope* varScope;
	struct Instruction* instructions;
	unsigned int numberOfInstructions;

	char* name;
	unsigned int id;
};

struct VarScope{
	unsigned int numberOfVars;
	struct Var* vars;

	unsigned int numberOfFunctions;
	struct Function* functions;
};

struct Array {
	struct Var* vars;
	unsigned int size;
	unsigned int length;
};
struct Array createArray();
void addToArray(struct Array* array, struct Var* var);

struct CustomType generateCustomType(char*name, int* codes);

struct CommonTypes {
	unsigned int length;
	int* codes;
};

void testVar();

void freeVar(struct Var* var);
void assignString(struct String* string, char* value, unsigned int length);
void assignInt(struct Int* int_s, char* value, unsigned int length);
void assignFloat(struct Float* float_s, char* value, unsigned int length);

struct Type* getType(int code, struct Var* var);
int getSignificantType(struct CommonTypes* commonTypes);
struct CommonTypes getCommonTypes(struct Var* first, struct Var* second);

void assignValue(struct Var* var, struct Var* other); // assign others value to var
struct Var addVars(struct Var* first, struct Var* second); // add vars and return new var with merged types
struct Var subVars(struct Var* first, struct Var* second);
struct Var divVars(struct Var* first, struct Var* second);
struct Var mulVars(struct Var* first, struct Var* second);
struct Var lessThan(struct Var* first, struct Var* second);
struct Var greaterThan(struct Var* first, struct Var* second);
struct Var equalTo(struct Var* first, struct Var* second);

int stringToBool(char*string);
int isFloat(char*string, unsigned int length);
int isNum(char*string, unsigned int length);

#endif