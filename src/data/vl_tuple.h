#pragma once
#include "vl_type.h"

void VL_Tuple_init(VL_Tuple* self, size_t len);
VL_Tuple* VL_Tuple_new(size_t len);
void VL_Tuple_clear(VL_Tuple* self);
void VL_Tuple_delete(VL_Tuple* self);

void VL_Tuple_set(VL_Tuple* self, VL_Tuple* src);
VL_Tuple* VL_Tuple_copy(VL_Tuple* self);

void VL_Tuple_append(VL_Tuple* self, VL_Object* other);
VL_Object* VL_Tuple_pop(VL_Tuple* self);

void VL_Tuple_print(const VL_Tuple* self);