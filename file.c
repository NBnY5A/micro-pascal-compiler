#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./definitions/file.h"

FILE *input = NULL;
FILE *output = NULL;
FILE *tsOutput = NULL;
FILE *errorOutput = NULL;

void saveFile(Token *token)
{
    fprintf(output, "<%s, %s>: <%d, %d>\n",
            token->name, token->lexeme, token->row, token->column);
}

char *createPath(const char *filename, const char *extension)
{
    const char *baseName = strrchr(filename, '/');
    baseName = baseName ? baseName + 1 : filename;

    size_t baseLen = strlen(baseName);
    const char *dot = strrchr(baseName, '.');
    size_t stemLen = (dot != NULL) ? (size_t)(dot - baseName) : baseLen;

    size_t outputNameLen = stemLen + strlen(extension) + 1;
    char *outputName = (char *)malloc(outputNameLen);
    if (outputName == NULL)
    {
        return NULL;
    }

    memcpy(outputName, baseName, stemLen);
    outputName[stemLen] = '\0';
    strcat(outputName, extension);

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