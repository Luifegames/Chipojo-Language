#ifndef PARSER_H
#define PARSER_H

#include "lizard.h"

void forward();
void consume(TypeToken type, char *message);
int expression();
int arith_expr();
int term();
int factor();
void assignation();
void print_stmt();
void if_stmt();
void block();
void skip_block();
void program();

#endif