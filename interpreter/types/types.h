#ifndef TYPES_H
#define TYPES_H

typedef void(*AddString)(struct String* string, char*other);

enum TypeCodes{
	String,
	Int16,
	Int32,
	Int64,
	Float,
	Bool,
	Char,
	User,
	Function,
	Empty,
	Any
};

union Types{
	struct String* string;
	struct Int16* int16;
	struct Int32* int32;
	struct Int64* int64;
	struct Float* float_t;
	struct Bool* bool_t;
	struct Char* char_t;
};

struct TypeTable{
	int*table;
	union Types types;
};
struct TypeTable getOffsetTable();

struct Operation{
	char*type;
	char**operations;
	void**functions;
};

struct OperationsTable{
	struct Operation* operation;
};

struct Type{
	enum typeCodes code;
	void* type;
	void* operation;
};
struct Type generateType(enum TypeCodes code, struct TypeTable* offsetTable);

struct Var{
	char* name;
	struct Type* types;
};
struct Var generateVar(enum TypeCodes* codes, char* name, struct TypeTable* offsetTable);

struct VarTable{
	struct Var* vars;
	unsigned int size;
	unsigned int length;
};
struct VarTable createVarTable();
void addToVarTable(struct VarTable* varTable, struct Var* var);

struct CustomType{
	char* name;
	struct Type* types;
};
struct CustomType generateCustomType(char*name, enum TypeCodes* codes, struct TypeTable* offsetTable);

struct String{
	char* cString;
	unsigned int size;
	unsigned int length;
};
struct String newString(char*initialValue);
void addString(struct String* string, char*other);

struct Int16{
	short int value;
};

struct Int32{
	int value;
};

struct Int64{
	long long value;
};

struct Float{
	float value;
};

struct Bool{
	int8_t value;
};

struct Char{
	char value;
};

struct User{

};

struct Function{

};

struct Empty{

};

struct Any{

};

#endif