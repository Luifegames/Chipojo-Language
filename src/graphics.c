#include "graphics.h"
#include "error.h"

Value native_window(Value *args, int arg_count, int line)
{
    if (arg_count != 3)
    {
        runtime_error("window(title,width,height)");
    }

    if (args[0].type != VAR_STRING)
    {
        runtime_error("window title must be string");
    }

    InitWindow(
        (int)args[1].value.num,
        (int)args[2].value.num,
        args[0].value.str);

    SetTargetFPS(60);

    Value v = {0};
    v.type = VAR_NULL;

    return v;
}

Value native_should_close(Value *args, int arg_count, int line)
{
    Value v = {0};

    v.type = VAR_NUMBER;
    v.value.num = WindowShouldClose();

    return v;
}

Value native_begin_draw(Value *args, int arg_count, int line)
{
    BeginDrawing();

    Value v = {0};
    v.type = VAR_NULL;

    return v;
}

Value native_end_draw(Value *args, int arg_count, int line)
{
    EndDrawing();

    Value v = {0};
    v.type = VAR_NULL;

    return v;
}

Value native_clear(Value *args, int arg_count, int line)
{
    ClearBackground(BLACK);

    Value v = {0};
    v.type = VAR_NULL;

    return v;
}

Value native_draw_text(Value *args, int arg_count, int line)
{
    if (arg_count != 4)
    {
        runtime_error("draw_text(text,x,y,size)");
    }

    DrawText(
        args[0].value.str,
        (int)args[1].value.num,
        (int)args[2].value.num,
        (int)args[3].value.num,
        WHITE);

    Value v = {0};
    v.type = VAR_NULL;

    return v;
}
