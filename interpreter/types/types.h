#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "../errorHandler/errorHandler.h"
#include "../filePatterns/patternReader.h"
#include "interpretType.h"
#include "../processState.h"

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
void addKey(struct Key* keyP, char* name, unsigned int length, int key, struct ProcessState* processState);
struct KeyChars createKeyChars(struct ProcessState* processState);

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
struct Type generateType(int code, char* value, unsigned int length, struct ProcessState* processState);

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

struct Var* generateVar(int* codes, unsigned int numberOfTypes, char* name, char* value, struct Param* param, struct ProcessState* processState);
struct Var* copyVar(struct Var* instance, struct ProcessState* processState);
struct Param* copyParam(struct Param* param, struct ProcessState* processState);
struct VarScope* copyVarScope(struct VarScope* varScope, struct ProcessState* processState);
struct InheritedVarscopes* hardcopyInheritedVarscope(struct InheritedVarscopes* inheritedVarscopes, struct ProcessState* processState);
struct InheritedVarscopes* copyInheritedVarscope(struct InheritedVarscopes* inheritedVarscopes, struct ProcessState* processState);
void addVarScope(struct InheritedVarscopes* scopes, struct VarScope* varScope, struct ProcessState* processState);

void freeParam(struct Param* param, struct ProcessState* processState);

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

struct Var* getVarFromInheritedScopes(struct InheritedVarscopes* inheritedVarscopes, char* varName, struct ProcessState* processState);
struct Var* getVarFromScopes(struct VarScope* localScope, struct VarScope* globalScope, char* varName, struct ProcessState* processState);
struct Var* getVarFromScope(struct VarScope* scope, char* varName, struct ProcessState* processState);
void addVarToScope(struct VarScope* scope, struct Var* var, struct ProcessState* processState);

struct Array {
	struct Var* vars;
	unsigned int size;
	unsigned int length;
};
struct Array createArray(struct ProcessState* processState);
void addToArray(struct Array* array, struct Var* var, struct ProcessState* processState);

struct CustomType generateCustomType(char*name, int* codes, struct ProcessState* processState);

struct CommonTypes {
	unsigned int length;
	int* codes;
};

void freeVar(struct Var* var, struct ProcessState* processState);
void assignString(struct String* string, char* value, unsigned int length, struct ProcessState* processState);
void assignInt(struct Int* int_s, char* value, unsigned int length, struct ProcessState* processState);
void assignFloat(struct Float* float_s, char* value, unsigned int length, struct ProcessState* processState);
void assignFunction(struct Function* function, char* value, unsigned int length, struct ProcessState* processState);

struct Type* getType(int code, struct Var* var, struct ProcessState* processState);
int getSignificantType(struct CommonTypes* commonTypes, struct ProcessState* processState);
struct CommonTypes getCommonTypes(struct Var* first, struct Var* second, struct ProcessState* processState);

void assignValue(struct Var* var, struct Var* other, struct ProcessState* processState); // assign others value to var
struct Var* addVars(struct Var* first, struct Var* second, struct ProcessState* processState); // add vars and return new var with merged types
struct Var* subVars(struct Var* first, struct Var* second, struct ProcessState* processState);
struct Var* divVars(struct Var* first, struct Var* second, struct ProcessState* processState);
struct Var* mulVars(struct Var* first, struct Var* second, struct ProcessState* processState);
struct Var* lessThan(struct Var* first, struct Var* second, struct ProcessState* processState);
struct Var* greaterThan(struct Var* first, struct Var* second, struct ProcessState* processState);
struct Var* equalTo(struct Var* first, struct Var* second, struct ProcessState* processState);

int isTrue(struct Var* var, struct ProcessState* processState);

int stringToBool(char*string, struct ProcessState* processState);
int isFloat(char*string, unsigned int length, struct ProcessState* processState);
int isNum(char*string, unsigned int length, struct ProcessState* processState);

int isString(char*value, unsigned int length, struct ProcessState* processState);
//struct CommonTypes getValidTypes(char*value, unsigned int length, struct ProcessState* processState);
//struct Var* generateVarFromString(char*value, unsigned int length, struct ProcessState* processState);

#endif