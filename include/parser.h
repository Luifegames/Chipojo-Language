#ifndef PARSER_H
#define PARSER_H

#include "lizard.h"
#include "variables.h"

void forward();
void consume(TokenType type, char *message);
Value expression();
Value define_dict();
Value comparison_expr();
Value arith_expr();
Value term();
Value factor();
void assignation();
void dict_print(Dict *dict);
void list_print(List *list);
void define_new_function();
Value function_call(Value func_val, Value *args,int count,int last_indx);
Value if_stmt();
Value block();
void skip_block();
void program();

#endif