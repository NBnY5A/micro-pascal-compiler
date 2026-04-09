#pragma once

#include <stdio.h>
#include "lexer.h"

extern FILE *input;
extern FILE *output;

void saveFile(Token *token);
char *createPath(const char *filename);