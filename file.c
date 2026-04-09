#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./definitions/file.h"

FILE *input = NULL;
FILE *output = NULL;

void saveFile(Token *token)
{
    fprintf(output, "<%d, %s, \"%s\"> : <%d, %d>\n",
            token->type, token->name, token->lexeme, token->row, token->column);
}

char *createPath(const char *filename)
{
    const char *baseName = strrchr(filename, '/');
    baseName = baseName ? baseName + 1 : filename;

    size_t outputNameLen = strlen(baseName) + 5;
    char *outputName = (char *)malloc(outputNameLen);

    if (outputName == NULL)
    {
        return NULL;
    }

    strcpy(outputName, baseName);

    char *dot = strrchr(outputName, '.');
    if (dot != NULL)
    {
        strcpy(dot, ".lex");
    }
    else
    {
        strcat(outputName, ".lex");
    }

    size_t outputPathLen = strlen("./lexical-tables/") + strlen(outputName) + 1;
    char *outputPath = (char *)malloc(outputPathLen);

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