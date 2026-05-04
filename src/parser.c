#include "parser.h"
#include "lexer.h"
#include "variables.h"

Token current_token;

void forward() { current_token = nextToken(); }
void consume(TypeToken type, char *message)
{
    if (current_token.type == type)
        forward();
    else
    {
        printf("%s (wait %d, got %d)\n", message, type, current_token.type);
        exit(1);
    }
}

// Prototypes
int comparison_op(int left, TypeToken op, int right);
void concat_element(char *buffer, size_t buffsize);
void print_concat();

void if_stmt()
{
    consume(TOKEN_IF, "if error");
    int cond = expression();
    consume(TOKEN_LEFTBRACE, "{ error");

    int executed = 0;

    if (cond != 0)
    {
        // Execute if block
        block();
        executed = 1;
    }
    else
    {
        skip_block();
        
    }

    while (current_token.type == TOKEN_ELIF)
    {
        forward();
        int elif_cond = expression();
        consume(TOKEN_LEFTBRACE, "{ error");
        if (!executed && elif_cond != 0){
            block();
            executed = 1;
        }else{
            skip_block();
        }
    }
    

    // if exist else
    if (current_token.type == TOKEN_ELSE)
    {
        forward();
        consume(TOKEN_LEFTBRACE, "{ error");
        if (executed == 0)
        {
            // Execute if block
            block();
        }
        else
        {
           skip_block();

        }
    }


    // if exist else
    if (current_token.type == TOKEN_ELSE)
    {
        forward();
        consume(TOKEN_LEFTBRACE, "{ error");
        if (executed == 0)
        {
            // Execute if block
            block();
        }
        else
        {
            skip_block();
        }
    }
}

void while_stmt(void)
{
    int after_cond = indx;
    int after_while = -1;
    while (1)
    {
        indx = after_cond;
        forward();
        int cond = expression();
        consume(TOKEN_LEFTBRACE, "Error {");
        if (cond != 0)
        {
            block();
        }
        else
        {
            skip_block();
            after_while = indx;
            break;
        }
    }
    indx = after_while;
}

void skip_block()
{
    int brace_count = 1;
    while (brace_count > 0)
    {
        if (current_token.type == TOKEN_LEFTBRACE)
            brace_count++;
        else if (current_token.type == TOKEN_RIGHTBRACE)
            brace_count--;
        if (brace_count == 0)
            break;
        forward();
    }
    forward(); // consume
}

void block()
{
    while (current_token.type != TOKEN_RIGHTBRACE && current_token.type != TOKEN_EOF)
    {
        if (current_token.type == TOKEN_PRINT)
            print_stmt();
        else if (current_token.type == TOKEN_ID)
            assignation();
        else if (current_token.type == TOKEN_IF)
            if_stmt();
        
        else
        {
            printf("Error invalid sentence in block (token %d)\n", current_token.type);
            exit(1);
        }
    }
    consume(TOKEN_RIGHTBRACE, "error }");
}

int expression()
{
    int left = arith_expr();
    while (current_token.type == TOKEN_EQ || current_token.type == TOKEN_NE ||
           current_token.type == TOKEN_LT || current_token.type == TOKEN_GT ||
           current_token.type == TOKEN_LE || current_token.type == TOKEN_GE)
    {
        TypeToken op = current_token.type;
        forward();
        int right = arith_expr();
        left = comparison_op(left, op, right);
    }
    return left;
}

int arith_expr()
{
    int left = term();
    while (current_token.type == TOKEN_SUM || current_token.type == TOKEN_REST)
    {
        if (current_token.type == TOKEN_SUM)
        {
            forward();
            left += term();
        }
        else
        {
            forward();
            left -= term();
        }
    }
    return left;
}

int comparison_op(int left, TypeToken op, int right)
{
    switch (op)
    {
    case TOKEN_EQ:
        return left == right;
    case TOKEN_NE:
        return left != right;
    case TOKEN_LT:
        return left < right;
    case TOKEN_GT:
        return left > right;
    case TOKEN_LE:
        return left <= right;
    case TOKEN_GE:
        return left >= right;
    default:
        return 0;
    }
}

int term()
{
    int left = factor();
    while (current_token.type == TOKEN_MUL || current_token.type == TOKEN_DIV)
    {
        if (current_token.type == TOKEN_MUL)
        {
            forward();
            left *= factor();
        }
        else
        {
            forward();
            int divisor = factor();
            if (divisor == 0)
            {
                printf("Error: division by Zero\n");
                exit(1);
            }
            left /= divisor;
        }
    }
    return left;
}

int factor()
{ // Operator Prefix ++ --
    if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
    {
        TypeToken op = current_token.type;
        forward();
        if (current_token.type != TOKEN_ID)
        {
            printf("Error, se esperaba indetificador ++ o --\n");
            exit(1);
        }

        char name[64];
        strcpy(name, current_token.name);
        int old = getIntVar(name);
        int new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
        assignIntVar(name, new_val);
        forward();
        return new_val;
    }

    // Number
    if (current_token.type == TOKEN_NUM)
    {
        int val = current_token.value;
        forward();
        return val;
    }
    else if (current_token.type == TOKEN_ID)
    {
        char name[64];
        strcpy(name, current_token.name);
        forward();
        if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
        {
            TypeToken op = current_token.type;
            forward();
            int old = getIntVar(name);
            int new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
            assignIntVar(name, new_val);
            return old;
        }

        return getIntVar(name);
    }
    else if (current_token.type == TOKEN_LEFTPARENT)
    {
        forward();
        int val = expression();
        consume(TOKEN_RIGHTPARENT, "Falta ')'");
        return val;
    }
    else
    {
        printf("Error in factor syntax: token type %d\n", current_token.type);
        exit(1);
    }
}

void assign_compound(char *name, TypeToken op, int val)
{
    int current = getIntVar(name);
    int result;
    switch (op)
    {
    case TOKEN_PLUS_ASSIGN:
        result = current + val;
        break;
    case TOKEN_MINUS_ASSIGN:
        result = current - val;
        break;
    case TOKEN_MULT_ASSIGN:
        result = current * val;
        break;
    case TOKEN_DIV_ASSIGN:
        if (val == 0)
        {
            printf("Error: división por cero en asignación compuesta\n");
            exit(1);
        }
        result = current / val;
        break;

    default:
        return;
    }
    assignIntVar(name, result);
}

void assignation()
{
    char name[64];
    strcpy(name, current_token.name);

    if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
    {
        TypeToken op = current_token.type;
        forward();
        if (current_token.type != TOKEN_ID)
        {
            printf("Error: se esperaba identificador\n");
            exit(1);
        }
        strcpy(name, current_token.name);
        forward();
        int old = getIntVar(name);
        int new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
        assignIntVar(name, new_val);
        return;
    }

    if (current_token.type == TOKEN_ID)
    {
        strcpy(name, current_token.name);
        forward(); // consumir ID
        if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
        {
            TypeToken op = current_token.type;
            forward();
            int old = getIntVar(name);
            int new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
            assignIntVar(name, new_val);
            return;
        }
    }

    if (current_token.type == TOKEN_PLUS_ASSIGN ||
        current_token.type == TOKEN_MINUS_ASSIGN ||
        current_token.type == TOKEN_DIV_ASSIGN ||
        current_token.type == TOKEN_MULT_ASSIGN)
    {
        TypeToken op = current_token.type;
        forward();
        int val = expression();
        assign_compound(name, op, val);
        return;
    }

    consume(TOKEN_ASIGN, "not found '='");

    if (current_token.type == TOKEN_STRING)
    {
        assignStringVar(name, current_token.name);
        forward();
    }
    else
    {
        int val = expression();
        assignIntVar(name, val);
    }
}

void concat_element(char *buffer, size_t buffsize)
{
    char temp[64];

    if (current_token.type == TOKEN_STRING)
    {
        strncat(buffer, current_token.name, buffsize - strlen(buffer) - 1);
        forward();
    }
    else if (current_token.type == TOKEN_ID)
    {
        char var_name[64];
        strcpy(var_name, current_token.name);
        TypeToken next = peek_next_token_type();
        if (next == TOKEN_INC || next == TOKEN_DEC)
        {
            int val = factor();
            snprintf(temp, sizeof(temp), "%d", val);
            strncat(buffer, temp, buffsize - strlen(buffer) - 1);
        }
        else
        {
            forward();
            int found = 0;
            for (int i = 0; i < num_vars; i++)
            {
                if (strcmp(vars_table[i].name, var_name) == 0)
                {
                    if (vars_table[i].type == VAR_STRING)
                    {
                        strncat(buffer, vars_table[i].value.str_val, buffsize - strlen(buffer) - 1);
                    }
                    else
                    {
                        snprintf(temp, sizeof(temp), "%d", vars_table[i].value.int_val);
                        strncat(buffer, temp, buffsize - strlen(buffer) - 1);
                    }
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                printf("Error: variable '%s' no definida\n", var_name);
                exit(1);
            }
        }
    }
    else
    {
        int val = factor();
        snprintf(temp, sizeof(temp), "%d", val);
        strncat(buffer, temp, buffsize - strlen(buffer) - 1);
    }
}

void print_concat()
{
    char buffer[4096] = "";
    concat_element(buffer, sizeof(buffer));

    while (current_token.type == TOKEN_SUM)
    {
        forward();
        concat_element(buffer, sizeof(buffer));
    }

    printf("%s\n", buffer);
}

void print_stmt()
{
    consume(TOKEN_PRINT, "not found 'print'");
    consume(TOKEN_LEFTPARENT, "not found '('");
    print_concat();
    consume(TOKEN_RIGHTPARENT, "no found')'");
}

void program()
{
    while (current_token.type != TOKEN_EOF)
    {
        if (current_token.type == TOKEN_PRINT)
        {
            print_stmt();
        }
        else if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
        {
            expression();
        }
        else if (current_token.type == TOKEN_ID)
        {
            assignation();
        }
        else if (current_token.type == TOKEN_IF)
        {
            if_stmt();
        }
       
        else
        {
            printf("Error, invalid statement (token %d)\n", current_token.type);
            exit(1);
        }
    }
}