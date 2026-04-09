#include <stdlib.h>
#include <string.h>

#include "definitions/lexer.h"
#include "definitions/file.h"
#include "definitions/parser.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Argument not specified, use:\n\t--help\n");
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
    {
        printf("Usage:\n\t--file <file>\t\tReads a pascal file and do the lexical analysis\n");
        return 0;
    }

    if ((strcmp(argv[1], "--file") != 0 && strcmp(argv[1], "-f") != 0) || argc < 3)
    {
        printf("File not specified:\n\t--file <file>\n");
        return 1;
    }

    const char *inputName = argv[2];
    const char *fileExt = strrchr(inputName, '.');

    if (!fileExt || strcmp(fileExt, ".pas") != 0)
    {
        printf("Invalid file extension. Please provide a .pas file.\n");
        return 1;
    }

    char *outputPath = createPath(inputName);
    if (outputPath == NULL)
    {
        fprintf(stderr, "Failed to create output path.\n");
        return 1;
    }

    input = fopen(inputName, "r");
    if (input == NULL)
    {
        printf("File not found:\n\t--file <file>\n");
        free(outputPath);
        return 1;
    }

    output = fopen(outputPath, "w");
    if (output == NULL)
    {
        fprintf(stderr, "Unable to create output file: %s\n", outputPath);
        fclose(input);
        free(outputPath);
        return 1;
    }

    HashTable *table = initTable();
    if (table == NULL)
    {
        fprintf(stderr, "Unable to allocate token table.\n");
        fclose(input);
        fclose(output);
        free(outputPath);
        return 1;
    }

    Token *token = NULL;
    do
    {
        token = lexerAnalysis(table);
    } while (token != NULL && token->type != ERROR && token->type != END_OF_FILE);

    free(token);

    Entry *entry = table->buckets[0];
    while (entry != NULL)
    {
        printf("<%d, %s, '%s'> : <%d, %d>\n",
               entry->token->type,
               entry->token->name,
               entry->token->lexeme,
               entry->token->row,
               entry->token->column);
        saveFile(entry->token);
        entry = entry->next;
    }

    ASTNode *ast = parseTokens(table);
    freeNode(ast);

    freeTable(table);
    fclose(input);
    fclose(output);
    free(outputPath);

    return 0;
}