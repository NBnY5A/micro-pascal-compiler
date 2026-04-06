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

static Token *createNewToken(char *name, char *lexeme, TokenType type, int row, int column);

static Token *searchKeyInTable(HashTable *table, char *key);

static void *insertTokenInTable(HashTable *table, Token *token, char *key);

static int isReservedWord(const char *lexeme);

static int isReservedOperator(const char *lexeme);
