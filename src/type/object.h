#pragma once
#include "type.h"

VL_ARC_Object* VL_ARC_Object_new(VL_Object* self);
void VL_ARC_Object_strong_clear(VL_ARC_Object* self);
void VL_ARC_Object_weak_clear(VL_ARC_Object* self);
void VL_ARC_Object_print(VL_ARC_Object* self);

void VL_Object_init(VL_Object* self, const VL_Type type);
VL_Object* VL_Object_new(const VL_Type type);

void VL_Object_copy(VL_Object* self, const VL_Object* src);
VL_Object* VL_Object_clone(const VL_Object* self);
void VL_Object_move(VL_Object* self, VL_Object* dest);
VL_Object* VL_Object_move_ref(VL_Object* self);

#define DEC(NAME, ARG)                          \
VL_Object* VL_Object_wrap_##NAME(ARG);          \
void VL_Object_set_##NAME(VL_Object* self, ARG);

DEC(bool, const VL_Bool val)
DEC(int, const VL_Int val)
DEC(float, const VL_Float val)
DEC(symbol, const VL_Symbol val)
DEC(cstr, const char* val)

DEC(str, VL_Str* val)
DEC(tuple, VL_Tuple* val)
DEC(expr, VL_Expr* val)

VL_Object* VL_Object_make_ref(VL_Object* self);
VL_Object* VL_Object_strong_share(VL_ARC_Object* self);
VL_Object* VL_Object_weak_share(VL_ARC_Object* self);

void VL_Object_clear(VL_Object* self);
void VL_Object_delete(VL_Object* self);

void VL_Object_print(const VL_Object* self);
void VL_Object_repr(const VL_Object* self);

void VL_print(VL_Object* self);
void VL_println(VL_Object* self);