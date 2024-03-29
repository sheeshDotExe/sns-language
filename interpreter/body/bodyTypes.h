#ifndef BODY_TYPES_H
#define BODY_TYPES_H

#include "../types/types.h"
#include "../builtins/builtins.h"

struct Routes {
	struct Route** routes;
	unsigned int numberOfRoutes;
};

struct Files {
	struct UserFile** files;
	unsigned int numberOfFiles;
};

struct State {
	struct KeyChars keyChars;
	struct Builtins* builtins;
	struct VarScope* globalScope;
	struct VarScope* localScope;
	struct InheritedVarscopes* inheritedVarscopes;
	int useInheritence;
	struct Routes* routes;
	struct Files* files;
	char** fileExtension;
};

struct Routes* copy_routes(struct Routes* routes, struct ProcessState* processState);

#endif