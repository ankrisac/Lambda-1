#pragma once
#include "type.h"

void VL_Str_init(VL_Str* self, size_t len);
VL_Str* VL_Str_new(size_t len);
void VL_Str_clear(VL_Str* self);
void VL_Str_delete(VL_Str* self);

void VL_Str_print(const VL_Str* self);
void VL_Str_repr(const VL_Str* self);
void VL_Str_print_internal(const VL_Str* self);

void VL_Str_copy(VL_Str* self, const VL_Str* src);
void VL_Str_copy_cstr(VL_Str* self, const char* str);
VL_Str* VL_Str_clone(const VL_Str* self);

void VL_Str_concat(VL_Str* self, const VL_Str* value);
void VL_Str_append_char(VL_Str* self, const char value);
void VL_Str_append_cstr(VL_Str* self, const char* value);
void VL_Str_append_int(VL_Str* self, VL_Int value);

VL_Str* VL_Str_from_int(VL_Int val);
VL_Str* VL_Str_from_cstr(const char* str);
VL_Str* VL_Str_from_file_cstr(const char* file_path);
VL_Str* VL_Str_from_file(const VL_Str* file_path);
VL_Str* VL_Str_from_cin();

char* VL_Str_to_cstr(const VL_Str* self);

void VL_Str_reverse(VL_Str* self);

short VL_Str_cmp(const VL_Str* self, const VL_Str* other);
short VL_Str_cmp_cstr(const VL_Str* self, const char* str);
bool VL_Str_is_char(const VL_Str* self, const char value);
size_t VL_Str_hash(const VL_Str* self);