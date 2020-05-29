#include "type.h"

void VL_Function_init(VL_Function* self, VL_Closure* parent_env, VL_Expr* args, VL_Object* body);
VL_Function* VL_Function_new(VL_Closure* parent_env, VL_Expr* args, VL_Object* body);
void VL_Function_clear(VL_Function* self);
void VL_Function_delete(VL_Function* self);

void VL_Function_copy(VL_Function* self, const VL_Function* src);
VL_Function* VL_Function_clone(const VL_Function* self);

const VL_Symbol* VL_Function_getArg(const VL_Function* self, const size_t i);

void VL_Function_print(const VL_Function* self);
void VL_Function_repr(const VL_Function* self);
void VL_Function_print_type(const VL_Function* self);