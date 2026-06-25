#ifndef METHODS_H
#define METHODS_H

#include "variables.h"

typedef struct
{
    char *name;
    NativeFunction func;
} MethodEntry;

Value call_method(Value object, char *method,Value *args,int arg_count,int line);

#endif