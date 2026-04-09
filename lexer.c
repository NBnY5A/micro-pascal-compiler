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

HashTable *initTable(void)
{
    return (HashTable *)calloc(1, sizeof(HashTable));
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

    Entry *entry = (Entry *)malloc(sizeof(Entry));

    if (entry == NULL)
    {
        return;
    }

    entry->key = token->lexeme;
    entry->token = token;
    entry->next = NULL;
    entry->previous = NULL;

    // Mantém ordem de leitura para parser/main (lista linear em buckets[0]).
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
    return isInList(lexeme, reservedOperators, sizeof(reservedOperators) / sizeof(reservedOperators[0]), 0);
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
    return c == RESERVERD_SMB_OBC[0] || c == RESERVERD_SMB_CBC[0] || c == RESERVERD_SMB_SEM[0] ||
           c == RESERVERD_SMB_OPA[0] || c == RESERVERD_SMB_CPA[0] || c == RESERVERD_SMB_DOT[0] ||
           c == RESERVERD_SMB_COM[0] || c == RESERVERD_SMB_COL[0];
}

int isOperatorStartChar(int c)
{
    return c == RESERVERD_OP_AD[0] || c == RESERVERD_OP_MIN[0] || c == RESERVERD_OP_DIV[0] ||
           c == RESERVERD_OP_MUL[0] || c == RESERVERD_OP_LT[0] || c == RESERVERD_OP_GT[0];
}

Token *emitToken(HashTable *table, char **word, const char *name, TokenType type, int tokenRow, int tokenColumn)
{
    Token *token = createNewToken(*word, name, type, tokenRow, tokenColumn);

    if (token == NULL)
    {
        free(*word);
        *word = NULL;
        return NULL;
    }

    insertTokenInTable(table, token);

    free(*word);
    *word = NULL;
    return token;
}

Token *lexerAnalysis(HashTable *table)
{
    char *word = (char *)malloc(1);

    if (word == NULL)
    {
        return NULL;
    }

    word[0] = '\0';

    int state = 0;
    int size = 0;

    while ((ch = fgetc(input)) != EOF)
    {
        column++;

        switch (state)
        {
        case 0:
            if (ch == SPC_BLANK_SPACE || ch == SPC_TAB || ch == SPC_NEW_LINE)
            {
                if (ch == SPC_NEW_LINE)
                {
                    row++;
                    column = 0;
                }
                break;
            }

            if (isdigit((unsigned char)ch))
            {
                addWord(&word, &size, (char)ch);
                state = 2;
                break;
            }

            if (isalpha((unsigned char)ch) || ch == '_')
            {
                addWord(&word, &size, (char)ch);
                state = 1;
                break;
            }

            if (isSymbolChar(ch))
            {
                addWord(&word, &size, (char)ch);

                if (ch == RESERVERD_SMB_COL[0])
                {
                    state = 5;
                    break;
                }

                return emitToken(table, &word, "Symbol", SYMBOL, row, column);
            }

            if (isOperatorStartChar(ch))
            {
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

                    return emitToken(table, &word, "Binary Arithmetic Operator", RESERVED_OPERATOR, row, column);
                }

                state = 4;
                break;
            }

            fprintf(stderr, "Unknown character '%c' at <%d, %d>\n", (char)ch, row, column);
            free(word);
            return NULL;

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
                fprintf(stderr, "Invalid identifier \"%s\" at <%d, %d>\n", word, row, column);
                free(word);
                return NULL;
            }

            if (isReservedWord(word))
            {
                return emitToken(table, &word, "Reserved-word", RESERVED_WORD, row, column);
            }

            if (isReservedType(word))
            {
                return emitToken(table, &word, "Reserved-type", RESERVED_TYPE, row, column);
            }

            if (isReservedOperator(word))
            {
                return emitToken(table, &word, "Reserved-operator", RESERVED_OPERATOR, row, column);
            }

            return emitToken(table, &word, "Identifier", IDENTIFIER, row, column);

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
                fprintf(stderr, "Invalid identifier \"%s\" at <%d, %d>\n", word, row, column);
                free(word);
                return NULL;
            }

            ungetc(ch, input);
            column--;
            return emitToken(table, &word, "Integer number", NUMBER, row, column);

        case 3:
            if (isdigit((unsigned char)ch))
            {
                addWord(&word, &size, (char)ch);
                break;
            }

            ungetc(ch, input);
            column--;
            return emitToken(table, &word, "Real number", NUMBER, row, column);

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
            return emitToken(table, &word, "Relational Operator", RESERVED_OPERATOR, row, column);

        case 5:
            if (ch == RESERVERD_OP_EQ[0] && word[size - 1] == RESERVERD_SMB_COL[0])
            {
                addWord(&word, &size, (char)ch);
                return emitToken(table, &word, "Assignment Operator", RESERVED_OPERATOR, row, column);
            }

            ungetc(ch, input);
            column--;
            return emitToken(table, &word, "Symbol", SYMBOL, row, column);

        default:
            fprintf(stderr, "Unknown state %d at <%d, %d>\n", state, row, column);
            free(word);
            return NULL;
        }
    }

    if (size > 0)
    {
        if (state == 1)
        {
            if (isReservedWord(word))
            {
                return emitToken(table, &word, "Reserved-word", RESERVED_WORD, row, column);
            }

            if (isReservedType(word))
            {
                return emitToken(table, &word, "Reserved-type", RESERVED_TYPE, row, column);
            }

            if (isReservedOperator(word))
            {
                return emitToken(table, &word, "Reserved-operator", RESERVED_OPERATOR, row, column);
            }

            return emitToken(table, &word, "Identifier", IDENTIFIER, row, column);
        }

        if (state == 2)
        {
            return emitToken(table, &word, "Integer number", NUMBER, row, column);
        }

        if (state == 3)
        {
            return emitToken(table, &word, "Real number", NUMBER, row, column);
        }

        if (state == 4)
        {
            return emitToken(table, &word, "Relational Operator", RESERVED_OPERATOR, row, column);
        }

        if (state == 5)
        {
            return emitToken(table, &word, "Symbol", SYMBOL, row, column);
        }
    }

    free(word);
    return createNewToken("EOF", "EOF", END_OF_FILE, row, column);
}