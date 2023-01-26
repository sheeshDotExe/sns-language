#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum TypeCodes {
	String_c,
	Int_c,
	Float_c,
	Bool_c,
	Char_c,
	User_c,
	Function_c,
	Empty_c,
	Any_c
};

#define NUMBER_OF_TYPES 7

struct CustomType {
	char* name;
	struct Type* types;
};

struct String {
	char* cString;
	unsigned int size;
	unsigned int length;
};

struct Int {
	long long value;
};

struct Float {
	float value;
};

struct Bool{
	short int value;
};

struct Char {
	char value;
};

struct User {

};

struct Function {

};

struct Empty {

};

struct Any {

};

struct Type {
	int code;
	void* type;
};
struct Type generateType(int code);

struct Var {
	char* name;
	char* value;
	struct Type* types;
};
struct Var generateVar(int* codes, unsigned int numberOfTypes, char* name);

struct Array {
	struct Var* vars;
	unsigned int size;
	unsigned int length;
};
struct Array createArray();
void addToArray(struct Array* array, struct Var* var);

struct CustomType generateCustomType(char*name, int* codes);

void testVar();

void assignValue(struct Var* var, struct Var*other); // assign others value to var
struct Var addVars(struct Var* first, struct Var* second); // add vars and return new var with merged types
struct Var subVars(struct Var* first, struct Var* second);
struct Var divVars(struct Var* first, struct Var* second);
struct Var mulVars(struct Var* first, struct Var* second);
struct Var lessThan(struct Var* first, struct Var* second);
struct Var greaterThan(struct Var* first, struct Var* second);
struct Var equalTo(struct Var* first, struct Var* second);


#endif