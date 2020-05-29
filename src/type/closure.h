#include "type.h"

VL_Closure* VL_Closure_new(VL_Closure* parent, size_t len);
void VL_Closure_delete(VL_Closure* self);
void VL_Closure_force_delete(VL_Closure* self);

VL_Closure* VL_Closure_share(VL_Closure* self);
void VL_Closure_print(const VL_Closure* self);

void VL_Closure_insert_cstr(VL_Closure* self, const VL_Str* str, size_t str_hash, VL_Object* value);
void VL_Closure_insert(VL_Closure* self, const VL_Symbol* sym, VL_Object* value);

VL_Object* VL_Closure_find_str(const VL_Closure* self, const VL_Str* str);
VL_Object* VL_Closure_find(const VL_Closure* self, const VL_Symbol* sym);