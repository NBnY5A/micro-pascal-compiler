#pragma once

#include "./lexer.h"

typedef struct ASTNode
{
    int type;
    char *value;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

ASTNode *createNode(int type, const char *value);

void freeNode(ASTNode *node);

ASTNode *parseConditional(HashTable *table, Entry **currentEntry);

ASTNode *parseRepetitive(HashTable *table, Entry **currentEntry);

ASTNode *parseFactor(HashTable *table, Entry **currentEntry);

ASTNode *parseTerm(HashTable *table, Entry **currentEntry);

ASTNode *parseSimpleExpression(HashTable *table, Entry **currentEntry);

ASTNode *parseExpression(HashTable *table, Entry **currentEntry);

ASTNode *parseAssignment(HashTable *table, Entry **currentEntry);

ASTNode *parseStatement(HashTable *table, Entry **currentEntry);

ASTNode *parseCompoundStatement(HashTable *table, Entry **currentEntry);

ASTNode *parseIdentifierList(HashTable *table, Entry **currentEntry);

ASTNode *parseDeclaration(HashTable *table, Entry **currentEntry);

ASTNode *parseVarDeclaration(HashTable *table, Entry **currentEntry);

ASTNode *parseBlock(HashTable *table, Entry **currentEntry);

ASTNode *parseProgram(HashTable *table, Entry **currentEntry);

ASTNode *parseTokens(HashTable *table);