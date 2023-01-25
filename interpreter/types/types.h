#ifndef TYPES_H
#define TYPES_H

enum typeCodes{
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

struct Var{
	typeCodes type;
	void*
}

struct String{
	char* cString;
	unsigned int size;
	unsigned int length;
};

struct Int16{

};

struct Int32{

};

struct Int64{

};

struct Float{

};

struct Bool{

};

struct Char{

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