#pragma once
#include "vl_type.h"

typedef struct{
    VL_Object* val;
    VLP_Pos begin;
    VLP_Pos end;
}VL_ExprAtom;

void VL_Expr_init(VL_Expr* self, size_t len);
VL_Expr* VL_Expr_new(size_t len);
void VL_Expr_clear(VL_Expr* self);
void VL_Expr_delete(VL_Expr* self);

void VL_Expr_set(VL_Expr* self, VL_Expr* src);
VL_Expr* VL_Expr_copy(VL_Expr* self);

void VL_Expr_append_Object(VL_Expr* self, VL_Object* other, VLP_Pos begin, VLP_Pos end);
void VL_Expr_append(VL_Expr* self, VL_ExprAtom other);

VL_Object* VL_Expr_pop_Object(VL_Expr* self);
VL_ExprAtom VL_Expr_pop(VL_Expr* self);
VL_ExprAtom VL_Expr_get(VL_Expr* self, size_t i);

void VL_Expr_print(const VL_Expr* self);