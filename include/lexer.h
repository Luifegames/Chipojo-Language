#ifndef LEXER_H
#define LEXER_H

#include "lizard.h"
#include "error.h"


void nextChar();
char currentChar();
void jumpBlankspace();
void jumpBOM();
TokenType peek_next_token_type(void);
Token nextToken();
#endif