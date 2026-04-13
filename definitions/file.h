#pragma once

#include <stdio.h>
#include "lexer.h"

extern FILE *input;
extern FILE *output;
extern FILE *tsOutput;
extern FILE *errorOutput;

void saveFile(Token *token);
char *createPath(const char *filename, const char *extension);