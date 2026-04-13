#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "definitions/lexer.h"
#include "definitions/token.h"
#include "definitions/file.h"

static int column = 0;
static int row = 1;
static int ch = 0;

static const char *reservedWords[] = {
    RESERVERD_WORD_PROGRAM,
    RESERVERD_WORD_VAR,
    RESERVERD_WORD_BEGIN,
    RESERVERD_WORD_END,
    RESERVERD_WORD_IF,
    RESERVERD_WORD_THEN,
    RESERVERD_WORD_ELSE,
    RESERVERD_WORD_WHILE,
    RESERVERD_WORD_DO,
};

static const char *reservedTypes[] = {
    RESERVERD_WORD_INTEGER,
    RESERVERD_WORD_REAL,
};

static const char *reservedOperators[] = {
    RESERVERD_OP_AD,
    RESERVERD_OP_DIV,
    RESERVERD_OP_MUL,
    RESERVERD_OP_MIN,
    RESERVERD_OP_EQ,
    RESERVERD_OP_NE,
    RESERVERD_OP_LT,
    RESERVERD_OP_LE,
    RESERVERD_OP_GT,
    RESERVERD_OP_GE,
    RESERVERD_OP_ASS,
    "and",
    "or",
    "not",
};

char *duplicateString(const char *value)
{
    if (value == NULL)
    {
        return NULL;
    }

    size_t len = strlen(value) + 1;
    char *copy = (char *)malloc(len);

    if (copy == NULL)
    {
        return NULL;
    }

    memcpy(copy, value, len);
    return copy;
}

HashTable *initTable(HashTableMode mode)
{
    HashTable *table = (HashTable *) calloc(1, sizeof(HashTable));
    if (table == NULL)
    {
        return NULL;
    }
    
    table->mode = mode;

    if (mode == SYMBOL_TABLE)
    {
        for (size_t i = 0; i < sizeof(reservedWords) / sizeof(reservedWords[0]); i++)
        {
            Token *token = createNewToken("Reserved-word", reservedWords[i], RESERVED_WORD, 0, 0);
            if (token != NULL)
            {
                insertTokenInTable(table, token);
            }
        }

        for (size_t i = 0; i < sizeof(reservedTypes) / sizeof(reservedTypes[0]); i++)
        {
            Token *token = createNewToken("Reserved-type", reservedTypes[i], RESERVED_TYPE, 0, 0);
            if (token != NULL)
            {
                insertTokenInTable(table, token);
            }
        }
    }

    return table;
}

int generateHashKey(const char *key)
{
    if (key == NULL)
    {
        return 0;
    }

    unsigned long hash = 5381;
    for (int i = 0; key[i] != '\0'; i++)
    {
        hash = ((hash << 5) + hash) + (unsigned long)tolower((unsigned char)key[i]);
    }

    return (int)(hash % HASHTABLE_SIZE);
}

Token *searchKeyInTable(HashTable *table, const char *key)
{
    if (table == NULL || key == NULL)
    {
        return NULL;
    }

    Entry *entry = table->buckets[0];
    while (entry != NULL)
    {
        if (entry->key != NULL && strcasecmp(entry->key, key) == 0)
        {
            return entry->token;
        }

        entry = entry->next;
    }

    return NULL;
}

void freeTable(HashTable *table)
{
    if (table == NULL)
    {
        return;
    }

    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        Entry *entry = table->buckets[i];

        while (entry != NULL)
        {
            Entry *next = entry->next;

            if (entry->token != NULL)
            {
                free(entry->token->name);
                free(entry->token->lexeme);
                free(entry->token);
            }

            free(entry);
            entry = next;
        }
    }

    free(table);
}

Token *createNewToken(const char *name, const char *lexeme, TokenType type, int tokenRow, int tokenColumn)
{
    Token *token = (Token *)malloc(sizeof(Token));

    if (token == NULL)
    {
        return NULL;
    }

    token->name = duplicateString(name);
    token->lexeme = duplicateString(lexeme);
    token->type = type;
    token->row = tokenRow;
    token->column = tokenColumn;
    token->next = NULL;

    if (token->name == NULL || token->lexeme == NULL)
    {
        free(token->name);
        free(token->lexeme);
        free(token);
        return NULL;
    }

    return token;
}

void insertTokenInTable(HashTable *table, Token *token)
{
    if (table == NULL || token == NULL)
    {
        return;
    }

    if (table->mode == SYMBOL_TABLE)
    {
        if (token->type != IDENTIFIER && token->type != RESERVED_WORD && token->type != RESERVED_TYPE)
        {
            free(token->name);
            free(token->lexeme);
            free(token);
            return;
        }

        for (int i = 0; token->lexeme[i] != '\0'; i++)
        {
            token->lexeme[i] = (char)tolower((unsigned char)token->lexeme[i]);
        }

        if (searchKeyInTable(table, token->lexeme) != NULL)
        {
            free(token->name);
            free(token->lexeme);
            free(token);
            return;
        }
    }

    Entry *entry = (Entry *)malloc(sizeof(Entry));

    if (entry == NULL)
    {
        free(token->name);
        free(token->lexeme);
        free(token);
        return;
    }

    entry->key = token->lexeme;
    entry->token = token;
    entry->next = NULL;
    entry->previous = NULL;

    int index = 0;

    if (table->buckets[index] == NULL)
    {
        table->buckets[index] = entry;
        return;
    }

    Entry *current = table->buckets[index];
    while (current->next != NULL)
    {
        current = current->next;
    }

    current->next = entry;
    entry->previous = current;
}

int isInList(const char *lexeme, const char *const *list, size_t listSize, int ignoreCase)
{
    if (lexeme == NULL)
    {
        return 0;
    }

    for (size_t i = 0; i < listSize; i++)
    {
        int equal = ignoreCase ? (strcasecmp(lexeme, list[i]) == 0) : (strcmp(lexeme, list[i]) == 0);

        if (equal)
        {
            return 1;
        }
    }

    return 0;
}

int isReservedWord(const char *lexeme)
{
    return isInList(lexeme, reservedWords, sizeof(reservedWords) / sizeof(reservedWords[0]), 1);
}

int isReservedOperator(const char *lexeme)
{
    return isInList(lexeme, reservedOperators, sizeof(reservedOperators) / sizeof(reservedOperators[0]), 1);
}

int isReservedType(const char *word)
{
    return isInList(word, reservedTypes, sizeof(reservedTypes) / sizeof(reservedTypes[0]), 1);
}

int isValidIdentifier(const char *word)
{
    if (word == NULL || word[0] == '\0')
    {
        return 0;
    }

    if (!(word[0] == '_' || isalpha((unsigned char)word[0])))
    {
        return 0;
    }

    for (int i = 1; word[i] != '\0'; i++)
    {
        if (!(word[i] == '_' || isalnum((unsigned char)word[i])))
        {
            return 0;
        }
    }

    return 1;
}

void addWord(char **word, int *size, char current)
{
    *word = (char *)realloc(*word, (size_t)(*size + 2));

    if (*word == NULL)
    {
        return;
    }

    (*word)[(*size)++] = current;
    (*word)[*size] = '\0';
}

int isSymbolChar(int c)
{
    return c == RESERVERD_SMB_SEM[0] || c == RESERVERD_SMB_OPA[0] || c == RESERVERD_SMB_CPA[0] ||
           c == RESERVERD_SMB_DOT[0] || c == RESERVERD_SMB_COM[0] || c == RESERVERD_SMB_COL[0];
}

int isOperatorStartChar(int c)
{
    return c == RESERVERD_OP_AD[0] || c == RESERVERD_OP_MIN[0] || c == RESERVERD_OP_DIV[0] ||
           c == RESERVERD_OP_MUL[0] || c == RESERVERD_OP_LT[0] || c == RESERVERD_OP_GT[0] ||
           c == RESERVERD_OP_EQ[0];
}

Token *emitToken(HashTable *tokenTable, HashTable *symbolTable, char **word, const char *name, TokenType type, int tokenRow, int tokenColumn)
{
    Token *token = createNewToken(name, *word, type, tokenRow, tokenColumn);

    if (token == NULL)
    {
        free(*word);
        *word = NULL;
        return NULL;
    }

    insertTokenInTable(tokenTable, token);

    if (symbolTable != NULL && (type == IDENTIFIER || type == RESERVED_WORD || type == RESERVED_TYPE))
    {
        Token *symbolToken = createNewToken(name, *word, type, tokenRow, tokenColumn);
        if (symbolToken != NULL)
        {
            insertTokenInTable(symbolTable, symbolToken);
        }
    }

    free(*word);
    *word = NULL;
    return token;
}

Token *lexerAnalysis(HashTable *tokenTable, HashTable *symbolTable)
{
    char *word = (char *)malloc(1);

    if (word == NULL)
    {
        return NULL;
    }

    word[0] = '\0';

    int state = 0;
    int size = 0;
    int tokenRow = row;
    int tokenColumn = column;

    while ((ch = fgetc(input)) != EOF)
    {
        column++;

        switch (state)
        {
        case 0:
            if (ch == SPC_BLANK_SPACE || ch == SPC_TAB || ch == SPC_NEW_LINE || ch == '\r')
            {
                if (ch == SPC_NEW_LINE)
                {
                    row++;
                    column = 0;
                }
                break;
            }

            if (ch == RESERVERD_SMB_OBC[0])
            {
                tokenRow = row;
                tokenColumn = column;
                state = 7;
                size = 0;
                word[0] = '\0';
                break;
            }

            if (ch == '\'')
            {
                tokenRow = row;
                tokenColumn = column;
                addWord(&word, &size, (char)ch);
                state = 6;
                break;
            }

            if (isdigit((unsigned char)ch))
            {
                tokenRow = row;
                tokenColumn = column;
                addWord(&word, &size, (char)ch);
                state = 2;
                break;
            }

            if (isalpha((unsigned char)ch) || ch == '_')
            {
                tokenRow = row;
                tokenColumn = column;
                addWord(&word, &size, (char)ch);
                state = 1;
                break;
            }

            if (isSymbolChar(ch))
            {
                tokenRow = row;
                tokenColumn = column;
                addWord(&word, &size, (char)ch);

                if (ch == RESERVERD_SMB_COL[0])
                {
                    state = 5;
                    break;
                }

                return emitToken(tokenTable, symbolTable, &word, "Symbol", SYMBOL, tokenRow, tokenColumn);
            }

            if (isOperatorStartChar(ch))
            {
                tokenRow = row;
                tokenColumn = column;
                addWord(&word, &size, (char)ch);

                if (ch == RESERVERD_OP_AD[0] || ch == RESERVERD_OP_MIN[0] ||
                    ch == RESERVERD_OP_DIV[0] || ch == RESERVERD_OP_MUL[0])
                {
                    if (ch == RESERVERD_OP_DIV[0])
                    {
                        int peek = fgetc(input);

                        if (peek == RESERVERD_OP_DIV[0])
                        {
                            column++;

                            while ((ch = fgetc(input)) != EOF && ch != SPC_NEW_LINE)
                            {
                                column++;
                            }

                            if (ch == SPC_NEW_LINE)
                            {
                                row++;
                                column = 0;
                            }

                            size = 0;
                            word[0] = '\0';
                            state = 0;
                            break;
                        }

                        if (peek != EOF)
                        {
                            ungetc(peek, input);
                        }
                    }

                    return emitToken(tokenTable, symbolTable, &word, "Binary Arithmetic Operator", RESERVED_OPERATOR, row, column);
                }

                state = 4;
                break;
            }

             if (errorOutput != NULL)
            {
                fprintf(errorOutput, "INVALID_CHARACTER %d %d\n", row, column);
            }

            free(word);
            return createNewToken("ERROR", "INVALID_CHARACTER", ERROR, row, column);

        case 1:
            if (isalnum((unsigned char)ch) || ch == '_')
            {
                addWord(&word, &size, (char)ch);
                break;
            }

            ungetc(ch, input);
            column--;

            if (!isValidIdentifier(word))
            {
                if (errorOutput != NULL)
                {
                    fprintf(errorOutput, "INVALID_IDENTIFIER %d %d\n", tokenRow, tokenColumn);
                }

                free(word);
                return createNewToken("ERROR", "INVALID_IDENTIFIER", ERROR, tokenRow, tokenColumn);
            }

            if (isReservedWord(word))
            {
                for (int i = 0; word[i] != '\0'; i++) word[i] = (char)tolower((unsigned char)word[i]);
                return emitToken(tokenTable, symbolTable, &word, "Reserved-word", RESERVED_WORD, tokenRow, tokenColumn);
            }

            if (isReservedType(word))
            {
                for (int i = 0; word[i] != '\0'; i++) word[i] = (char)tolower((unsigned char)word[i]);
                return emitToken(tokenTable, symbolTable, &word, "Reserved-type", RESERVED_TYPE, tokenRow, tokenColumn);
            }

            if (isReservedOperator(word))
            {
                for (int i = 0; word[i] != '\0'; i++) word[i] = (char)tolower((unsigned char)word[i]);
                return emitToken(tokenTable, symbolTable, &word, "Reserved-operator", RESERVED_OPERATOR, tokenRow, tokenColumn);
            }

            return emitToken(tokenTable, symbolTable, &word, "Identifier", IDENTIFIER, tokenRow, tokenColumn);

        case 2:
            if (isdigit((unsigned char)ch))
            {
                addWord(&word, &size, (char)ch);
                break;
            }

            if (ch == RESERVERD_SMB_DOT[0])
            {
                addWord(&word, &size, (char)ch);
                state = 3;
                break;
            }

            if (isalpha((unsigned char)ch) || ch == '_')
            {
                addWord(&word, &size, (char)ch);

                if (errorOutput != NULL)
                {
                    fprintf(errorOutput, "INVALID_IDENTIFIER %d %d\n", tokenRow, tokenColumn);
                }

                free(word);
                return createNewToken("ERROR", "INVALID_IDENTIFIER", ERROR, tokenRow, tokenColumn);
            }

            ungetc(ch, input);
            column--;
            return emitToken(tokenTable, symbolTable, &word, "Integer number", NUMBER, tokenRow, tokenColumn);

        case 3:
            if (isdigit((unsigned char)ch))
            {
                addWord(&word, &size, (char)ch);
                break;
            }

            if (isalpha((unsigned char)ch) || ch == '_')
            {
                addWord(&word, &size, (char)ch);

                if (errorOutput != NULL)
                {
                    fprintf(errorOutput, "INVALID_NUMBER %d %d\n", tokenRow, tokenColumn);
                }

                free(word);
                return createNewToken("ERROR", "INVALID_NUMBER", ERROR, tokenRow, tokenColumn);
            }

            ungetc(ch, input);
            column--;
            return emitToken(tokenTable, symbolTable, &word, "Real number", NUMBER, tokenRow, tokenColumn);

        case 4:
            if ((ch == RESERVERD_OP_EQ[0] &&
                 (word[size - 1] == RESERVERD_OP_GT[0] || word[size - 1] == RESERVERD_OP_LT[0])) ||
                (ch == RESERVERD_OP_GT[0] && word[size - 1] == RESERVERD_OP_LT[0]))
            {
                addWord(&word, &size, (char)ch);
                break;
            }

            ungetc(ch, input);
            column--;
            return emitToken(tokenTable, symbolTable, &word, "Relational Operator", RESERVED_OPERATOR, tokenRow, tokenColumn);

        case 5:
            if (ch == RESERVERD_OP_EQ[0] && word[size - 1] == RESERVERD_SMB_COL[0])
            {
                addWord(&word, &size, (char)ch);
                return emitToken(tokenTable, symbolTable, &word, "Assignment Operator", RESERVED_OPERATOR, tokenRow, tokenColumn);
            }

            ungetc(ch, input);
            column--;
            return emitToken(tokenTable, symbolTable, &word, "Symbol", SYMBOL, tokenRow, tokenColumn);

        case 6:
            if (ch == SPC_NEW_LINE)
            {
                if (errorOutput != NULL)
                {
                    fprintf(errorOutput, "UNCLOSED_STRING %d %d\n", tokenRow, tokenColumn);
                }

                row++;
                column = 0;
                free(word);
                return createNewToken("ERROR", "UNCLOSED_STRING", ERROR, tokenRow, tokenColumn);
            }

            addWord(&word, &size, (char)ch);

            if (ch == '\'')
            {
                return emitToken(tokenTable, symbolTable, &word, "String", STRING, tokenRow, tokenColumn);
            }

            break;

        case 7:
            if (ch == RESERVERD_SMB_CBC[0])
            {
                state = 0;
                size = 0;
                word[0] = '\0';
                break;
            }

            if (ch == SPC_NEW_LINE)
            {
                row++;
                column = 0;
            }

            break;

        default:
            if (errorOutput != NULL)
            {
                fprintf(errorOutput, "UNKNOWN_STATE %d %d\n", row, column);
            }

            free(word);
            return createNewToken("ERROR", "UNKNOWN_STATE", ERROR, row, column);
        }
    }

    if (state == 7)
    {
        if (errorOutput != NULL)
        {
            fprintf(errorOutput, "UNCLOSED_COMMENT %d %d\n", tokenRow, tokenColumn);
        }

        free(word);
        return createNewToken("ERROR", "UNCLOSED_COMMENT", ERROR, tokenRow, tokenColumn);
    }

    if (state == 6)
    {
        if (errorOutput != NULL)
        {
            fprintf(errorOutput, "UNCLOSED_STRING %d %d\n", tokenRow, tokenColumn);
        }

        free(word);
        return createNewToken("ERROR", "UNCLOSED_STRING", ERROR, tokenRow, tokenColumn);
    }

    if (size > 0)
    {
        if (state == 1)
        {
            if (isReservedWord(word))
            {
                for (int i = 0; word[i] != '\0'; i++) word[i] = (char)tolower((unsigned char)word[i]);
                return emitToken(tokenTable, symbolTable, &word, "Reserved-word", RESERVED_WORD, tokenRow, tokenColumn);
            }

            if (isReservedType(word))
            {
                for (int i = 0; word[i] != '\0'; i++) word[i] = (char)tolower((unsigned char)word[i]);
                return emitToken(tokenTable, symbolTable, &word, "Reserved-type", RESERVED_TYPE, tokenRow, tokenColumn);
            }

            if (isReservedOperator(word))
            {
                for (int i = 0; word[i] != '\0'; i++) word[i] = (char)tolower((unsigned char)word[i]);
                return emitToken(tokenTable, symbolTable, &word, "Reserved-operator", RESERVED_OPERATOR, tokenRow, tokenColumn);
            }

            return emitToken(tokenTable, symbolTable, &word, "Identifier", IDENTIFIER, tokenRow, tokenColumn);
        }

        if (state == 2)
        {
            return emitToken(tokenTable, symbolTable, &word, "Integer number", NUMBER, tokenRow, tokenColumn);
        }

        if (state == 3)
        {
            return emitToken(tokenTable, symbolTable, &word, "Real number", NUMBER, tokenRow, tokenColumn);
        }

        if (state == 4)
        {
            return emitToken(tokenTable, symbolTable, &word, "Relational Operator", RESERVED_OPERATOR, tokenRow, tokenColumn);
        }

        if (state == 5)
        {
            return emitToken(tokenTable, symbolTable, &word, "Symbol", SYMBOL, tokenRow, tokenColumn);
        }
    }

    free(word);
    return createNewToken("EOF", "EOF", END_OF_FILE, row, column);
}