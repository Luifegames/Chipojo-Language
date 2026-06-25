#ifndef ERROR_H
#define ERROR_H

#include "chipojo.h"
#include <setjmp.h>

const char *expected_string(TokenType type);
void __attribute__((noreturn)) syntax_error(const char *message, Token token);
void __attribute__((noreturn)) syntax_error_line(const char *message, int line);
void __attribute__((noreturn)) runtime_error(const char *message);
void __attribute__((noreturn)) undefined_variable_error(const char *var_name, int line);
void __attribute__((noreturn)) type_error(const char *var_name, const char *expected, const char *found, int line);
void __attribute__((noreturn)) chipojo_throw(const char *message);
void error_try_push(jmp_buf *env);
void error_try_pop(void);
const char *error_last_message(void);

#endif
