    #include "parser.h"
    #include "lexer.h"
    #include "variables.h"
    #include "native.h"
    #include "methods.h"
    #include "error.h"
    #include <ctype.h>
    #include <math.h>

    Token current_token;

    void forward() { current_token = nextToken(); }
    void consume(TokenType type, char *message)
    {
        if (current_token.type == type)
            forward();
        else
        {
            syntax_error(message,current_token);
        }
    }

    // Prototypes
    Value logical_or(void);
    Value logical_and(void);
    Value comparison_op(Value left, TokenType op, Value right,int line);
    Value parse_postfix(Value base);
    Value get_property(Value object, char *property, int line);
    static Value define_list(void);
    static int is_truthy(Value v);
    void while_stmt(void);
    void for_stmt(void);
    void switch_stmt(void);
    void try_stmt(void);
    void throw_stmt(void);
    void class_stmt(int exported);
    static void assign_compound(char *name, TokenType op, double val, int op_line);
    void handle_typed_decl(int is_const_flag);

    static int utf8_length(const char *str)
    {
        int count = 0;
        const unsigned char *p = (const unsigned char *)str;
        while (*p)
        {
            if ((*p & 0xC0) != 0x80)
                count++;
            p++;
        }
        return count;
    }

    // Lists
    Value define_list()
    {
        consume(TOKEN_LEFTBRACKET, "Expected '['");

        List *list = malloc(sizeof(List));

        list->count = 0;
        list->capacity = 4;
        list->items = malloc(sizeof(Value) * (size_t)list->capacity);

        if (current_token.type == TOKEN_RIGHTBRACKET)
        {
            consume(TOKEN_RIGHTBRACKET, "Expected ']'");
            Value v = {0};
            v.type = VAR_LIST;
            v.value.list = list;
            return v;
        }

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
        Dict *dict = malloc(sizeof(Dict));
        dict->capacity = 4;
        dict->count = 0;
        dict->entries = malloc((size_t)dict->capacity * sizeof(DictEntry));

        if (current_token.type == TOKEN_RIGHTBRACE)
        {
            forward(); // Consume '}'
            Value v = {0};
            v.type = VAR_DICT;
            v.value.dict = dict;
            return v;
        }

        int braces = 1;
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
        if (v.type == VAR_NUMBER) return fabs(v.value.num) > 1e-10;
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

    void define_new_function_ex(int exported)
    {
        int had_void = 0;
        if (current_token.type == TOKEN_VOID)
        {
            had_void = 1;
            forward();
        }
        if (current_token.type == TOKEN_FUNC || current_token.type == TOKEN_DEF)
            forward();
        else if (!had_void)
            syntax_error("expected func, def, or void func", current_token);
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
                if (current_token.type == TOKEN_EOF)
                    syntax_error("Unclosed function body", current_token);
                if (current_token.type == TOKEN_LEFTBRACE)
                    braces++;
                else if (current_token.type == TOKEN_RIGHTBRACE)
                    braces--;
                forward();
        }

        define_function(name, start, params, param_count, exported, NULL);
    }

    void define_new_function()
    {
        define_new_function_ex(0);
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
            val_ret = block();
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
                val_ret = block();
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
                val_ret = block();
                executed = 1;
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
        if (current_token.type == TOKEN_EOF)
            syntax_error("Unclosed while body", current_token);
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

    static int value_equals(Value left, Value right)
    {
        if (left.type != right.type)
            return 0;
        if (left.type == VAR_NUMBER)
            return fabs(left.value.num - right.value.num) < 1e-10;
        if (left.type == VAR_STRING)
            return strcmp(left.value.str, right.value.str) == 0;
        if (left.type == VAR_NULL)
            return 1;
        return 0;
    }

    static void skip_to_leftbrace()
    {
        while (current_token.type != TOKEN_LEFTBRACE && current_token.type != TOKEN_EOF)
            forward();
    }

    void for_stmt(void)
    {
        forward();

        if (current_token.type != TOKEN_SEMICOLON)
            assignation();
        if (current_token.type != TOKEN_SEMICOLON)
            syntax_error("expected ';' after for init", current_token);

        int cond_start = indx;
        forward();
        Value cond_val = expression();
        if (current_token.type != TOKEN_SEMICOLON)
            syntax_error("expected ';' after for condition", current_token);

        int inc_start = indx;
        forward();
        skip_to_leftbrace();
        int body_start = indx;
        forward();

        int braces = 1;
        while (braces > 0)
        {
            if (current_token.type == TOKEN_EOF)
                syntax_error("Unclosed for body", current_token);
            if (current_token.type == TOKEN_LEFTBRACE)
                braces++;
            else if (current_token.type == TOKEN_RIGHTBRACE)
                braces--;
            forward();
        }
        int after_for = indx;
        Token after_for_token = current_token;

        while (1)
        {
            indx = cond_start;
            forward();
            cond_val = expression();
            if (!is_truthy(cond_val))
                break;

            indx = body_start;
            forward();
            block();

            indx = inc_start;
            forward();
            assignation();
        }

        indx = after_for;
        current_token = after_for_token;
    }

    void switch_stmt(void)
    {
        forward();
        Value target = expression();
        consume(TOKEN_LEFTBRACE, "expected '{'");

        int matched = 0;
        while (current_token.type != TOKEN_RIGHTBRACE && current_token.type != TOKEN_EOF)
        {
            int should_run = 0;

            if (current_token.type == TOKEN_CASE)
            {
                forward();
                Value case_val = expression();
                should_run = !matched && value_equals(target, case_val);
                consume(TOKEN_COLON, "expected ':'");
            }
            else if (current_token.type == TOKEN_DEFAULT)
            {
                forward();
                should_run = !matched;
                consume(TOKEN_COLON, "expected ':'");
            }
            else
            {
                syntax_error("expected case or default", current_token);
            }

            consume(TOKEN_LEFTBRACE, "expected '{'");
            if (should_run)
            {
                block();
                matched = 1;
            }
            else
            {
                skip_block();
            }
        }

        consume(TOKEN_RIGHTBRACE, "expected '}'");
    }

    void class_stmt(int exported)
    {
        consume(TOKEN_CLASS, "expected class");
        char class_name[64];
        strcpy(class_name, current_token.name);
        consume(TOKEN_ID, "expected class name");

        push_scope();
        consume(TOKEN_LEFTBRACE, "expected '{'");
        while (current_token.type != TOKEN_RIGHTBRACE)
        {
            if (current_token.type == TOKEN_PUBLIC || current_token.type == TOKEN_PRIVATE)
                forward();

            if (current_token.type == TOKEN_VOID ||
                current_token.type == TOKEN_FUNC ||
                current_token.type == TOKEN_DEF)
                define_new_function_ex(0);
            else
                syntax_error("expected method in class", current_token);
        }
        consume(TOKEN_RIGHTBRACE, "expected '}'");

        Dict *dict = malloc(sizeof(Dict));
        dict->capacity = scope_stack[scope_depth].count + 4;
        dict->count = 0;
        dict->entries = malloc((size_t)dict->capacity * sizeof(DictEntry));

        Scope *sc = &scope_stack[scope_depth];
        for (int i = 0; i < sc->count; i++)
            dict_set(dict, sc->vars[i]->name, sc->vars[i]);

        pop_scope();

        Value val = {0};
        val.type = VAR_DICT;
        val.value.dict = dict;
        variable_set(class_name, val);
        free_value_internals(&val);

        if (exported)
        {
            Scope *out = &scope_stack[scope_depth];
            for (int i = 0; i < out->count; i++)
                if (strcmp(out->vars[i]->name, class_name) == 0)
                    out->vars[i]->exported = 1;
        }
    }

    void skip_block()
    {
        int brace_count = 1;
        while (brace_count > 0)
        {
            if (current_token.type == TOKEN_EOF)
                syntax_error("Unclosed block", current_token);
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

    static void value_to_message(Value value, char *out, size_t out_size)
    {
        switch (value.type)
        {
        case VAR_STRING:
            snprintf(out, out_size, "%s", value.value.str ? value.value.str : "");
            break;
        case VAR_NUMBER:
            snprintf(out, out_size, "%g", value.value.num);
            break;
        case VAR_NULL:
            snprintf(out, out_size, "null");
            break;
        case VAR_LIST:
            snprintf(out, out_size, "[list]");
            break;
        case VAR_DICT:
        case VAR_MODULE:
            snprintf(out, out_size, "{object}");
            break;
        default:
            snprintf(out, out_size, "error");
            break;
        }
    }

    void throw_stmt(void)
    {
        consume(TOKEN_THROW, "expected 'throw'");
        Value message = expression();
        char text[512];
        value_to_message(message, text, sizeof(text));
        chipojo_throw(text);
    }

    static void scan_after_current_block(Token *after_token, int *after_index, int *after_line)
    {
        int saved_index = indx;
        int saved_line = g_line;
        Token saved_token = current_token;

        int brace_count = 1;
        while (brace_count > 0)
        {
            if (current_token.type == TOKEN_EOF)
                syntax_error("Unclosed block", current_token);
            if (current_token.type == TOKEN_LEFTBRACE)
                brace_count++;
            else if (current_token.type == TOKEN_RIGHTBRACE)
                brace_count--;
            forward();
        }

        *after_token = current_token;
        *after_index = indx;
        *after_line = g_line;

        indx = saved_index;
        g_line = saved_line;
        current_token = saved_token;
    }

    void try_stmt(void)
    {
        consume(TOKEN_TRY, "expected 'try'");
        consume(TOKEN_LEFTBRACE, "expected '{'");

        Token after_try_token;
        int after_try_index;
        int after_try_line;
        int saved_scope = scope_depth;
        char *saved_input = input;
        scan_after_current_block(&after_try_token, &after_try_index, &after_try_line);

        jmp_buf try_env;
        int failed = 0;
        error_try_push(&try_env);
        if (setjmp(try_env) == 0)
        {
            block();
        }
        else
        {
            failed = 1;
            input = saved_input;
            while (scope_depth > saved_scope)
                pop_scope();
        }
        error_try_pop();

        if (failed)
        {
            current_token = after_try_token;
            indx = after_try_index;
            g_line = after_try_line;
        }

        consume(TOKEN_CATCH, "expected 'catch'");

        char catch_name[64] = "error";
        if (current_token.type == TOKEN_PARENTLEFT)
        {
            forward();
            if (current_token.type == TOKEN_ID)
            {
                strcpy(catch_name, current_token.name);
                forward();
            }
            consume(TOKEN_PARENTRIGHT, "expected ')'");
        }
        else if (current_token.type == TOKEN_ID)
        {
            strcpy(catch_name, current_token.name);
            forward();
        }

        consume(TOKEN_LEFTBRACE, "expected '{'");
        if (failed)
        {
            Value err = {0};
            err.type = VAR_STRING;
            err.value.str = (char *)error_last_message();
            variable_set(catch_name, err);
            block();
        }
        else
        {
            skip_block();
        }
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
            else if (current_token.type == TOKEN_FOR)
                for_stmt();
            else if (current_token.type == TOKEN_SWITCH)
                switch_stmt();
            else if (current_token.type == TOKEN_TRY)
                try_stmt();
            else if (current_token.type == TOKEN_THROW)
                throw_stmt();
            else if (current_token.type == TOKEN_CLASS)
                class_stmt(0);
            else if (current_token.type == TOKEN_RETURN){
                forward();
                ret_val = expression();
                forward(); // consume  '}' 
                return ret_val;
            }
            else if (current_token.type == TOKEN_FUNC || current_token.type == TOKEN_DEF || current_token.type == TOKEN_VOID)
                define_new_function_ex(0);
            else if (current_token.type == TOKEN_EXPORT)
            {
                forward();
                if (current_token.type == TOKEN_DEFAULT)
                {
                    forward();
                    Value val = expression();
                    variable_set("default", val);
                    Scope *sc = &scope_stack[scope_depth];
                    for (int i = 0; i < sc->count; i++)
                        if (strcmp(sc->vars[i]->name, "default") == 0)
                            sc->vars[i]->exported = 1;
                }
                else if (current_token.type == TOKEN_FUNC || current_token.type == TOKEN_DEF)
                    define_new_function_ex(1);
                else if (current_token.type == TOKEN_CONST)
                {
                    forward();
                    char export_name[64];
                    strcpy(export_name, current_token.name);
                    handle_typed_decl(1);
                    Scope *sc = &scope_stack[scope_depth];
                    for (int i = 0; i < sc->count; i++)
                        if (strcmp(sc->vars[i]->name, export_name) == 0)
                        {
                            sc->vars[i]->exported = 1;
                            sc->vars[i]->is_const = 1;
                        }
                }
                else if (current_token.type == TOKEN_ID)
                {
                    char export_name[64];
                    strcpy(export_name, current_token.name);
                    assignation();
                    Scope *sc = &scope_stack[scope_depth];
                    for (int i = 0; i < sc->count; i++)
                        if (strcmp(sc->vars[i]->name, export_name) == 0)
                            sc->vars[i]->exported = 1;
                }
                else
                    syntax_error("expected 'default', 'func', or variable after 'export'", current_token);
            }
            else if (current_token.type == TOKEN_IMPORT)
                import_stmt();
            else if (current_token.type == TOKEN_FROM)
                from_stmt();
            else if (current_token.type == TOKEN_CONST)
            {
                forward();
                handle_typed_decl(1);
            }
            else if (current_token.type == TOKEN_VAR ||
                     current_token.type == TOKEN_LET)
            {
                forward();
                handle_typed_decl(0);
            }
            else if (current_token.type == TOKEN_FLOAT_TYPE ||
                     current_token.type == TOKEN_STRING_TYPE ||
                     current_token.type == TOKEN_BOOL_TYPE)
            {
                forward();
                handle_typed_decl(0);
            }
            else if (current_token.type == TOKEN_LEFTBRACE)
                        braces++;
            else if (current_token.type == TOKEN_RIGHTBRACE)
                braces--;

            else
            {
                syntax_error("Invalid sentence\n", token);
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
                result = (fabs(l - r) < 1e-10);
                break;
            case TOKEN_NE:
                result = (fabs(l - r) >= 1e-10);
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
        int used_member = 0;
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
                used_member = 1;
                consume(TOKEN_DOT, "Expected '.'");

                char member[64];

                strcpy(member,current_token.name);

                if (current_token.type != TOKEN_ID && current_token.type != TOKEN_DEFAULT)
                    consume(TOKEN_ID, "Expected identifier");
                else
                    forward();

                // For dict/module, get property first, then handle ( as function call
                if (base.type == VAR_DICT || base.type == VAR_MODULE)
                {
                    base = get_property(base, member, current_token.line);
                    // If next token is (, it will be handled by the function call code below
                    continue;
                }

                // Method call for other types (string, list)
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
        if (!used_member && base.type == VAR_MODULE)
        {
            Value imported = dict_get(base.value.dict, "default");
            if (imported.type == VAR_NULL)
                runtime_error("Module has no default export");
            return imported;
        }
        return base;
    }

    Value get_property(Value object, char *property, int line){
        if (object.type == VAR_DICT || object.type == VAR_MODULE){
            if (strcmp(property, "length") == 0)
            {
                Value v = {0};
                v.type = VAR_NUMBER;
                v.value.num = object.value.dict ? object.value.dict->count : 0;
                return v;
            }

            Value result = dict_get(object.value.dict, property);
            if (result.type == VAR_NULL)
            {
                char message[128];
                snprintf(message, sizeof(message), "Property '%s' not found", property);
                syntax_error_line(message, line);
            }
            return result;
        }

        if (object.type == VAR_LIST && strcmp(property, "length") == 0){
            Value v = {0};
            v.type = VAR_NUMBER;
            v.value.num = object.value.list ? object.value.list->count : 0;
            return v;
        }

        if (object.type == VAR_STRING){
            if (strcmp(property, "length") == 0){
                Value v = {0};

                v.type = VAR_NUMBER;

                v.value.num = (double)utf8_length(object.value.str ? object.value.str : "");
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
            }

            if (left.type == VAR_NUMBER && right.type == VAR_NUMBER)
            {
                if (op == TOKEN_MUL)
                    left.value.num *= right.value.num;
                else
                {
                    double divisor = right.value.num;
                    if (fabs(divisor) < 1e-10)
                    {
                        syntax_error("Division by zero", token);
                    }
                    left.value.num /= right.value.num;
                }
            }
        }
        return left;
    }


    static int scan_is_arrow(void)
    {
        int i = indx;
        while (input[i] && isspace((unsigned char)input[i])) i++;
        if (input[i] == ')')
        {
            i++;
            while (input[i] && isspace((unsigned char)input[i])) i++;
            return (input[i] == '=' && input[i+1] == '>');
        }
        if (isalpha((unsigned char)input[i]) || input[i] == '_')
        {
            while (input[i] && (isalnum((unsigned char)input[i]) || input[i] == '_')) i++;
            while (input[i] && isspace((unsigned char)input[i])) i++;
            while (input[i] == ',')
            {
                i++;
                while (input[i] && isspace((unsigned char)input[i])) i++;
                if (isalpha((unsigned char)input[i]) || input[i] == '_')
                {
                    while (input[i] && (isalnum((unsigned char)input[i]) || input[i] == '_')) i++;
                    while (input[i] && isspace((unsigned char)input[i])) i++;
                }
            }
            if (input[i] == ')')
            {
                i++;
                while (input[i] && isspace((unsigned char)input[i])) i++;
                return (input[i] == '=' && input[i+1] == '>');
            }
        }
        return 0;
    }

    static void skip_expression_body(void)
    {
        int depth = 0;
        while (input[indx])
        {
            char c = input[indx];
            if (c == '(' || c == '[') depth++;
            else if (c == '{') depth++;
            else if (c == ')') { if (depth == 0) break; depth--; }
            else if (c == ']') { if (depth == 0) break; depth--; }
            else if (c == '}') break;
            else if ((c == '\n' || c == ';') && depth == 0) break;
            indx++;
        }
    }

    Value parse_arrow_function(void)
    {
        char *params[100];
        int param_count = 0;
        int saved_indx = indx;
        int saved_line = g_line;

        while (input[indx] && isspace((unsigned char)input[indx])) indx++;

        if (input[indx] != ')')
        {
            indx = saved_indx;
            g_line = saved_line;
            while (1)
            {
                forward();
                if (current_token.type == TOKEN_ID)
                {
                    params[param_count] = strdup(current_token.name);
                    param_count++;
                    forward();
                }
                if (current_token.type == TOKEN_COMMA)
                    continue;
                else if (current_token.type == TOKEN_PARENTRIGHT)
                    break;
                else
                    syntax_error("Expected ')' in arrow function params", current_token);
            }
        }
        else
        {
            indx = saved_indx;
            g_line = saved_line;
            forward(); // read )
        }

        consume(TOKEN_PARENTRIGHT, "expected ')' in arrow params");

        if (current_token.type != TOKEN_ARROW)
            syntax_error("expected '=>'", current_token);

        while (input[indx] && isspace((unsigned char)input[indx])) indx++;
        int start = indx;
        forward();

        Value v = {0};
        v.name = "anonymous";
        v.type = VAR_FUNCTION;

        if (current_token.type == TOKEN_LEFTBRACE)
        {
            v.value.func.is_block = 1;
            start = indx;
            int braces = 1;
            while (braces > 0)
            {
                forward();
                if (current_token.type == TOKEN_LEFTBRACE) braces++;
                else if (current_token.type == TOKEN_RIGHTBRACE) braces--;
            }
            forward();
        }
        else
        {
            v.value.func.is_block = 0;
            skip_expression_body();
            forward();
        }
        v.value.func.start = start;
        v.value.func.param = malloc((size_t)param_count * sizeof(char *));
        for (int i = 0; i < param_count; i++)
            v.value.func.param[i] = strdup(params[i]);
        v.value.func.param_count = param_count;
        v.value.func.buffer = NULL;
        return v;
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
                Value v2 = {0};
                v2.type = VAR_NUMBER;
                v2.value.num = truthy ? 0 : 1;
                return v2;
            }
            else if (current_token.type == TOKEN_LEFTBRACKET){
                return parse_postfix(define_list());

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
                return parse_postfix(v);
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
            // keywords allowed as variable names in expressions
            else if (current_token.type == TOKEN_DEFAULT)
            {
                char name[64];
                strcpy(name, current_token.name);
                forward();
                Value base = var_value_get(name);
                return parse_postfix(base);
            }
            else if (current_token.type == TOKEN_PARENTLEFT)
            {
                if (scan_is_arrow())
                {
                    return parse_arrow_function();
                }
                forward();
                v = expression();
                consume(TOKEN_PARENTRIGHT, "expected )");
                return parse_postfix(v);
            }
            else if (current_token.type == TOKEN_STRING)
            {

                v.type = VAR_STRING;
                v.value.str = strdup(current_token.name);
                forward();
                return parse_postfix(v);
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
                if (fabs(val) < 1e-10)
                {
                    syntax_error_line("Division by zero", op_line);
            }
            result = current / val;
            break;

        default:
            return;
        }
        assign_number_val(name, result);
    }

    static void check_not_const(char *name)
    {
        for (int d = scope_depth; d >= 0; d--)
        {
            Scope *sc = &scope_stack[d];
            for (int i = 0; i < sc->count; i++)
            {
                if (strcmp(sc->vars[i]->name, name) == 0 && sc->vars[i]->is_const)
                {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "Cannot reassign constant '%s'", name);
                    syntax_error_line(msg, current_token.line);
                }
            }
        }
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
            }
            strcpy(name, current_token.name);
            forward();
            Value v = var_value_get(name);
            double old = v.value.num;
            check_not_const(name);
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
                factor();
                return;
            }

            forward(); //Consume ID
            
            if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
            {
                TokenType op = current_token.type;
                forward();
                Value v = var_value_get(name);
                check_not_const(name);
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
            int current_line = current_token.line;
            forward();
            check_not_const(name);
            Value val = expression();
            assign_compound(name, op, val.value.num,current_line);
            return;
        }

        check_not_const(name);

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

            // Push closure variables (captured module scope) before params
            // so params can shadow closure variables if needed
            if (func_val.value.func.closure)
            {
                Dict *cl = func_val.value.func.closure;
                for (int i = 0; i < cl->count; i++)
                {
                    variable_set(cl->entries[i].key, *cl->entries[i].value);
                }
            }

            for (int x = 0; x < count; x++)
            {
                variable_set(func_val.value.func.param[x], args_v[x]);
            }

            char *saved_input = NULL;
            if (func_val.value.func.buffer)
            {
                saved_input = input;
                input = func_val.value.func.buffer;
            }
            indx = func_val.value.func.start;
            forward();
            Value ret_val;
            if (func_val.value.func.is_block)
                ret_val = block();
            else
                ret_val = expression();

            if (saved_input)
            {
                input = saved_input;
            }
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
            case VAR_LIST:
                list_print(dict->entries[e].value->value.list);
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

    // Keep dot paths compatible, but prefer slash paths in new code.
    static void read_module_path(char *out, int max_len)
    {
        char tmp[512];
        int pos = 0;
        while (1)
        {
            int remaining = (int)sizeof(tmp) - pos - 1;
            strncpy(tmp + pos, current_token.name, (size_t)(remaining > 63 ? 63 : remaining));
            pos += (int)strlen(current_token.name);
            consume(current_token.type, "expected identifier");
            if (current_token.type == TOKEN_DOT)
            {
                if (pos + 1 >= (int)sizeof(tmp) - 1) break;
                tmp[pos++] = '/';
                forward(); // consume '.'
            }
            else
                break;
        }
        tmp[pos] = '\0';
        strncpy(out, tmp, (size_t)(max_len - 1));
        out[max_len - 1] = '\0';
    }

    static void read_module_name(char *out, int max_len)
    {
        if (current_token.type == TOKEN_STRING)
        {
            int j = 0;
            for (int i = 0; current_token.name[i] && j < max_len - 1; i++)
            {
                char c = current_token.name[i];
                if (c == '.' && strchr(current_token.name, '/') == NULL && current_token.name[0] != '@')
                    c = '/';
                out[j++] = c;
            }
            out[j] = '\0';
            forward();
            return;
        }

        read_module_path(out, max_len);
    }

    static void import_named_exports(Value mod_val)
    {
        if (mod_val.type != VAR_MODULE)
            runtime_error("Module error");

        consume(TOKEN_LEFTBRACE, "expected '{'");
        while (current_token.type != TOKEN_RIGHTBRACE)
        {
            char export_name[64];
            char local_name[64];

            if (current_token.type != TOKEN_ID && current_token.type != TOKEN_DEFAULT)
                consume(TOKEN_ID, "expected export name");

            strcpy(export_name, current_token.name);
            strcpy(local_name, current_token.name);
            forward();

            if (current_token.type == TOKEN_AS)
            {
                forward();
                strcpy(local_name, current_token.name);
                consume(TOKEN_ID, "expected alias name");
            }

            Value imported = dict_get(mod_val.value.dict, export_name);
            variable_set(local_name, imported);

            if (current_token.type == TOKEN_COMMA)
                consume(TOKEN_COMMA, "expected ','");
            else
                break;
        }
        consume(TOKEN_RIGHTBRACE, "expected '}'");
    }

    static Value default_export_or_module(Value mod_val)
    {
        if (mod_val.type != VAR_MODULE)
            return mod_val;

        Value imported = dict_get(mod_val.value.dict, "default");
        if (imported.type != VAR_NULL)
            return imported;

        return mod_val;
    }

    static int default_block_starts_here()
    {
        if (current_token.type != TOKEN_ID)
            return 0;

        int i = indx;
        while (input[i] && isspace((unsigned char)input[i]))
            i++;
        return input[i] == '{';
    }

    static Value parse_default_block()
    {
        char block_name[64];
        strcpy(block_name, current_token.name);
        forward();

        push_scope();
        consume(TOKEN_LEFTBRACE, "expected '{'");
        while (current_token.type != TOKEN_RIGHTBRACE)
        {
            if (current_token.type == TOKEN_FUNC || current_token.type == TOKEN_DEF)
                define_new_function_ex(0);
            else
                syntax_error("expected func inside default block", current_token);
        }
        consume(TOKEN_RIGHTBRACE, "expected '}'");

        Dict *dict = malloc(sizeof(Dict));
        dict->capacity = scope_stack[scope_depth].count + 4;
        dict->count = 0;
        dict->entries = malloc((size_t)dict->capacity * sizeof(DictEntry));

        Scope *sc = &scope_stack[scope_depth];
        for (int i = 0; i < sc->count; i++)
            dict_set(dict, sc->vars[i]->name, sc->vars[i]);

        pop_scope();

        Value val = {0};
        val.name = block_name;
        val.type = VAR_DICT;
        val.value.dict = dict;
        return val;
    }

    static void set_default_export(Value val)
    {
        variable_set("default", val);
        Scope *sc = &scope_stack[scope_depth];
        for (int i = 0; i < sc->count; i++)
            if (strcmp(sc->vars[i]->name, "default") == 0)
                sc->vars[i]->exported = 1;
    }

    void import_stmt()
    {
        Token import_tok = current_token;
        consume(TOKEN_IMPORT, "expected 'import'");

        if (current_token.type == TOKEN_LEFTBRACE)
        {
            char export_names[64][64];
            char local_names[64][64];
            int import_count = 0;
            char mod_name[512];

            consume(TOKEN_LEFTBRACE, "expected '{'");
            while (current_token.type != TOKEN_RIGHTBRACE)
            {
                if (import_count >= 64)
                    syntax_error("too many named imports", current_token);

                if (current_token.type != TOKEN_ID && current_token.type != TOKEN_DEFAULT)
                    consume(TOKEN_ID, "expected export name");

                strcpy(export_names[import_count], current_token.name);
                strcpy(local_names[import_count], current_token.name);
                forward();

                if (current_token.type == TOKEN_AS)
                {
                    forward();
                    strcpy(local_names[import_count], current_token.name);
                    consume(TOKEN_ID, "expected alias name");
                }

                import_count++;

                if (current_token.type == TOKEN_COMMA)
                    consume(TOKEN_COMMA, "expected ','");
                else
                    break;
            }
            consume(TOKEN_RIGHTBRACE, "expected '}'");
            consume(TOKEN_FROM, "expected 'from'");
            read_module_name(mod_name, sizeof(mod_name));

            Value mod_val = load_module(mod_name, current_token.line);
            if (mod_val.type != VAR_MODULE)
                runtime_error("Module error");

            for (int i = 0; i < import_count; i++)
            {
                Value imported = dict_get(mod_val.value.dict, export_names[i]);
                variable_set(local_names[i], imported);
            }
            return;
        }

        char var_name[64];
        strcpy(var_name, current_token.name);

        char mod_name[512];
        read_module_name(mod_name, sizeof(mod_name));

        int has_modname = 0;

        if (current_token.type == TOKEN_FROM && current_token.line == import_tok.line)
        {
            // import <var> from "<module>" (same line only)
            forward();
            strcpy(mod_name, current_token.name);
            consume(TOKEN_STRING, "expected module name string");
            has_modname = 1;
        }

        if (!has_modname && current_token.type == TOKEN_AS)
        {
            // import <module> as <alias>
            forward();
            strcpy(var_name, current_token.name);
            consume(TOKEN_ID, "expected alias name");
        }
        else if (!has_modname)
        {
            // Use last path segment as variable name
            char *last_slash = strrchr(mod_name, '/');
            if (last_slash)
                strcpy(var_name, last_slash + 1);
            // else keep original var_name (first token)
        }

        Value mod_val = load_module(mod_name, current_token.line);
        Value imported = default_export_or_module(mod_val);
        variable_set(var_name, imported);
    }

    void from_stmt()
    {
        consume(TOKEN_FROM, "expected 'from'");

        char mod_name[512];
        read_module_name(mod_name, sizeof(mod_name));

        consume(TOKEN_IMPORT, "expected 'import'");

        Value mod_val = load_module(mod_name, current_token.line);

        if (current_token.type == TOKEN_LEFTBRACE)
        {
            import_named_exports(mod_val);
            return;
        }

        if (mod_val.type != VAR_MODULE)
            runtime_error("Module error");

        while (1)
        {
            char var_name[64];
            strcpy(var_name, current_token.name);
            // Accept keywords like 'default' as variable names in from-import
            if (current_token.type == TOKEN_ID || current_token.type == TOKEN_DEFAULT)
                forward();
            else
                consume(TOKEN_ID, "expected variable name");

            Value imported = dict_get(mod_val.value.dict, var_name);
            variable_set(var_name, imported);

            if (current_token.type == TOKEN_COMMA)
                consume(TOKEN_COMMA, "expected ','");
            else
                break;
        }
    }

    void handle_typed_decl(int is_const_flag)
    {
        char name[64];
        strcpy(name, current_token.name);
        consume(TOKEN_ID, "expected variable name");
        consume(TOKEN_ASIGN, "expected '='");
        Value val = expression();
        variable_set(name, val);
        if (is_const_flag)
        {
            Scope *sc = &scope_stack[scope_depth];
            for (int i = 0; i < sc->count; i++)
                if (strcmp(sc->vars[i]->name, name) == 0)
                    sc->vars[i]->is_const = 1;
        }
    }

    void program()
    {
        while (current_token.type != TOKEN_EOF)
        {
            if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
            {
                expression();
            }
            else if (current_token.type == TOKEN_EXPORT)
            {
                forward();
                if (current_token.type == TOKEN_DEFAULT)
                {
                    forward();
                    Value val = default_block_starts_here() ? parse_default_block() : expression();
                    set_default_export(val);
                    if (val.type == VAR_DICT || val.type == VAR_MODULE)
                        free_value_internals(&val);
                }
                else if (current_token.type == TOKEN_FUNC || current_token.type == TOKEN_DEF || current_token.type == TOKEN_VOID)
                {
                    define_new_function_ex(1);
                }
                else if (current_token.type == TOKEN_CLASS)
                {
                    class_stmt(1);
                }
                else if (current_token.type == TOKEN_CONST)
                {
                    forward();
                    char export_name[64];
                    strcpy(export_name, current_token.name);
                    handle_typed_decl(1);
                    Scope *sc = &scope_stack[scope_depth];
                    for (int i = 0; i < sc->count; i++)
                        if (strcmp(sc->vars[i]->name, export_name) == 0)
                        {
                            sc->vars[i]->exported = 1;
                            sc->vars[i]->is_const = 1;
                        }
                }
                else if (current_token.type == TOKEN_ID)
                {
                    char export_name[64];
                    strcpy(export_name, current_token.name);
                    assignation();
                    Scope *sc = &scope_stack[scope_depth];
                    for (int i = 0; i < sc->count; i++)
                        if (strcmp(sc->vars[i]->name, export_name) == 0)
                            sc->vars[i]->exported = 1;
                }
                else
                {
                    syntax_error("expected 'default', 'func', or variable after 'export'", current_token);
                }
            }
            else if (current_token.type == TOKEN_CONST)
            {
                forward();
                handle_typed_decl(1);
            }
            else if (current_token.type == TOKEN_VAR ||
                     current_token.type == TOKEN_LET)
            {
                forward();
                handle_typed_decl(0);
            }
            else if (current_token.type == TOKEN_FLOAT_TYPE ||
                     current_token.type == TOKEN_STRING_TYPE ||
                     current_token.type == TOKEN_BOOL_TYPE)
            {
                forward();
                handle_typed_decl(0);
            }
            else if (current_token.type == TOKEN_ID)
            {
                assignation();
            }
            else if (current_token.type == TOKEN_FUNC || current_token.type == TOKEN_DEF || current_token.type == TOKEN_VOID)
            {
                define_new_function_ex(0);
            }
            else if (current_token.type == TOKEN_IF)
            {
                if_stmt();
            }
            else if (current_token.type == TOKEN_WHILE)
            {
                while_stmt();
            }
            else if (current_token.type == TOKEN_FOR)
            {
                for_stmt();
            }
            else if (current_token.type == TOKEN_SWITCH)
            {
                switch_stmt();
            }
            else if (current_token.type == TOKEN_TRY)
            {
                try_stmt();
            }
            else if (current_token.type == TOKEN_THROW)
            {
                throw_stmt();
            }
            else if (current_token.type == TOKEN_CLASS)
            {
                class_stmt(0);
            }
            else if (current_token.type == TOKEN_IMPORT)
            {
                import_stmt();
            }
            else if (current_token.type == TOKEN_FROM)
            {
                from_stmt();
            }
            else
            {
                syntax_error("invalid statement", current_token);
            }
        }
    }
