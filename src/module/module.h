#pragma once
#include "../util.h"
#include "../type/type.h"

struct VL_ParseState{
    VL_SrcPos p;
    VL_Object* val;
    bool ok;
};
struct VL_ModuleSrc{
    VL_Str* file_path;
    VL_Str* stream;
    VL_Tuple* error_stack;
};
struct VL_Module{
    VL_ModuleSrc* src;
    VL_Object* ast_tree;
};

VL_Module* VL_Module_new();
void VL_Module_delete(VL_Module* self);
bool VL_Module_parse(VL_Module* self, const VL_Str* file_path);
/*
VL_Compiler* VL_Compiler_new(const VL_Str* file_path);
void VL_Compiler_clear(VL_Compiler* self);
void VL_Compiler_delete(VL_Compiler* self);

void VLC_compile(VL_Compiler* self);
*/

#include "parser.h"
#include "source.h"