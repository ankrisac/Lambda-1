#pragma once
#include "vl_object.h"
#include "vl_parser.h"

typedef struct{
    VL_Object* AST;
    VL_Tuple* error_stack;
    VL_Parser* main_file;
}VL_Compiler;

VL_Compiler* VL_Compiler_new(const VL_Str* file_path);
void VL_Compiler_clear(VL_Compiler* self);
void VL_Compiler_delete(VL_Compiler* self);

void VLC_compile(VL_Compiler* self);