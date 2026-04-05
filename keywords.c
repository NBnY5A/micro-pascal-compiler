#include <strings.h>
#include <stdio.h>

char *keywords [] = {
    "program",
    "var",
    "integer",
    "real",
    "begin",
    "end",
    "if",
    "then",
    "else",
    "while",
    "do"
};

const int KEYWORDS_LENGTH = 11;

int verifyIfIsKeyword(char *word) {
    for (size_t i = 0; i < KEYWORDS_LENGTH; i++) {
        if (strcasecmp(keywords[i], word) == 0) return 1;
    }
    return 0;
}
