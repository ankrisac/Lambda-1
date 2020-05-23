#include "type.h"

void VL_SymMap_init(VL_SymMap* self, VL_SymMap* parent, size_t len);
VL_SymMap* VL_SymMap_new(VL_SymMap* parent, size_t len);
void VL_SymMap_clear(VL_SymMap* self);
void VL_SymMap_delete(VL_SymMap* self);

void VL_SymMap_print(const VL_SymMap* self);

void VL_SymMap_insert_cstr(VL_SymMap* self, const VL_Str* str, size_t str_hash, VL_Object* value);
void VL_SymMap_insert(VL_SymMap* self, const VL_Symbol* sym, VL_Object* value);

VL_Object* VL_SymMap_find_str(const VL_SymMap* self, const VL_Str* str);
VL_Object* VL_SymMap_find(const VL_SymMap* self, const VL_Symbol* sym);