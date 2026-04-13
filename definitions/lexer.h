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

typedef enum HashTableMode
{
    TOKEN_STREAM_TABLE = 0,
    SYMBOL_TABLE = 1
} HashTableMode;

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
    HashTableMode mode;
} HashTable;

HashTable *initTable(HashTableMode mode);

void freeTable(HashTable *table);

Token *lexerAnalysis(HashTable *tokenTable, HashTable *symbolTable);

int generateHashKey(const char *key);

Token *createNewToken(const char *name, const char *lexeme, TokenType type, int row, int column);

Token *searchKeyInTable(HashTable *table, const char *key);

void insertTokenInTable(HashTable *table, Token *token);

int isReservedWord(const char *lexeme);

int isReservedOperator(const char *lexeme);

void addWord(char **word, int *size, const char ch);

void removeWord(char **word, int *size);

int isValidIdentifier(const char *word);

int isReservedType(const char *word);