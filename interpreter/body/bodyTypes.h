#ifndef BODY_TYPES_H
#define BODY_TYPES_H

#include "../types/types.h"
#include "../builtins/builtins.h"

struct Routes {
	struct Route* routes;
};

struct State {
	struct KeyChars keyChars;
	struct Builtins* builtins;
	struct VarScope* globalScope;
	struct VarScope* localScope;
};


#endif