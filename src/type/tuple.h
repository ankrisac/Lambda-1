#pragma once
#include "type.h"

void VL_Tuple_init(VL_Tuple* self, size_t len);
VL_Tuple* VL_Tuple_new(size_t len);
void VL_Tuple_clear(VL_Tuple* self);
void VL_Tuple_delete(VL_Tuple* self);

void VL_Tuple_copy(VL_Tuple* self, const VL_Tuple* src);
VL_Tuple* VL_Tuple_clone(const VL_Tuple* self);

void VL_Tuple_append_copy(VL_Tuple* self, const VL_Object* value);
void VL_Tuple_append(VL_Tuple* self, VL_Object* value);
VL_Object* VL_Tuple_pop(VL_Tuple* self);
void VL_Tuple_pop_to(VL_Tuple* self, VL_Object* dest);
void VL_Tuple_drop(VL_Tuple* self);
void VL_Tuple_dropn(VL_Tuple* self, size_t n);

const VL_Object* VL_Tuple_get(const VL_Tuple* self, size_t i);
const VL_Object* VL_Tuple_rget(const VL_Tuple* self, size_t i);
VL_Object* VL_Tuple_mget(VL_Tuple* self, size_t i);
VL_Object* VL_Tuple_mrget(VL_Tuple* self, size_t i);

void VL_Tuple_set(VL_Tuple* self, size_t i, VL_Object* value);
void VL_Tuple_rset(VL_Tuple* self, size_t i, VL_Object* value);

void VL_Tuple_print(const VL_Tuple* self);
void VL_Tuple_repr(const VL_Tuple* self);