#pragma once
#include "type.h"

void VL_ExprAtom_init(VL_ExprAtom* self);
VL_ExprAtom* VL_ExprAtom_new();
void VL_ExprAtom_clear(VL_ExprAtom* self);
void VL_ExprAtom_delete(VL_ExprAtom* self);

void VL_ExprAtom_copy(VL_ExprAtom* self, const VL_ExprAtom* src);
VL_ExprAtom* VL_ExprAtom_clone(const VL_ExprAtom* self);
void VL_ExprAtom_move(VL_ExprAtom* self, VL_ExprAtom* src);
VL_ExprAtom* VL_ExprAtom_wrap(VL_ExprAtom* self);

void VL_ExprAtom_print(const VL_ExprAtom* self);


void VL_Expr_init(VL_Expr* self, size_t len);
VL_Expr* VL_Expr_new(size_t len);
void VL_Expr_clear(VL_Expr* self);
void VL_Expr_delete(VL_Expr* self);

void VL_Expr_copy(VL_Expr* self, const VL_Expr* src);
VL_Expr* VL_Expr_clone(const VL_Expr* self);

void VL_Expr_append_Object(VL_Expr* self, VL_Object* other, VL_SrcPos begin, VL_SrcPos end, size_t module_id);
void VL_Expr_append(VL_Expr* self, VL_ExprAtom* other);
void VL_Expr_append_expr(VL_Expr* self, const VL_Expr* other);
void VL_Expr_mappend_expr(VL_Expr* self, VL_Expr* other);

VL_Object* VL_Expr_pop_Object(VL_Expr* self);
VL_ExprAtom* VL_Expr_pop(VL_Expr* self);

const VL_ExprAtom* VL_Expr_get(const VL_Expr* self, size_t i);
const VL_ExprAtom* VL_Expr_rget(const VL_Expr* self, size_t i);
VL_ExprAtom* VL_Expr_mget(const VL_Expr* self, size_t i);
VL_ExprAtom* VL_Expr_mrget(const VL_Expr* self, size_t i);
void VL_Expr_set(VL_Expr* self, size_t i, VL_ExprAtom* value);
void VL_Expr_rset(VL_Expr* self, size_t i, VL_ExprAtom* value);

void VL_Expr_print(const VL_Expr* self);
void VL_Expr_repr(const VL_Expr* self);