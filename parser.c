#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "definitions/lexer.h"
#include "definitions/parser.h"
#include "definitions/token.h"

static char *copyString(const char *value)
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

static int lexemeEquals(const Entry *entry, const char *lexeme)
{
    return entry != NULL && entry->token != NULL && strcmp(entry->token->lexeme, lexeme) == 0;
}

static void parserError(const Entry *entry, const char *message)
{
    if (entry && entry->token)
    {
        fprintf(stderr, "%s at <%d, %d>\n", message, entry->token->row, entry->token->column);
    }
    else
    {
        fprintf(stderr, "%s\n", message);
    }
}

static void parserUnexpected(const Entry *entry)
{
    if (entry && entry->token)
    {
        fprintf(stderr, "Unexpected token \"%s\" at <%d, %d>\n",
                entry->token->lexeme,
                entry->token->row,
                entry->token->column);
    }
    else
    {
        fprintf(stderr, "Unexpected end of input\n");
    }
}

static void parserAbort(const Entry *entry, const char *message)
{
    parserError(entry, message);
    exit(EXIT_FAILURE);
}

static void parserAbortFree(const Entry *entry, const char *message, ASTNode *node)
{
    freeNode(node);
    parserAbort(entry, message);
}

ASTNode *createNode(int type, const char *value)
{
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));

    if (!node)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    node->type = type;
    node->value = copyString(value);
    node->left = NULL;
    node->right = NULL;

    if (value != NULL && node->value == NULL)
    {
        free(node);
        fprintf(stderr, "Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    return node;
}

void freeNode(ASTNode *node)
{
    if (node)
    {
        freeNode(node->left);
        freeNode(node->right);
        free(node->value);
        free(node);
    }
}

static int isValidNumber(const char *str)
{
    int hasDecimalPoint = 0;

    if (str == NULL || *str == '\0')
    {
        return 0;
    }

    while (*str)
    {
        if (*str == '.')
        {
            if (hasDecimalPoint)
            {
                return 0;
            }
            hasDecimalPoint = 1;
        }
        else if (*str < '0' || *str > '9')
        {
            return 0;
        }
        str++;
    }

    return 1;
}

ASTNode *parseConditional(HashTable *table, Entry **currentEntry)
{
    Entry *entry = *currentEntry;

    if (!entry || entry->token->type != RESERVED_WORD || strcmp(entry->token->lexeme, RESERVERD_WORD_IF) != 0)
    {
        parserAbort(entry, "Expected \"if\" statement");
    }

    ASTNode *ifNode = createNode(entry->token->type, entry->token->lexeme);

    if (entry->next == NULL)
    {
        parserAbortFree(entry, "Expected expression after \"if\" statement", ifNode);
    }

    *currentEntry = entry->next;
    ifNode->left = parseExpression(table, currentEntry);

    entry = *currentEntry;
    if (!entry || entry->token->type != RESERVED_WORD || strcmp(entry->token->lexeme, RESERVERD_WORD_THEN) != 0)
    {
        parserAbortFree(entry, "Expected \"then\" statement", ifNode);
    }

    *currentEntry = entry->next;
    ifNode->right = parseStatement(table, currentEntry);

    entry = *currentEntry;
    if (entry && entry->token->type == RESERVED_WORD && strcmp(entry->token->lexeme, RESERVERD_WORD_ELSE) == 0)
    {
        *currentEntry = entry->next;
        ASTNode *elseNode = parseStatement(table, currentEntry);

        ASTNode *elseBranchNode = createNode(RESERVED_WORD, RESERVERD_WORD_ELSE);
        elseBranchNode->left = ifNode->right;
        elseBranchNode->right = elseNode;
        ifNode->right = elseBranchNode;
    }

    return ifNode;
}

ASTNode *parseRepetitive(HashTable *table, Entry **currentEntry)
{
    Entry *entry = *currentEntry;

    if (!entry || entry->token->type != RESERVED_WORD || strcmp(entry->token->lexeme, RESERVERD_WORD_WHILE) != 0)
    {
        parserAbort(entry, "Expected \"while\" statement");
    }

    ASTNode *whileNode = createNode(entry->token->type, entry->token->lexeme);

    if (entry->next == NULL)
    {
        parserAbortFree(entry, "Expected expression after \"while\" statement", whileNode);
    }

    *currentEntry = entry->next;
    whileNode->left = parseExpression(table, currentEntry);

    entry = *currentEntry;
    if (!entry || entry->token->type != RESERVED_WORD || strcmp(entry->token->lexeme, RESERVERD_WORD_DO) != 0)
    {
        parserAbortFree(entry, "Expected \"do\" statement", whileNode);
    }

    if (entry->next == NULL)
    {
        parserAbortFree(entry, "Expected expression after \"do\" statement", whileNode);
    }

    *currentEntry = entry->next;
    whileNode->right = parseStatement(table, currentEntry);

    return whileNode;
}

ASTNode *parseFactor(HashTable *table, Entry **currentEntry)
{
    Entry *entry = *currentEntry;
    ASTNode *factorNode = NULL;

    if (!entry)
    {
        parserAbort(entry, "Expected factor");
    }

    if (entry->token->type == NUMBER)
    {
        if (!isValidNumber(entry->token->lexeme))
        {
            parserAbort(entry, "Invalid number");
        }

        factorNode = createNode(entry->token->type, entry->token->lexeme);

        if (entry->next == NULL)
        {
            parserAbortFree(entry, "Expected expression or semicolon", factorNode);
        }

        *currentEntry = entry->next;
    }
    else if (entry->token->type == IDENTIFIER)
    {
        factorNode = createNode(entry->token->type, entry->token->lexeme);
        *currentEntry = entry->next;
    }
    else if (lexemeEquals(entry, RESERVERD_SMB_OPA))
    {
        if (entry->next == NULL)
        {
            parserAbort(entry, "Expected expression after open parentheses");
        }

        *currentEntry = entry->next;
        factorNode = parseExpression(table, currentEntry);
        entry = *currentEntry;

        if (entry && lexemeEquals(entry, RESERVERD_SMB_OPA))
        {
            factorNode = parseExpression(table, currentEntry);
            *currentEntry = entry->next;
            entry = *currentEntry;
        }

        if (entry == NULL || !lexemeEquals(entry, RESERVERD_SMB_CPA))
        {
            parserAbortFree(entry, "Expected close parentheses", factorNode);
        }

        if (entry->next == NULL)
        {
            parserAbortFree(entry, "Expected expression or semicolon", factorNode);
        }

        *currentEntry = entry->next;
    }
    else
    {
        parserUnexpected(entry);
        exit(EXIT_FAILURE);
    }

    return factorNode;
}

ASTNode *parseTerm(HashTable *table, Entry **currentEntry)
{
    ASTNode *termNode = parseFactor(table, currentEntry);
    Entry *entry = *currentEntry;

    while (entry && (lexemeEquals(entry, RESERVERD_OP_MUL) || lexemeEquals(entry, RESERVERD_OP_DIV)))
    {
        ASTNode *operatorNode = createNode(entry->token->type, entry->token->lexeme);

        if (entry->next == NULL)
        {
            freeNode(operatorNode);
            parserAbortFree(entry, "Expected factor after operator", termNode);
        }

        *currentEntry = entry->next;
        ASTNode *rightFactor = parseFactor(table, currentEntry);

        operatorNode->left = termNode;
        operatorNode->right = rightFactor;
        termNode = operatorNode;
        entry = *currentEntry;
    }

    return termNode;
}

ASTNode *parseSimpleExpression(HashTable *table, Entry **currentEntry)
{
    Entry *entry = *currentEntry;
    ASTNode *simpleExprNode = NULL;

    if (entry && entry->token->type == RESERVED_OPERATOR &&
        (lexemeEquals(entry, RESERVERD_OP_AD) || lexemeEquals(entry, RESERVERD_OP_MIN)))
    {
        simpleExprNode = createNode(entry->token->type, entry->token->lexeme);

        if (entry->next == NULL)
        {
            parserAbortFree(entry, "Expected term after operator", simpleExprNode);
        }

        *currentEntry = entry->next;
    }

    ASTNode *termNode = parseTerm(table, currentEntry);

    if (simpleExprNode)
    {
        simpleExprNode->left = termNode;
    }
    else
    {
        simpleExprNode = termNode;
    }

    entry = *currentEntry;

    while (entry && (lexemeEquals(entry, RESERVERD_OP_AD) || lexemeEquals(entry, RESERVERD_OP_MIN)))
    {
        ASTNode *operatorNode = createNode(entry->token->type, entry->token->lexeme);

        if (entry->next == NULL)
        {
            freeNode(operatorNode);
            parserAbortFree(entry, "Expected term after operator", simpleExprNode);
        }

        *currentEntry = entry->next;
        ASTNode *rightTermNode = parseTerm(table, currentEntry);

        operatorNode->left = simpleExprNode;
        operatorNode->right = rightTermNode;
        simpleExprNode = operatorNode;
        entry = *currentEntry;
    }

    return simpleExprNode;
}

ASTNode *parseExpression(HashTable *table, Entry **currentEntry)
{
    ASTNode *expressionNode = parseSimpleExpression(table, currentEntry);
    Entry *entry = *currentEntry;

    if (entry && (lexemeEquals(entry, RESERVERD_OP_EQ) || lexemeEquals(entry, RESERVERD_OP_NE) ||
                  lexemeEquals(entry, RESERVERD_OP_LT) || lexemeEquals(entry, RESERVERD_OP_LE) ||
                  lexemeEquals(entry, RESERVERD_OP_GT) || lexemeEquals(entry, RESERVERD_OP_GE) ||
                  lexemeEquals(entry, "and") || lexemeEquals(entry, "or") || lexemeEquals(entry, "not")))
    {
        ASTNode *relationNode = createNode(entry->token->type, entry->token->lexeme);

        if (entry->next == NULL)
        {
            freeNode(relationNode);
            parserAbortFree(entry, "Expected expression after operator", expressionNode);
        }

        *currentEntry = entry->next;
        ASTNode *rightExprNode = parseSimpleExpression(table, currentEntry);

        relationNode->left = expressionNode;
        relationNode->right = rightExprNode;
        expressionNode = relationNode;
    }

    return expressionNode;
}

ASTNode *parseAssignment(HashTable *table, Entry **currentEntry)
{
    Entry *entry = *currentEntry;

    if (!entry || entry->token->type != IDENTIFIER)
    {
        parserAbort(entry, "Expected identifier before \":=\"");
    }

    ASTNode *assignmentNode = createNode(RESERVED_OPERATOR, RESERVERD_OP_ASS);
    assignmentNode->left = createNode(entry->token->type, entry->token->lexeme);

    if (entry->next == NULL)
    {
        parserAbortFree(entry, "Expected \":=\"", assignmentNode);
    }

    *currentEntry = entry->next;
    entry = *currentEntry;

    if (!entry || !lexemeEquals(entry, RESERVERD_OP_ASS))
    {
        parserAbortFree(entry, "Expected \":=\"", assignmentNode);
    }

    if (entry->next == NULL)
    {
        parserAbortFree(entry, "Expected expression after \":=\"", assignmentNode);
    }

    *currentEntry = entry->next;
    assignmentNode->right = parseExpression(table, currentEntry);

    entry = *currentEntry;
    if (!entry || !lexemeEquals(entry, RESERVERD_SMB_SEM))
    {
        parserAbortFree(entry, "Unexpected token", assignmentNode);
    }

    *currentEntry = entry->next;
    return assignmentNode;
}

ASTNode *parseStatement(HashTable *table, Entry **currentEntry)
{
    Entry *entry = *currentEntry;

    if (entry == NULL)
    {
        parserAbort(entry, "Expected statement");
    }

    if (entry->token->type == IDENTIFIER)
    {
        return parseAssignment(table, currentEntry);
    }

    if (entry->token->type == RESERVED_WORD && strcmp(entry->token->lexeme, RESERVERD_WORD_VAR) == 0)
    {
        return parseVarDeclaration(table, currentEntry);
    }

    if (entry->token->type == RESERVED_WORD && strcmp(entry->token->lexeme, RESERVERD_WORD_BEGIN) == 0)
    {
        return parseCompoundStatement(table, currentEntry);
    }

    if (entry->token->type == RESERVED_WORD && strcmp(entry->token->lexeme, RESERVERD_WORD_IF) == 0)
    {
        return parseConditional(table, currentEntry);
    }

    if (entry->token->type == RESERVED_WORD && strcmp(entry->token->lexeme, RESERVERD_WORD_WHILE) == 0)
    {
        return parseRepetitive(table, currentEntry);
    }

    parserAbort(entry, "Invalid statement");
    return NULL;
}

ASTNode *parseCompoundStatement(HashTable *table, Entry **currentEntry)
{
    Entry *entry = *currentEntry;

    if (!entry || entry->token->type != RESERVED_WORD || strcmp(entry->token->lexeme, RESERVERD_WORD_BEGIN) != 0)
    {
        parserAbort(entry, "Expected \"begin\" statement");
    }

    ASTNode *compoundStmtNode = createNode(entry->token->type, entry->token->lexeme);

    if (entry->next == NULL)
    {
        parserAbortFree(entry, "Expected statement after \"begin\"", compoundStmtNode);
    }

    *currentEntry = entry->next;
    entry = *currentEntry;

    ASTNode *stmtListNode = NULL;
    ASTNode *lastStmtNode = NULL;

    while (entry != NULL && !lexemeEquals(entry, RESERVERD_WORD_END))
    {
        ASTNode *stmtNode = parseStatement(table, currentEntry);

        if (stmtListNode == NULL)
        {
            stmtListNode = stmtNode;
        }
        else
        {
            lastStmtNode->right = stmtNode;
        }

        lastStmtNode = stmtNode;
        entry = *currentEntry;

        if (entry == NULL)
        {
            parserAbortFree(entry, "Expected \"end\" statement", compoundStmtNode);
        }
    }

    if (!entry || !lexemeEquals(entry, RESERVERD_WORD_END))
    {
        parserAbortFree(entry, "Expected \"end\" statement", compoundStmtNode);
    }

    compoundStmtNode->left = stmtListNode;
    *currentEntry = entry->next;

    return compoundStmtNode;
}

ASTNode *parseIdentifierList(HashTable *table, Entry **currentEntry)
{
    (void)table;

    Entry *entry = *currentEntry;
    if (!entry || entry->token->type != IDENTIFIER)
    {
        parserAbort(entry, "Expected identifier");
    }

    ASTNode *head = NULL;
    ASTNode *tail = NULL;

    while (entry && entry->token->type == IDENTIFIER)
    {
        ASTNode *idNode = createNode(entry->token->type, entry->token->lexeme);

        if (head == NULL)
        {
            head = idNode;
        }
        else
        {
            tail->right = idNode;
        }

        tail = idNode;

        if (entry->next == NULL)
        {
            parserAbortFree(entry, "Expected comma or semicolon", head);
        }

        *currentEntry = entry->next;
        entry = *currentEntry;

        if (entry && lexemeEquals(entry, RESERVERD_SMB_COM))
        {
            if (entry->next == NULL || entry->next->token->type != IDENTIFIER)
            {
                parserAbortFree(entry, "Expected identifier after \",\"", head);
            }

            *currentEntry = entry->next;
            entry = *currentEntry;
            continue;
        }

        break;
    }

    return head;
}

ASTNode *parseDeclaration(HashTable *table, Entry **currentEntry)
{
    Entry *entry = *currentEntry;

    if (!entry)
    {
        parserAbort(entry, "Expected declaration");
    }

    ASTNode *declNode = createNode(entry->token->type, entry->token->lexeme);
    declNode->left = parseIdentifierList(table, currentEntry);

    entry = *currentEntry;
    if (!entry || !lexemeEquals(entry, RESERVERD_SMB_COL))
    {
        parserAbortFree(entry, "Expected \":\" after identifier list", declNode);
    }

    if (entry->next == NULL)
    {
        parserAbortFree(entry, "Expected type after \":\"", declNode);
    }

    *currentEntry = entry->next;
    entry = *currentEntry;

    if (entry->token->type != RESERVED_TYPE)
    {
        parserAbortFree(entry, "Expected type after \":\"", declNode);
    }

    declNode->right = createNode(entry->token->type, entry->token->lexeme);

    if (entry->next == NULL || !lexemeEquals(entry->next, RESERVERD_SMB_SEM))
    {
        parserAbortFree(entry, "Expected \";\"", declNode);
    }

    *currentEntry = entry->next;
    return declNode;
}

ASTNode *parseVarDeclaration(HashTable *table, Entry **currentEntry)
{
    Entry *entry = *currentEntry;

    if (!entry || !lexemeEquals(entry, RESERVERD_WORD_VAR))
    {
        parserAbort(entry, "Expected \"var\" declaration");
    }

    ASTNode *varDeclNode = createNode(entry->token->type, entry->token->lexeme);

    if (entry->next == NULL)
    {
        parserAbortFree(entry, "Expected identifier after \"var\"", varDeclNode);
    }

    *currentEntry = entry->next;
    entry = *currentEntry;

    ASTNode *declList = NULL;

    while (entry && entry->token->type == IDENTIFIER)
    {
        ASTNode *declNode = parseDeclaration(table, currentEntry);

        if (declList == NULL)
        {
            declList = declNode;
        }
        else
        {
            ASTNode *seqNode = createNode(SYMBOL, RESERVERD_SMB_SEM);
            seqNode->left = declList;
            seqNode->right = declNode;
            declList = seqNode;
        }

        entry = *currentEntry;

        if (!entry || !lexemeEquals(entry, RESERVERD_SMB_SEM))
        {
            parserAbortFree(entry, "Expected \";\" after declaration", varDeclNode);
        }

        *currentEntry = entry->next; // avança para próxima declaração ou begin
        entry = *currentEntry;
    }

    if (declList == NULL)
    {
        parserAbortFree(entry, "Expected identifier after \"var\"", varDeclNode);
    }

    varDeclNode->right = declList;
    return varDeclNode;
}

ASTNode *parseBlock(HashTable *table, Entry **currentEntry)
{
    Entry *entry = *currentEntry;

    if (!entry)
    {
        parserAbort(entry, "Expected block");
    }

    ASTNode *blockNode = createNode(entry->token->type, entry->token->lexeme);
    blockNode->left = parseVarDeclaration(table, currentEntry);
    blockNode->right = parseCompoundStatement(table, currentEntry);

    return blockNode;
}

ASTNode *parseProgram(HashTable *table, Entry **currentEntry)
{
    Entry *entry = *currentEntry;

    if (!entry || entry->token->type != RESERVED_WORD || strcmp(entry->token->lexeme, RESERVERD_WORD_PROGRAM) != 0)
    {
        parserAbort(entry, "Expected \"program\" statement");
    }

    ASTNode *programNode = createNode(entry->token->type, entry->token->lexeme);

    if (entry->next == NULL)
    {
        parserAbortFree(entry, "Expected identifier after \"program\"", programNode);
    }

    *currentEntry = entry->next;
    entry = *currentEntry;

    if (entry->token->type != IDENTIFIER)
    {
        parserAbortFree(entry, "Expected identifier after \"program\"", programNode);
    }

    ASTNode *idNode = createNode(entry->token->type, entry->token->lexeme);
    programNode->right = idNode;

    if (entry->next == NULL)
    {
        parserAbortFree(entry, "Expected \";\"", programNode);
    }

    *currentEntry = entry->next;
    entry = *currentEntry;

    if (!lexemeEquals(entry, RESERVERD_SMB_SEM))
    {
        parserAbortFree(entry, "Expected \";\"", programNode);
    }

    idNode->right = createNode(entry->token->type, entry->token->lexeme);

    if (entry->next == NULL)
    {
        parserAbortFree(entry, "Expected block after \"program\" declaration", programNode);
    }

    *currentEntry = entry->next;
    programNode->left = parseBlock(table, currentEntry);

    entry = *currentEntry;
    if (!entry || !lexemeEquals(entry, RESERVERD_SMB_DOT))
    {
        parserAbortFree(entry, "Expected \".\" after \"program\" block", programNode);
    }

    *currentEntry = entry->next;
    return programNode;
}

ASTNode *parseTokens(HashTable *table)
{
    if (table == NULL || table->buckets[0] == NULL)
    {
        fprintf(stderr, "No tokens to parse\n");
        exit(EXIT_FAILURE);
    }

    Entry *entry = table->buckets[0];
    return parseProgram(table, &entry);
}