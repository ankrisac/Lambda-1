#pragma once
#include "../util.h"
#include "../type/type.h"

typedef struct{
    VL_SrcPos p;
    VL_Object* val;
    bool ok;
}VL_ParseState;

typedef struct VL_SymbolTableNode VL_SymbolTableNode;
struct VL_SymbolTableNode{
    VL_SymbolTableNode* left;
    VL_SymbolTableNode* right;
    VL_Str* key;
    size_t id;
};

typedef struct{
    VL_SymbolTableNode* root;
    size_t len;
}VL_SymbolTable;

typedef struct{
    VL_Str* file_path;
    VL_Str* source;
    VL_Tuple* error_stack;
    VL_Object* ast_tree;
    VL_SymbolTable* sym_table;
    size_t id;
}VL_Module;

typedef struct{
    VL_Module* data;
    size_t len;
    size_t reserve_len;
}VL_ModuleList;

typedef struct{
    struct{
        VL_Tuple* data;
        VL_Tuple* var;
    }stack;
    VL_ModuleList* modules;
}VL_Core;

VL_Core* VL_Core_new();
void VL_Core_delete(VL_Core* self);

void VL_Core_data_push_none(VL_Core* self);
void VL_Core_data_push_bool(VL_Core* self, bool val);

VL_Object* VL_Core_data_pop(VL_Core* self);
VL_Object* VL_Core_data_rget(VL_Core* self, size_t i);
void VL_Core_data_drop(VL_Core* self);
void VL_Core_data_dropn(VL_Core* self, size_t n);
void VL_Core_data_print(VL_Core* self);

void VL_Core_eval_expr_args(VL_Core* self, const VL_Expr* expr);

void VL_Core_print_location(VL_Core* self, const VL_ExprAtom* atom);
void VL_Core_error(VL_Core* self, const VL_ExprAtom* source, const char* msg);
void VL_Core_push_error(VL_Core* self, const VL_ExprAtom* source, const char* msg);

void VL_Core_apply_print(VL_Core* self, const VL_Expr* expr);
void VL_Core_apply_int(VL_Core* self, const VL_Expr* expr);
void VL_Core_apply_float(VL_Core* self, const VL_Expr* expr);
void VL_Core_apply_do(VL_Core* self, const VL_Expr* expr);

void VL_Core_eval_symbol(VL_Core* self, VL_Symbol symbol, const VL_Expr* expr);
void VL_Core_eval_expr(VL_Core* self, const VL_Expr* expr);
void VL_Core_eval_obj(VL_Core* self, const VL_Object* ast);

void VL_Core_exec(VL_Core* self, const VL_Object* ast);
void VL_Core_exec_file(VL_Core* self, const VL_Str* file_path);

#include "module.h"
#include "parser.h"
#include "source.h"
#include "infix.h"
#include "eval_math.h"
#include "symtable.h"