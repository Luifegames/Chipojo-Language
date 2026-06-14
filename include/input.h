#ifndef INPUT_H
#define INPUT_H

#include "variables.h"
#include "raylib.h"

void register_input();
Value key_down_native(Value *args, int count, int line);

#endif