    #include "parser.h"
    #include "lexer.h"
    #include "variables.h"
    #include "native.h"
    #include "methods.h"
    #include "error.h"

    Token current_token;

    void forward() { current_token = nextToken(); }
    void consume(TokenType type, char *message)
    {
        if (current_token.type == type)
            forward();
        else
        {
            syntax_error(message,current_token);   
            exit(1);
        }
    }

    // Prototypes
    Value logical_or(void);
    Value logical_and(void);
    Value comparison_op(Value left, TokenType op, Value right,int line);
    Value parse_postfix(Value base);
    Value get_property(Value object, char *property, int line);

    // Lists
    Value define_list()
    {
        consume(TOKEN_LEFTBRACKET, "Expected '['");

        List *list = malloc(sizeof(List));

        list->count = 0;
        list->capacity = 4;
        list->items = malloc(sizeof(Value) * list->capacity);

        int bracket = 1;
        while (bracket != 0)
        {

            Value val = expression();
            list_push(list, val);
            if (current_token.type == TOKEN_LEFTBRACKET) bracket ++;
            if (current_token.type == TOKEN_RIGHTBRACKET) bracket --;
            if (current_token.type == TOKEN_COMMA)
            {
                consume(TOKEN_COMMA, "Expected ','");
            }
        }
        consume(TOKEN_RIGHTBRACKET, "Expected ']'");
        Value v = {0};
        v.type = VAR_LIST;
        v.value.list = list;
        return v;
    }

    //Dictionaries
    Value define_dict(){
        consume(TOKEN_LEFTBRACE, "Expected '{'");
        int braces = 1;
        Dict *dict = malloc(sizeof(Dict));
        dict->capacity = 4;
        dict->count = 0;
        dict->entries = malloc(dict->capacity * sizeof(DictEntry));
        while (braces != 0)
        {
            char key[64];
            strcpy(key, current_token.name);
            consume(TOKEN_STRING, "Invalid key, Expected String");
            consume(TOKEN_COLON, "Expect ':'");
            Value val = expression();
            dict_set(dict, key, &val);
            if (current_token.type == TOKEN_LEFTBRACE)
                braces++;
            if (current_token.type == TOKEN_RIGHTBRACE)
                braces--;
            if (current_token.type == TOKEN_COMMA) consume(TOKEN_COMMA, "Expect ','");
         
        }

        Value v = {0};
        v.type = VAR_DICT;
        v.value.dict = dict;
        forward();
        return v;    
    }


    int is_truthy(Value v) {
        if (v.type == VAR_NUMBER) return v.value.num != 0;
        if (v.type == VAR_STRING) return v.value.str[0] != '\0';
        return 0; // Null values
    }

    Value expression(){
        return logical_or();
    }

    Value logical_or(void) {
        Value left = logical_and();
        while (current_token.type == TOKEN_OR) {
            forward();
            Value right = logical_and();
            if (is_truthy(left)) {
                Value v = {0}; v.type = VAR_NUMBER; v.value.num = 1; return v;
            } else {
                int truthy = is_truthy(right);
                Value v = {0}; v.type = VAR_NUMBER; v.value.num = truthy ? 1 : 0;
                return v;
            }
        }
        return left;
    }

    Value logical_and(void) {
        Value left = comparison_expr(); 
        while (current_token.type == TOKEN_AND) {
            forward();
            Value right = comparison_expr();
            if (!is_truthy(left)) {
                
                Value v = {0}; v.type = VAR_NUMBER; v.value.num = 0; return v;
            } else {
                int truthy = is_truthy(right);
                Value v = {0}; v.type = VAR_NUMBER; v.value.num = truthy ? 1 : 0;
                return v;
            }
        }
        return left;
    }

    void define_new_function()
    {

        consume(TOKEN_FUNC, "expected func");
        Token t = current_token;
        char name[64];
        consume(TOKEN_ID, "expected indentificator ");
        strcpy(name, t.name);
        char *params[100];
        int param_count = 0;
        consume(TOKEN_PARENTLEFT, "expected (");
        while (current_token.type != TOKEN_PARENTRIGHT)
        {
            if (current_token.type == TOKEN_ID)
            {
                params[param_count] = strdup(current_token.name);
                param_count++;
            }
            forward();
            }
            consume(TOKEN_PARENTRIGHT, "expected indentificator )");
            int start = indx;
            consume(TOKEN_LEFTBRACE, "expected indentificator {");

            int braces = 1;
            while (braces > 0)
            {
                if (current_token.type == TOKEN_LEFTBRACE)
                    braces++;
                else if (current_token.type == TOKEN_RIGHTBRACE)
                    braces--;
                forward();
        }
        
        define_function(name, start, params,param_count);
    }

    Value if_stmt()
    {
        consume(TOKEN_IF, "if error");
        Value val_ret;
        val_ret.type = VAR_NULL;
        Value cond_val = expression();
        int cond = is_truthy(cond_val);

        
        consume(TOKEN_LEFTBRACE, "{ error");

        int executed = 0;

        if (cond != 0)
        {
            // Execute if block
        return block();
            executed = 1;
        }
        else
        {
            skip_block();
        }

        while (current_token.type == TOKEN_ELIF)
        {
            forward();
            Value elif_cond_val = expression();
            int elif_cond = is_truthy(elif_cond_val);
            consume(TOKEN_LEFTBRACE, "{ error");
            if (!executed && elif_cond != 0)
            {
            return  block();
                executed = 1;
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
            return block();
            }
            else
            {
                skip_block();
            }
        }
        return val_ret;
    }

    void while_stmt(void)
    {
        int after_cond = indx;
        int after_while = -1;
        forward(); // while
        Value cond_val = expression();
        consume(TOKEN_LEFTBRACE, "Error {");
        int braces = 1;
        while (braces > 0)
        {
        if (current_token.type == TOKEN_LEFTBRACE)
            braces++;
        else if (current_token.type == TOKEN_RIGHTBRACE)
            braces--;
        forward();
        }
        after_while = indx;

        while (1)
        {
            indx = after_cond;
            forward();
            cond_val = expression();
            int cond = is_truthy(cond_val);

            consume(TOKEN_LEFTBRACE, "Error {");
            if (cond != 0)
            {
            block();
            }
            else
            {
                skip_block();
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

    Value block()
    {
        Token token = current_token;
        Value ret_val;
        ret_val.type = VAR_NULL;
        int braces = 1;
        while (braces != 0 && current_token.type != TOKEN_EOF)
        {
            if (current_token.type == TOKEN_ID)
                assignation();
            else if (current_token.type == TOKEN_IF)
            {
                ret_val = if_stmt();
               if (ret_val.type != VAR_NULL)
                return ret_val;
            }
            else if (current_token.type == TOKEN_WHILE)
                while_stmt();
            else if (current_token.type == TOKEN_RETURN){
                forward();
                ret_val = expression();
                forward(); // consume  '}' 
                return ret_val;
            }
            else if (current_token.type == TOKEN_LEFTBRACE)
                        braces++;
            else if (current_token.type == TOKEN_RIGHTBRACE)
                braces--;

            else
            {
                syntax_error("Invalid sentence\n", token);
                exit(1);
            }
        }

        consume(TOKEN_RIGHTBRACE, "error }");
        
        return ret_val;
    }

    Value comparison_expr()
    {
        Value left = arith_expr();
        while (current_token.type == TOKEN_EQ || current_token.type == TOKEN_NE ||
            current_token.type == TOKEN_LT || current_token.type == TOKEN_GT ||
            current_token.type == TOKEN_LE || current_token.type == TOKEN_GE)
        {
            TokenType op = current_token.type;
            int current_line = current_token.line;
            forward();
            Value right = arith_expr();
            left = comparison_op(left, op, right,current_line);
        }
        
        return left;
    }

    Value arith_expr()
    {
        Value left = term();
        while (current_token.type == TOKEN_SUM || current_token.type == TOKEN_REST)
        {



            Token token = current_token;
            TokenType op = current_token.type;
            forward();
            Value right = term();

            if (left.type == VAR_NULL || right.type == VAR_NULL){
                
                syntax_error("Operation with null",token);
                exit(1);
            }

                if (left.type == VAR_NUMBER && right.type == VAR_NUMBER)
                {
                    if (op == TOKEN_SUM)
                        left.value.num += right.value.num;
                    else
                        left.value.num -= right.value.num;
                }
                else if (left.type == VAR_STRING && right.type == VAR_STRING && op == TOKEN_SUM)
                {
                    char tmp[512];
                    snprintf(tmp, sizeof(tmp), "%s%s", left.value.str, right.value.str);
                    left.value.str = strdup(tmp);
                }
                else if (left.type == VAR_STRING && right.type == VAR_NUMBER && op == TOKEN_SUM)
                {
                    char tmp[512];
                    snprintf(tmp, sizeof(tmp), "%s%g", left.value.str, right.value.num);
                    left.value.str =  strdup( tmp);
                }
                else if (left.type == VAR_NUMBER && right.type == VAR_STRING && op == TOKEN_SUM)
                {
                    char tmp[512];
                    snprintf(tmp, sizeof(tmp), "%g%s", left.value.num, right.value.str);
                    left.value.str = strdup(tmp);
                    left.type = VAR_STRING;
                }
                else
                {
                    syntax_error("Error bad Arith", token);
                }
        }
        return left;
    }

    Value comparison_op(Value left, TokenType op, Value right,int line)
    {
        int result = 0;
        if (left.type == VAR_NUMBER && right.type == VAR_NUMBER)
        {
            double l = left.value.num, r = right.value.num;
            switch (op)
            {
            case TOKEN_EQ:
                result = (l == r);
                break;
            case TOKEN_NE:
                result = (l != r);
                break;
            case TOKEN_LT:
                result = (l < r);
                break;
            case TOKEN_GT:
                result = (l > r);
                break;
            case TOKEN_LE:
                result = (l <= r);
                break;
            case TOKEN_GE:
                result = (l >= r);
                break;
            default:
            {

                syntax_error_line("Wrong operation",line);
            }
            }
        }
        else if (left.type == VAR_STRING && right.type == VAR_STRING)
        {
            int cmp = strcmp(left.value.str, right.value.str);
            switch (op)
            {
            case TOKEN_EQ:
                result = (cmp == 0);
                break;
            case TOKEN_NE:
                result = (cmp != 0);
                break;
            case TOKEN_LT:
                result = (cmp < 0);
                break;
            case TOKEN_GT:
                result = (cmp > 0);
                break;
            case TOKEN_LE:
                result = (cmp <= 0);
                break;
            case TOKEN_GE:
                result = (cmp >= 0);
                break;
            default:
            {

                syntax_error_line("Wrong operation",line);
            }
            }
        }
        else if (left.type == VAR_NULL && right.type == VAR_NULL){
            switch (op)
            {
            case TOKEN_EQ:
                result = 1;
                break;
            case TOKEN_NE:
                result = 0;
                break;
            default:
                syntax_error_line("Wrong operation with null", line);
            }
        }
        else if (left.type == VAR_NULL || right.type == VAR_NULL)
        {
            switch (op)
            {
            case TOKEN_EQ:
                result = 0;
                break;
            case TOKEN_NE:
                result = 1;
                break;
            default:
                syntax_error_line("Wrong operation with null", line);
            }
        }
        else
        {
            switch (op)
            {
            case TOKEN_EQ:
                result = 0;
                break;
            case TOKEN_NE:
                result = 1;
                break;
            default:
            {

                syntax_error_line("Wrong operation", line);
            }
            }
        }
        Value v = {0};
        v.type = VAR_NUMBER;
        v.value.num = result;
        return v;
    }

    Value parse_postfix(Value base){
        while (1){
            if (current_token.type == TOKEN_LEFTBRACKET) // Get list value
            {
                consume(TOKEN_LEFTBRACKET, "Expected '['");

                Value idx = expression();

                consume(TOKEN_RIGHTBRACKET, "Expected ']'");

                if (base.type != VAR_LIST)
                {
                    runtime_error("Value is not a list");
                }

                if (idx.type != VAR_NUMBER)
                {
                    runtime_error("List index must be number");
                }

                base = list_get(base.value.list, (int)idx.value.num);

                continue;
            }

            if (current_token.type == TOKEN_DOT){
                consume(TOKEN_DOT, "Expected '.'");

                char member[64];

                strcpy(member,current_token.name);

                consume(TOKEN_ID, "Expected identifier");
                // Method call
                if (current_token.type == TOKEN_PARENTLEFT)
                {
                    consume(TOKEN_PARENTLEFT, "Expected '('");

                    Value args[100];

                    int arg_count = 0;

                    while (current_token.type != TOKEN_PARENTRIGHT)
                    {
                        args[arg_count++] = expression();
                        if (current_token.type == TOKEN_COMMA)
                        {
                            consume(TOKEN_COMMA,"Expected ','");
                        }
                    }

                    consume(TOKEN_PARENTRIGHT, "Expected ')'");

                    return call_method(base, member, args, arg_count, current_token.line);
                }
                else{
                    base = get_property(base,member,current_token.line);
                }
                continue;
            }

            if (current_token.type == TOKEN_PARENTLEFT)
            {
                consume(TOKEN_PARENTLEFT, "Expected '('");

                Value args[100];

                int arg_count = 0;

                while (current_token.type != TOKEN_PARENTRIGHT)
                {
                    args[arg_count++] = expression();
                    if (current_token.type == TOKEN_COMMA)
                    {
                        consume(TOKEN_COMMA,"Expected ','");
                    }
                }
                int last_index = indx;
                consume(TOKEN_PARENTRIGHT,"Expected ')'");

                if (base.type == VAR_NATIVE)
                {
                    return base.value.native_func(args, arg_count, current_token.line);
                }
                else if (base.type == VAR_FUNCTION)
                {
                    return function_call(base, args, arg_count, last_index);
                }
                else
                {
                    syntax_error("This value is not callable", current_token);
                }
                continue;
            }
            break;
        }
        return base;
    }

    Value get_property(Value object, char *property, int line){
        if (object.type == VAR_DICT){
            return dict_get(object.value.dict, property);
        }

        if (object.type == VAR_STRING){
            if (strcmp(property, "length") == 0){
                Value v = {0};

                v.type = VAR_NUMBER;

                v.value.num = strlen(object.value.str);
                return v;
            }

            char message[64];
            sprintf(message,"Property '%s' not found",property);
            syntax_error_line(message,line);
        }

        Value null_value = {0};
        null_value.type = VAR_NULL;
        return null_value;
    }

    Value term()
    {
        Value left = factor();
        while (current_token.type == TOKEN_MUL || current_token.type == TOKEN_DIV)
        {
            Token token = current_token;
            TokenType op = current_token.type;
            forward();
            Value right = factor();

            if (left.type == VAR_NULL || right.type == VAR_NULL)
            {
                syntax_error("Operation with null", token);
                exit(1);
            }

            if (left.type == VAR_NUMBER && right.type == VAR_NUMBER)
            {
                if (op == TOKEN_MUL)
                    left.value.num *= right.value.num;
                else
                {
                    double divisor = right.value.num;
                    if (divisor == 0)
                    {
                        syntax_error("Division by zero", token);
                        exit(1);
                    }
                    left.value.num /= right.value.num;
                }
            }
        }
        return left;
    }


        Value factor()
        { // Operator Prefix ++ --
            Value v = {0};
            v.type = VAR_NUMBER;

            if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
            {
                Token token = current_token;
                TokenType op = current_token.type;
                forward();
                if (current_token.type != TOKEN_ID)
                {
                    syntax_error("Syntax Error", token);
                    exit(1);
                }

                char name[256];
                strcpy(name, current_token.name);
                Value old = var_value_get(name);
                if (old.type != VAR_NUMBER)
                {
                    syntax_error("Bad operation", token);
                }
                double new_val = (op == TOKEN_INC) ? old.value.num + 1 : old.value.num - 1;
                assign_number_val(name, new_val);
                forward();
                v.type = VAR_NUMBER;
                v.value.num = new_val;
                return v;
            }
            else if (current_token.type == TOKEN_NOT)
            {
                forward();
                Value operand = factor(); 
                int truthy = is_truthy(operand);
                Value v = {0};
                v.type = VAR_NUMBER;
                v.value.num = truthy ? 0 : 1;
                return v;
            }
            else if (current_token.type == TOKEN_LEFTBRACKET){
                return define_list();

            }

            else if (current_token.type == TOKEN_TRUE)
            // Boolean
            {
                forward();
                v.value.num = 1;
                return v;
            }else if (current_token.type == TOKEN_FALSE)
            {
                forward();
                v.value.num = 0;
                return v;
            }
            // Negative number
            else if (current_token.type == TOKEN_REST)
            {
                int current_line = current_token.line;
                forward();
                if (current_token.type == TOKEN_ID)
                {
                    char name[256];
                    strcpy(name, current_token.name);
                    v = var_value_get(name);
                    if (v.type != VAR_NUMBER)
                    {
                        char message[256];
                        printf("type %d\n", current_token.type);
                        sprintf(message, "Variable %s is not a number", current_token.name);
                        syntax_error_line(message, current_line);
                    }
                    v.value.num = -v.value.num;
                }
                else if (current_token.type == TOKEN_NUM)
                {
                    v.value.num = -current_token.value;
                }
                else
                {
                    syntax_error_line("Can't assign negative value", current_line);
                }
                forward();
                return v;
            }
            // Positive number
            else if (current_token.type == TOKEN_NUM)
            {
                v.value.num = current_token.value;
                forward();
                return v;
            }
            //Dictionary
            else if (current_token.type == TOKEN_LEFTBRACE){
                v = define_dict();
                v.type = VAR_DICT;
                return v;
            }
            //id
            else if (current_token.type == TOKEN_ID)
            {
                char name[64];
                strcpy(name, current_token.name);
                consume(TOKEN_ID,"Expected identifier");
                Value base = var_value_get(name);

                return parse_postfix(base);

            }
            else if (current_token.type == TOKEN_PARENTLEFT)
            {
                forward();
                v = expression();
                consume(TOKEN_PARENTRIGHT, "expected )");
                return v;
            }
            else if (current_token.type == TOKEN_STRING)
            {

                v.type = VAR_STRING;
                v.value.str = strdup(current_token.name);
                forward();
                return v;
            }
            else if (current_token.type == TOKEN_NULL)
            {
                v.type = VAR_NULL;
                forward();
                return v;
            }
            else
            {
                syntax_error("Syntax Error", current_token);
                exit(1);
            }
        }

        void assign_compound(char *name, TokenType op, double val, int op_line)
        {
            Value v = var_value_get(name);
            double current = v.value.num;
            double result;
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
                    syntax_error_line("Division by zero", op_line);
                    exit(1);
            }
            result = current / val;
            break;

        default:
            return;
        }
        assign_number_val(name, result);
    }

    void assignation()
    {
        char name[64];
        strcpy(name, current_token.name);


        if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
        {
            Token token = current_token;
            TokenType op = current_token.type;

            forward();
            if (current_token.type != TOKEN_ID)
            {
                syntax_error("",token);
                exit(1);
            }
            strcpy(name, current_token.name);
            forward();
            Value v = var_value_get(name);
            double old = v.value.num;
            double new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
            assign_number_val(name, new_val);
            return;
        }

        if (current_token.type == TOKEN_ID)
        {
            strcpy(name, current_token.name);
            if (peek_next_token_type() == TOKEN_PARENTLEFT){ //is a function
                factor();
                return;
            }

            if (peek_next_token_type() == TOKEN_DOT) // is a dictionary
            {
                forward();
                Value base = var_value_get(name);

            
                char member[64];
                strcpy(member, current_token.name);
                Value object = parse_postfix(base);

                if (object.type == VAR_DICT){
                Dict *dict = object.value.dict;
                
                consume(TOKEN_ASIGN, "Expected '=");
                Value val = expression();
                dict_set(dict, member, &val);

                }
                return;
            }

            forward(); //Consume ID
            
            if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
            {
                TokenType op = current_token.type;
                forward();
                Value v = var_value_get(name);
                double old = v.value.num;
                double new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
                assign_number_val(name, new_val);
                return;
            }

        if (current_token.type == TOKEN_PLUS_ASSIGN ||
            current_token.type == TOKEN_MINUS_ASSIGN ||
            current_token.type == TOKEN_DIV_ASSIGN ||
            current_token.type == TOKEN_MULT_ASSIGN)
        {
            TokenType op = current_token.type;
            double current_line = current_token.line;
            forward();
            Value val = expression();
            assign_compound(name, op, val.value.num,current_line);
            return;
        }

        consume(TOKEN_ASIGN, "not found '='");


        Value val = expression();
        variable_set(name,val);
        }
    }

    Value function_call(Value func_val, Value *args_v, int count, int last_index)
    {
        push_scope();
            if (count < func_val.value.func.param_count)
            {
                syntax_error_line("Few params in function",current_token.line);
            }
            else if (count > func_val.value.func.param_count)
            {
                syntax_error_line("Too much params in function", current_token.line);
            }

            for (int x = 0; x < count; x++)
            {
                variable_set(func_val.value.func.param[x], args_v[x]);
            }
            indx = func_val.value.func.start;
            forward(); // Update position on first token in function
            Value ret_val = block();
            indx = last_index;
            forward();
            pop_scope();
            return ret_val;
    }

    void dict_print(Dict *dict){
        printf("{ ");
        for (int e = 0; e < dict->count; e++)
        {
            printf("%s :", dict->entries[e].key);
            switch (dict->entries[e].value->type)
            {
            case VAR_STRING:
                printf( "%s", dict->entries[e].value->value.str);
                break;
            case VAR_NULL:
                printf("%s", "null");
                break;
            case VAR_DICT:
                dict_print(dict->entries[e].value->value.dict);
                break;

            default:
                printf("%g", dict->entries[e].value->value.num);
                break;
            }
            if (e < dict->count - 1)
            {
                printf(" ,");
            }
        }
        printf(" }");
    }

    void list_print(List *list)
    {
        printf("[");
        for (int e = 0; e < list->count; e++)
        {
            switch (list->items[e].type)
            {
            case VAR_STRING:
                printf("%s", list->items[e].value.str);
                break;
            case VAR_NULL:
                printf("%s", "null");
                break;
            case VAR_DICT:
                dict_print(list->items[e].value.dict);
                break;
            case VAR_LIST:
                list_print(list->items[e].value.list);
                break;

            default:
                printf("%g", list->items[e].value.num);
                break;
            }
            if (e < list->count - 1)
            {
                printf(", ");
            }
        }
        printf("]");
    }

    void program()
    {
        while (current_token.type != TOKEN_EOF)
        {
            if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
            {
                expression();
            }
            else if (current_token.type == TOKEN_ID)
            {
                assignation();
            }
            else if (current_token.type == TOKEN_FUNC)
            {
                define_new_function();
            }
            else if (current_token.type == TOKEN_IF)
            {
                if_stmt();
            }
            else if (current_token.type == TOKEN_WHILE)
            {
                while_stmt();
            }
            else
            {
                syntax_error("invalid statement", current_token);
                exit(1);
            }
        }
    }