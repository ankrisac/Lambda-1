#include "type.h"

VL_ARC_Object* VL_ARC_Object_new(VL_Object* self);
void VL_ARC_Object_strong_clear(VL_ARC_Object* self);
void VL_ARC_Object_weak_clear(VL_ARC_Object* self);

VL_ARC_Object* VL_ARC_Object_strong_copy(VL_ARC_Object* self);
VL_ARC_Object* VL_ARC_Object_weak_copy(VL_ARC_Object* self);

void VL_ARC_Object_print(VL_ARC_Object* self);
void VL_ARC_Object_print_type(VL_ARC_Object* self);