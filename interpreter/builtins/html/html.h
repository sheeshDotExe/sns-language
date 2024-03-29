#ifndef HTML_H
#define HTML_H

#include "../../types/types.h"
#include "../../body/functionLogic/functionLogic.h"
#include "../../body/bodyTypes.h"
#include "../../fileReader/fileReader.h"
#include "../builtins.h"
#include <unistd.h>
#include "../../processState.h"

struct Var *html(struct Param *params, struct State *state, struct ProcessState* processState);

#endif