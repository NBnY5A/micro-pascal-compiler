#include "lexer.h"

static void saveFile(Token *token);

static char *createPath(const char *filename);