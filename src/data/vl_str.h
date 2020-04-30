#pragma once
#include "vl_type.h"


void VL_Str_init(VL_Str* self, size_t len);
void VL_Str_clear(VL_Str* self);
void VL_Str_delete(VL_Str* self);

VL_Str* VL_Str_new(size_t len);
VL_Str* VL_Str_copy(VL_Str* self);

void VL_Str_set(VL_Str* self, VL_Str* src);
void VL_Str_set_cstr(VL_Str* self, const char* str);

void VL_Str_append(VL_Str* self, const VL_Str* out);
void VL_Str_append_char(VL_Str* self, char value);
void VL_Str_append_cstr(VL_Str* self, const char* str);
void VL_Str_append_int(VL_Str* self, VL_Int val);

void VL_Str_print(VL_Str* self);
void VL_Str_print_internal(VL_Str* self);

VL_Str* VL_Str_from_int(VL_Int val);
VL_Str* VL_Str_from_cstr(const char* str);
VL_Str* VL_Str_from_file_cstr(const char* file_path);
VL_Str* VL_Str_from_file(VL_Str* file_path);

char* VL_Str_to_cstr(VL_Str* self);

void VL_Str_reverse(VL_Str* self);
int VL_Str_cmp_cstr(const VL_Str* self, const char* str);