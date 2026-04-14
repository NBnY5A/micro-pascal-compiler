# Micro Pascal Compiler (C)

Este repositório consiste na elaboração de um projeto acadêmico para a construção de um **analisador léxico** (AFD) para a linguagem MicroPascal (µ-Pascal). O analisador léxico tem como objetivo reconhecer os tokens da linguagem, identificar erros léxicos (**`.err`**), criar uma tabela de símbolos (**`.ts`**) e gerar um arquivo de saída (**`.lex`**) contendo os tokens reconhecidos da linguagem. O projeto foi modularizado em diferentes pastas para melhor organização e eventuais manutenções do projeto.

## 1. Estrutura de Arquivos

A modularização separa responsabilidades entre leitura/escrita de arquivos, análise léxica, análise sintática e definições compartilhadas:
```bash
├── definitions
│   ├── file.h
│   ├── lexer.h
│   ├── parser.h
│   └── token.h
├── file.c
├── lexer.c
├── lexical-tables
├── LICENSE
├── main.c
├── parser.c
├── README.md
└── tests
    ├── fail
    │   ├── test001.pas
    │   ├── test002.pas
    │   └── test003.pas
    └── pass
        ├── test001.pas
        ├── test002.pas
        └── test003.pas
```
### Explicações de cada arquivo
- `main.c`  
  Orquestra o fluxo: valida argumentos, abre arquivos, executa lexer, salva saídas e chama parser.
- `lexer.c`  
  Implementa o autômato finito determinístico (AFD), tokenização e manutenção das tabelas de tokens/símbolos.
- `parser.c`  
  Implementa o parser recursivo descendente e construção da AST (Abstract Syntax Tree).
- `file.c`  
  Rotinas de saída (`.lex`) e criação de caminhos de arquivos de saída.
- `definitions/lexer.h`  
  Tipos (`Token`, `Entry`, `HashTable` e `HashTableMode`) o quais são estruturas e assinaturas de métodos que serão utilizados por essas estruturas.
- `definitions/parser.h`  
  Estrutura `ASTNode` e assinaturas do parser.
- `definitions/file.h`  
  Assinaturas e `FILE*` globais para entrada/saída.
- `definitions/token.h`  
  Constantes léxicas (palavras reservadas, operadores, símbolos e caracteres especiais).
- `tests/pass/`  
  Casos esperados para compilação sem erro.
- `tests/fail/`  
  Casos esperados com erro léxico/sintático.
- `lexical-tables/`  
  Diretório de saída dos arquivos (`.lex`), (`.ts`) e (`.err`).

## 2. Estruturas de Dados (Structs)

```C
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
} TokenType;                        // Definição dos tipos de Tokens

typedef enum HashTableMode
{
    TOKEN_STREAM_TABLE = 0,
    SYMBOL_TABLE = 1                // Serve como um interruptor para alternar o estado da HashTable dependendo 
} HashTableMode;                    // de qual operação ela está sendo usada

typedef struct Token {
    char *name;                     // Nome da classe (ex.: "Identifier")
    char *lexeme;                   // Lexema reconhecido no código fonte
    TokenType type;                 // Enum interno do token
    int row;                        // Linha da ocorrência
    int column;                     // Coluna da ocorrência
    struct Token *next;             // Encadeamento auxiliar
} Token;

typedef struct Entry {
    char *key;                      // Chave textual para busca
    Token *token;                   // Token associado
    struct Entry *previous;         // Entrada anterior (lista dupla)
    struct Entry *next;             // Próxima entrada no bucket
} Entry;

typedef struct HashTable {
    Entry *buckets[HASHTABLE_SIZE]; // Vetor de buckets
    HashTableMode mode;             // TOKEN_STREAM_TABLE ou SYMBOL_TABLE
} HashTable;

typedef struct ASTNode {
    int type;                       // Tipo sintático/léxico
    char *value;                    // Valor textual do nó
    struct ASTNode *left;           // Filho esquerdo
    struct ASTNode *right;          // Filho direito
} ASTNode;
```

## 3. Módulo: `main.c`

**Funções**  
```C
int main(int argc, char **argv)
```
Executa o pipeline completo do compilador: valida CLI, cria caminhos de saída, abre arquivos, inicializa tabelas, executa o lexer em loop, escreve `.lex`/`.ts`, chama parser se não houver erro léxico e libera recursos.

---

## 3. Módulo: `file.c`

**Funções**  
```C void saveFile(Token *token)
```
Grava um token no arquivo `.lex` usando os campos `name`, `lexeme`, `row` e `column`.

```C
char *createPath(const char *filename, const char *extension)
```
Gera o caminho de saída em `./lexical-tables/`, preservando o nome base do arquivo de entrada e trocando a extensão.

---

## 3. Módulo: `lexer.c`

**Funções**  
```C
char *duplicateString(const char *value)
```
Duplica string com alocação dinâmica.

```C
HashTable *initTable(HashTableMode mode)
```
Inicializa tabela hash e, no modo de símbolos, pré-carrega palavras reservadas e tipos.

```C
int generateHashKey(const char *key)
```
Calcula hash textual (djb2 com normalização para minúsculas).

```C
Token *searchKeyInTable(HashTable *table, const char *key)
```
Busca token por chave na tabela (comparação case-insensitive).

```C
void freeTable(HashTable *table)
```
Libera todos os `Entry` e `Token` da tabela.

```C
Token *createNewToken(const char *name, const char *lexeme, TokenType type, int row, int column)
```
Cria e inicializa um token.

```C
void insertTokenInTable(HashTable *table, Token *token)
```
Insere token na tabela; no modo de símbolos aplica filtro de tipos, normalização case-insensitive e deduplicação.

```C
int isInList(const char *lexeme, const char *const *list, size_t listSize, int ignoreCase)
```
Função auxiliar de pertencimento em lista de palavras.

```C
int isReservedWord(const char *lexeme)
```
Verifica palavra reservada.

```C
int isReservedOperator(const char *lexeme)
```
Verifica operador reservado.

```C
int isReservedType(const char *word)
```
Verifica tipo reservado.

```C
int isValidIdentifier(const char *word)
```
Valida identificador (`[A-Za-z_][A-Za-z0-9_]*`).

```C
void addWord(char **word, int *size, char current)
```
Acrescenta caractere ao lexema em construção.

```C
int isSymbolChar(int c)
```
Verifica símbolo simples (`; , ( ) : .`).

```C
int isOperatorStartChar(int c)
```
Verifica início de operador (`+ - / * < > =`).

```C
Token *emitToken(HashTable *tokenTable, HashTable *symbolTable, char **word, const char *name, TokenType type, int tokenRow, int tokenColumn)
```
Finaliza token reconhecido, insere em tabela de tokens e, quando aplicável, em tabela de símbolos.

```C
Token *lexerAnalysis(HashTable *tokenTable, HashTable *symbolTable)
```
Executa o AFD de análise léxica: reconhece identificadores, números, strings, operadores, símbolos, comentários e erros léxicos; devolve um token por chamada.

---

## 3. Módulo: `parser.c`

**Funções**  
```C
ASTNode *createNode(int type, const char *value)
```
Cria nó da AST.

```C
void freeNode(ASTNode *node)
```
Libera recursivamente a AST.

```C
ASTNode *parseConditional(HashTable *table, Entry **currentEntry)
```
Processa estrutura `if ... then ... [else ...]`.

```C
ASTNode *parseRepetitive(HashTable *table, Entry **currentEntry)
```
Processa estrutura `while ... do ...`.

```C
ASTNode *parseFactor(HashTable *table, Entry **currentEntry)
```
Processa fator de expressão (número, identificador, expressão entre parênteses).

```C
ASTNode *parseTerm(HashTable *table, Entry **currentEntry)
```
Processa termo com multiplicação/divisão.

```C
ASTNode *parseSimpleExpression(HashTable *table, Entry **currentEntry)
```
Processa soma/subtração e sinal unário.

```C
ASTNode *parseExpression(HashTable *table, Entry **currentEntry)
```
Processa expressão relacional/lógica.

```C
ASTNode *parseAssignment(HashTable *table, Entry **currentEntry)
```
Processa atribuição `identificador := expressão ;`.

```C
ASTNode *parseStatement(HashTable *table, Entry **currentEntry)
```
Despacha para o tipo correto de comando.

```C
ASTNode *parseCompoundStatement(HashTable *table, Entry **currentEntry)
```
Processa bloco `begin ... end`.

```C
ASTNode *parseIdentifierList(HashTable *table, Entry **currentEntry)
```
Processa lista de identificadores separada por vírgula.

```C
ASTNode *parseDeclaration(HashTable *table, Entry **currentEntry)
```
Processa declaração de variáveis (`id_list : type ;`).

```C
ASTNode *parseVarDeclaration(HashTable *table, Entry **currentEntry)
```
Processa seção `var` e múltiplas declarações.

```C
ASTNode *parseBlock(HashTable *table, Entry **currentEntry)
```
Processa bloco principal (`var` + comando composto).

```C
ASTNode *parseProgram(HashTable *table, Entry **currentEntry)
```
Processa estrutura completa `program ... ; ... end .`.

```C
ASTNode *parseTokens(HashTable *table)
```
Ponto de entrada do parser para a tabela de tokens.

## 4. Como Compilar e Rodar os testes

### Compilar (Linux / GCC)

```bash
gcc main.c lexer.c parser.c file.c -o micro-pascal-compiler.out
```

### Rodar um arquivo específico

```bash
./micro-pascal-compiler.out --file tests/pass/test001.pas
```

### Rodar todos os testes de sucesso (`tests/pass`)

Crie um script `.sh` e dentro do script cole o conteúdo abaixo

```bash
for f in tests/pass/*.pas; do
  echo ">>> $f"
  ./micro-pascal-compiler.out --file "$f"
done
```

Depois rode o script no terminal:

```bash
sh <nome_do_seu_script>.sh
```

O mesmo processo funciona para os arquivos de falha

```bash
for f in tests/fail/*.pas; do
  echo ">>> $f"
  ./micro-pascal-compiler.out --file "$f"
done
```

### Saídas geradas

Para cada `arquivo.pas`, são gerados em `lexical-tables/`:

- `arquivo.lex` (tokens reconhecidos)
- `arquivo.ts` (tabela de símbolos)
- `arquivo.err` (erros léxicos)