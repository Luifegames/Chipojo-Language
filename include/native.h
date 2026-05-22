#ifndef NATIVE_H
#define NATIVE_H
#include "variables.h"


typedef struct
{
    char *name;
    NativeFunction func;
} NativeFunctions;


Value native_print(Value *args, int arg_count, int line);

void register_natives();

#endif