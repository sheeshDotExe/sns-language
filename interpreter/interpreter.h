#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdio.h>
#include <stdlib.h>
#include "fileReader/fileReader.h"
#include "header/findHeader.h"
#include "types/types.h"
#include "types/interpretType.h"

int interpret(FILE *file);

#endif