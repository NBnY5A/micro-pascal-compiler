#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

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

    char *lexPath = createPath(inputName, ".lex");
    char *tsPath = createPath(inputName, ".ts");
    char *errPath = createPath(inputName, ".err");

    if (lexPath == NULL || tsPath == NULL || errPath == NULL)
    {
        fprintf(stderr, "Failed to create output paths.\n");
        free(lexPath);
        free(tsPath);
        free(errPath);
        return 1;
    }

    input = fopen(inputName, "r");
    if (input == NULL)
    {
        printf("File not found:\n\t--file <file>\n");
        free(lexPath);
        free(tsPath);
        free(errPath);
        return 1;
    }

    output = fopen(lexPath, "w");
    tsOutput = fopen(tsPath, "w");
    errorOutput = fopen(errPath, "w");

    if (output == NULL || tsOutput == NULL || errorOutput == NULL)
    {
        fprintf(stderr, "Unable to create output files.\n");

        if (output != NULL) fclose(output);
        if (tsOutput != NULL) fclose(tsOutput);
        if (errorOutput != NULL) fclose(errorOutput);

        fclose(input);
        free(lexPath);
        free(tsPath);
        free(errPath);
        return 1;
    }

    HashTable *tokenTable = initTable(TOKEN_STREAM_TABLE);
    HashTable *symbolTable = initTable(SYMBOL_TABLE);

    if (tokenTable == NULL || symbolTable == NULL)
    {
        fprintf(stderr, "Unable to allocate token tables.\n");
        freeTable(tokenTable);
        freeTable(symbolTable);
        fclose(input);
        fclose(output);
        fclose(tsOutput);
        fclose(errorOutput);
        free(lexPath);
        free(tsPath);
        free(errPath);
        return 1;
    }

    int lexicalError = 0;
    Token *token = NULL;

    do
    {
        token = lexerAnalysis(tokenTable, symbolTable);

        if (token == NULL)
        {
            lexicalError = 1;
            break;
        }

        if (token->type == ERROR)
        {
            lexicalError = 1;
            free(token);
            token = NULL;
            break;
        }
    } while (token->type != END_OF_FILE);

    if (token != NULL)
    {
        free(token);
    }

    Entry *entry = tokenTable->buckets[0];
    while (entry != NULL)
    {
        saveFile(entry->token);
        entry = entry->next;
    }

    entry = symbolTable->buckets[0];
    while (entry != NULL)
    {
        fprintf(tsOutput, "<%s, %s>\n", entry->token->name, entry->token->lexeme);
        entry = entry->next;
    }

    if (!lexicalError)
    {
        ASTNode *ast = parseTokens(tokenTable);

        struct stat st = {0};

        if (stat("./ast-graphic", &st) == -1)
        {
            mkdir("./ast-graphic", 0777);
            mkdir("./ast-graphic/dot", 0777);
            mkdir("./ast-graphic/images", 0777);
        }
        else
        {
            if (stat("./ast-graphic/dot", &st) == -1) mkdir("./ast-graphic/dot", 0777);
            if (stat("./ast-graphic/images", &st) == -1) mkdir("./ast-graphic/images", 0777);
        }

        const char *baseName = strrchr(inputName, '/');
        baseName = baseName ? baseName + 1 : inputName;
        const char *dotExt = strrchr(baseName, '.');
        size_t stemLen = (dotExt != NULL) ? (size_t)(dotExt - baseName) : strlen(baseName);

        char dotPath[512];
        snprintf(dotPath, sizeof(dotPath), "./ast-graphic/dot/%.*s.dot", (int) stemLen, baseName);

        exportASTToDot(ast, dotPath);

        char pngPath[512];
        snprintf(pngPath, sizeof(pngPath), "./ast-graphic/images/%.*s.png", (int) stemLen, baseName);

        char command[2048];
        snprintf(command, sizeof(command), "dot -Tpng %s -o %s", dotPath, pngPath);
        
        int result = system(command);

        if (result != 0)
        {
            fprintf(stderr, "Failed to execute Graphviz dot command. Make sure Graphviz is installed.\n");
        }
        else
        {
            printf("AST graphic generated at: %s\n", pngPath);
        }
    
        freeNode(ast);
    }
    else
    {
        fprintf(stderr, "Lexical analysis failed. Could not be possible to generate the AST graphic. Parsing aborted.\n");
    }

    freeTable(tokenTable);
    freeTable(symbolTable);

    fclose(input);
    fclose(output);
    fclose(tsOutput);
    fclose(errorOutput);

    free(lexPath);
    free(tsPath);
    free(errPath);

    return lexicalError ? 1 : 0;
}