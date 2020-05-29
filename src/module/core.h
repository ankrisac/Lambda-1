#pragma once
#include "../util.h"
#include "../type/type.h"

typedef struct{
    VL_SrcPos p;
    VL_Object* val;
    bool ok;
}VL_ParseState;

typedef struct{
    VL_Str* file_path;
    VL_Str* source;
    VL_Tuple* error_stack;
    VL_Object* ast_tree;
    size_t id;
}VL_Module;

typedef struct{
    VL_Module* data;
    size_t len;
    size_t reserve_len;
}VL_ModuleList;

typedef struct{
    VL_Tuple* stack;
    VL_ModuleList* modules;
    VL_Object* ast;
    
    VL_Closure* scope_global;
    VL_Closure* ptable;
    bool panic;
}VL_Core;

VL_Core* VL_Core_new();
void VL_Core_delete(VL_Core* self);

void VL_Core_eval_obj(VL_Core* self, VL_Closure* env, VL_Object* ast);

void VL_Core_print_location(VL_Core* self, const VL_ExprAtom* atom);

void VL_Core_exec(VL_Core* self, const VL_Object* ast);
void VL_Core_exec_file(VL_Core* self, const VL_Str* file_path);
void VL_Core_repl(VL_Core* self);

#include "module.h"
#include "parser.h"
#include "source.h"
#include "eval_math.h"