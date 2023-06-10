#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdio.h>
#include <stdlib.h>
#include "fileReader/fileReader.h"
#include "header/findHeader.h"
#include "types/types.h"
#include "body/interpretBody.h"
#include "body/bodyTypes.h"
#include "builtins/builtins.h"
#include "httpParser/httpParser.h"

int interpret(FILE *file, struct ProcessState* processState);

#endif