#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "./definitions/lexer.h";
#include "./definitions/token.h";


static const char *reserverdWords[] = {
    RESERVERD_WORD_PROGRAM,
    RESERVERD_WORD_VAR,
    RESERVERD_WORD_INTEGER,
    RESERVERD_WORD_REAL,
    RESERVERD_WORD_BEGIN,
    RESERVERD_WORD_END,
    RESERVERD_WORD_IF,
    RESERVERD_WORD_THEN,
    RESERVERD_WORD_ELSE,
    RESERVERD_WORD_WHILE,
    RESERVERD_WORD_DO,
};

static const char *reserverdOperators[] = {
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
    RESERVERD_OP_ASS
};

HashTable *initTable()
{
    HashTable *table = (HashTable *)calloc(1, sizeof(HashTable));

    if (table == NULL) return NULL;

    return table;
}

static int generateHashKey(char *key)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return (int)(hash % HASHTABLE_SIZE);
}

static Token *createNewToken(char *name, char *lexeme, TokenType type, int row, int column)
{
    Token *token = (Token *) malloc(sizeof(Token));

    token->name = name;
    token->lexeme = lexeme;
    token->type = type;
    token->row = row;
    token->column = column;
    token->next = NULL;

    return token;
}

static void *insertTokenInTable(HashTable *table, Token *token, char *key) 
{
    int index = generateHashKey(key);

    Entry *entry = (Entry *) malloc(sizeof(Entry));

    if (entry == NULL) return NULL;

    entry->key = key;
    entry->token = token;
    entry->next = NULL;
    entry->previous = NULL;

    if (table->buckets[index] == NULL) 
    {
        table->buckets[index] = entry;
    }
    else
	{
		Entry *current = table->buckets[index];
		while (current->next != NULL)
		{
			current = current->next;
		}
		current->next = entry;
		entry->previous = current;
	}
}

static Token *searchKeyInTable(HashTable *table, char *key)
{
    int index = generateHashKey(key);
    Entry *entry = table->buckets[index];

    while (entry != NULL)
    {
        if (strcmp(entry->key, key) == 0) return entry->token;

        entry = entry->next;
    }
    
    return NULL;
}

static int isReservedWord(const char *lexeme) 
{
    if (lexeme == NULL) return 0;

    const int reservedWordsSize = sizeof(reserverdWords) / sizeof(char *);

	for (int size = 0; size < reservedWordsSize; size++)
	{
		if (strcasecmp(lexeme, reserverdWords[size]) == 0) return 1;
	}

	return 0;
}

static int isReserverdOperator(const char *lexeme)
{
    if (lexeme == NULL) return 0;

    const int reserverdOperatorSize = sizeof(reserverdOperators) / sizeof(char *);

    for (int size = 0; size < reserverdOperatorSize; size++) {
        if (strcmp(lexeme, reserverdOperators[size]) == 0) return 1;
    }

    return 0;
}

