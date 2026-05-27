#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "raylib.h"
#include "variables.h"

Value native_window(Value *args, int arg_count, int line);
Value native_should_close(Value *args, int arg_count, int line);

Value native_begin_draw(Value *args, int arg_count, int line);
Value native_end_draw(Value *args, int arg_count, int line);

Value native_clear(Value *args, int arg_count, int line);

Value native_draw_text(Value *args, int arg_count, int line);


#endif