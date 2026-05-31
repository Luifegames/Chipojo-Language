#include "input.h"

void register_input(){
    assign_number_val("KEY_A",KEY_A);
    assign_number_val("KEY_W", KEY_W);
    assign_number_val("KEY_S", KEY_S);
    assign_number_val("KEY_D", KEY_D);
    assign_number_val("KEY_SPACE", KEY_SPACE);
}

Value key_down_native(Value *args, int count, int line){
    if (count != 1){
        syntax_error_line("key_pressed(key)",line);
    }

    Value v = {0};
    v.type = VAR_NUMBER;
    v.value.num = IsKeyDown((int)args[0].value.num);

    return v;
}