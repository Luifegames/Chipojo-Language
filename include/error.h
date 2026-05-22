#ifndef ERROR_H
#define ERROR_H

#include "lizard.h"

const char *expected_string(TokenType type);
void syntax_error(const char *message, Token token);
void syntax_error_line(const char *message, int line);
void runtime_error(const char *message);
void undefined_variable_error(const char *var_name, int line);
void type_error(const char *var_name, const char *expected, const char *found, int line);

#endif