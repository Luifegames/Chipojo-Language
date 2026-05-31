#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "raylib.h"
#include "variables.h"

Value native_window(Value *args, int arg_count, int line);
Value native_draw_text(Value *args, int arg_count, int line);
Value native_draw_circle(Value *args, int arg_count, int line);
Value native_draw_rect(Value *args, int arg_count, int line);

#endif