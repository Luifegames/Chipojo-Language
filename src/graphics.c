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
Value native_draw_text(Value *args, int arg_count, int line)
{
    if (arg_count != 4)
    {
        runtime_error("draw_text(text,x,y,size)");
    }
    DrawText(args[0].value.str,(int)args[1].value.num,(int)args[2].value.num,(int)args[3].value.num,WHITE);
    Value v = {0};
    v.type = VAR_NULL;

    return v;
}

Value native_draw_circle(Value *args, int arg_count, int line)
{
    if (arg_count != 3)
    {
        runtime_error("draw_circle(x,y,r)");
    }
    DrawCircle(args[0].value.num, (int)args[1].value.num, (int)args[2].value.num,WHITE);
    Value v = {0};
    v.type = VAR_NULL;

    return v;
}

Value native_draw_rect(Value *args, int arg_count, int line)
{
    if (arg_count != 4)
    {
        runtime_error("draw_rect(x,y,w,h)");
    }
    DrawRectangle(args[0].value.num, (int)args[1].value.num, (int)args[2].value.num, (int)args[3].value.num, WHITE);
    Value v = {0};
    v.type = VAR_NULL;

    return v;
}
