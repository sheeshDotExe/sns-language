#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "../errorHandler/errorHandler.h"
#include "../filePatterns/patternReader.h"
#include "interpretType.h"

enum TypeCodes {
	String_c,
	Char_c,
	Bool_c,
	Float_c,
	Int_c,
	Array_c,
	User_c,
	Function_c,
	Empty_c,
	Any_c
};

enum StatusCodes {
	Return_s,
};

enum KeyCodes {
	SubtractAssign_k, // -=
	AdditionAssign_k, // +=
	DivisionAssign_k, // /=
	MultiplicationAssign_k, // *=

	NameSpace_k, // ::

	LessThan_k, // <
	GreaterThan_k, // >
	EqualTo_k, // ==

	NewVar_k, // :
	Type_k, // ?
	Param_k, // ,
	Assign_k, // =

	Function_k, // ->
	FuncStart_k, // {
	FuncEnd_k, // }
	FuncTypeStart_k, // [
	FuncTypeEnd_k, // ]
	FuncCallStart_k, // (
	FuncCallEnd_k, // )

	Subtract_k, // - 16
	Addition_k, // +
	Division_k, // /
	Multiplication_k, // *

	SplitBySpace, // " "
};

#define NUMBER_OF_TYPES 9
#define NUMBERS L"0123456789-"
#define NUMBERS_WITH_DOT L"0123456789.-"

#define NUMBER_OF_KEYS 23
#define NUMBER_OF_OPERATORS 10

struct Key {
	char* name;
	unsigned int length;
	int key;
};

struct KeyChars {
	struct Key* keys;
	unsigned int length;
};
void addKey(struct Key* keyP, char* name, unsigned int length, int key);
struct KeyChars createKeyChars();

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
	int i;
};

struct Empty {
	int i;
};

struct Any {
	int i;
};

struct Type {
	int code;
	int hasValue;
	void* type;
};
struct Type generateType(int code, char* value, unsigned int length);

struct Var {
	int creationFlag;
	char* name;
	char* value;
	unsigned int numberOfTypes;
	struct Type* types;
	struct Param* param;
	struct Param* originalParam;
	int hasParam;
	struct Function* function;
	int hasFunction;
	int isBuiltin;
	int returned;
	int shouldExecute;
	int assignable;
	int inheritScopes;
};
struct Param {
	struct Var** inputVars;
	unsigned int inputCount;
	struct Var* returnValue;
};

struct VarScope{
	unsigned int numberOfVars;
	struct Var** vars;

	struct Var* currentVar;
	int hasCurrentVar;
	int isTrue;

	unsigned int numberOfFunctions;
	struct Function* functions;
};

struct InheritedVarscopes{
	struct VarScope** scopes;
	unsigned int numberOfScopes;
};

struct Var* generateVar(int* codes, unsigned int numberOfTypes, char* name, char* value, struct Param* param);
struct Var* copyVar(struct Var* instance);
struct Param* copyParam(struct Param* param);
struct VarScope* copyVarScope(struct VarScope* varScope);
struct InheritedVarscopes* copyInheritedVarscope(struct InheritedVarscopes* inheritedVarscopes);
void addVarScope(struct InheritedVarscopes* scopes, struct VarScope* varScope);

void freeParam(struct Param* param);

struct Instruction {
	void* function;
	void** args;
	int* argType;
	unsigned int argc;
};

struct DefinitionLine {
	char* value;
	unsigned int length;
};

struct DefinitionLines {
	struct DefinitionLine* lines;
	unsigned int length;
};

struct Function {
	struct VarScope* varScope;
	struct DefinitionLines* lines;
};

struct Var* getVarFromInheritedScopes(struct InheritedVarscopes* inheritedVarscopes, char* varName);
struct Var* getVarFromScopes(struct VarScope* localScope, struct VarScope* globalScope, char* varName);
struct Var* getVarFromScope(struct VarScope* scope, char* varName);
void addVarToScope(struct VarScope* scope, struct Var* var);

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

void freeVar(struct Var* var);
void assignString(struct String* string, char* value, unsigned int length);
void assignInt(struct Int* int_s, char* value, unsigned int length);
void assignFloat(struct Float* float_s, char* value, unsigned int length);
void assignFunction(struct Function* function, char* value, unsigned int length);

struct Type* getType(int code, struct Var* var);
int getSignificantType(struct CommonTypes* commonTypes);
struct CommonTypes getCommonTypes(struct Var* first, struct Var* second);

void assignValue(struct Var* var, struct Var* other); // assign others value to var
struct Var* addVars(struct Var* first, struct Var* second); // add vars and return new var with merged types
struct Var* subVars(struct Var* first, struct Var* second);
struct Var* divVars(struct Var* first, struct Var* second);
struct Var* mulVars(struct Var* first, struct Var* second);
struct Var* lessThan(struct Var* first, struct Var* second);
struct Var* greaterThan(struct Var* first, struct Var* second);
struct Var* equalTo(struct Var* first, struct Var* second);

int isTrue(struct Var* var);

int stringToBool(char*string);
int isFloat(char*string, unsigned int length);
int isNum(char*string, unsigned int length);

int isString(char*value, unsigned int length);
//struct CommonTypes getValidTypes(char*value, unsigned int length);
//struct Var* generateVarFromString(char*value, unsigned int length);

#endif