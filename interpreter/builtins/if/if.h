#ifndef IF_H
#define IF_H

#include "../../types/types.h"
#include "../../body/functionLogic/functionLogic.h"
#include "../../body/bodyTypes.h"
#include "../../fileReader/fileReader.h"
#include "../builtins.h"

struct Var* if_implementation(struct Param* params, struct State* state, struct ProcessState* processState);
struct Var* else_implementation(struct Param* params, struct State* state, struct ProcessState* processState);

#endif