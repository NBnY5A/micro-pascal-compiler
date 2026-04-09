#pragma once

#define HASHTABLE_SIZE 500

typedef enum TokenType
{
    RESERVED_WORD,
    RESERVED_TYPE,
    RESERVED_OPERATOR,
    IDENTIFIER,
    SYMBOL,
    STRING,
    NUMBER,
    ERROR,
    END_OF_FILE
} TokenType;

typedef struct Token
{
    char *name;
    char *lexeme;
    TokenType type;
    int row;
    int column;
    struct Token *next;
} Token;

typedef struct Entry
{
    char *key;
    Token *token;
    struct Entry *previous;
    struct Entry *next;
} Entry;

typedef struct HashTable
{
    Entry *buckets[HASHTABLE_SIZE];
} HashTable;


HashTable *initTable(void);
void freeTable(HashTable *table);
Token *lexerAnalysis(HashTable *table);

int generateHashKey(char *key);

Token *createNewToken(const char *name, const char *lexeme, TokenType type, int row, int column);

Token *searchKeyInTable(HashTable *table, char *key);

void insertTokenInTable(HashTable *table, Token *token);

int isReservedWord(const char *lexeme);

int isReserverdOperator(const char *lexeme);

void addWord(char **word, int *size, const char ch);

void removeWord(char **word, int *size);

int isValidIdentifier(const char *word);

int isReservedType(const char *word);
