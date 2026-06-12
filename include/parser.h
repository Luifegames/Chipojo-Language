#ifndef PARSER_H
#define PARSER_H

#include "chipojo.h"
#include "variables.h"

int is_truthy(Value v);
Value expression();
Value logical_or();
Value logical_and();
Value comparison_expr();
Value arith_expr();
Value term();
Value factor();
Value comparison_op(Value left, TokenType op, Value right, int line);
Value parse_postfix(Value base);
Value *parse_postfix_ref(Value *base);
Value get_property(Value object, char *property, int line);
Value define_dict();
Value define_list();
Value if_stmt();
Value block();
Value function_call(Value func_val, Value *args,int count,int last_indx);
void forward();
void consume(TokenType type, char *message);
void assignation();
void assign_compound(char *name, TokenType op, double val, int op_line);
void assign_compound_ref(Value *target, TokenType op, Value rhs, int line);
void skip_block();
void define_new_function();
void set_value(Value *target, Value value);
void while_stmt();
void print_dict(Dict *dict);
void print_list(List *list);
void program();

#endif