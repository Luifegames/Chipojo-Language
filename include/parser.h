#ifndef PARSER_H
#define PARSER_H

#include "lizard.h"
#include "variables.h"

void forward();
void consume(TypeToken type, char *message);
Value expression();
Value define_dict();
Value comparison_expr();
Value arith_expr();
Value term();
Value factor();
void assignation();
void print_stmt();
void value_print(Dict *dict, char *buffer, size_t buffsize);
void defineNewFunction();
Value function_call(char *name, Value v[],int count,int last_indx);
Value if_stmt();
Value block();
void skip_block();
void program();

#endif