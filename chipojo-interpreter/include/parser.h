#ifndef PARSER_H
#define PARSER_H

#include "chipojo.h"
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
void define_new_function_ex(int exported);
Value function_call(Value func_val, Value *args,int count,int last_indx);
Value if_stmt();
Value block();
void while_stmt(void);
void for_stmt(void);
void switch_stmt(void);
void try_stmt(void);
void throw_stmt(void);
void class_stmt(int exported);
void skip_block();
void import_stmt();
void from_stmt();
void program();

#endif
