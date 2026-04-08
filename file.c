#include <stdio.h>
#include <string.h>

#include "./definitions/file.h"

static void saveFile(Token *token) 
{
    fprintf("<%d, %s, \"%s\"> : <%d, %d>", 
        token->type, token->name, token->lexeme, token->row, token->column);
}

static char *createPath(const char *filename)
{
    const char *baseName = strrchr(filename, "/");

    baseName = baseName ? baseName + 1 : filename;

	size_t outputNameLen = strlen(baseName) + 5;
	char *outputName = malloc(outputNameLen);

	if (outputName == NULL)
	{
		return NULL;
	}

	strcpy(outputName, baseName);
	strcpy(strrchr(outputName, '.'), ".lex");

	size_t outputPathLen = strlen("./lexical-tables/") + strlen(outputName) + 1;
	char *outputPath = malloc(outputPathLen);

	if (outputPath == NULL)
	{
		free(outputName);
		return NULL;
	}

	strcpy(outputPath, "./lexical-tables/");
	strcat(outputPath, outputName);

	free(outputName);
	return outputPath;
}