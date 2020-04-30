#pragma once
#include "vl_object.h"

typedef struct{
    VL_ObjectData* data;
    VL_Type* type;
    size_t len;
    size_t pos;
}VL_Stack;

void VL_Stack_init(VL_Stack* self);
void VL_Stack_clear(VL_Stack* self);

void VL_Stack_push_int(VL_Stack* self, VL_Int val);
void VL_Stack_push_float(VL_Stack* self, VL_Float val);
void VL_Stack_push_str(VL_Stack* self, const char* val);
void VL_Stack_push_tuple(VL_Stack* self,  const size_t len);

void VL_Stack_add(VL_Stack* stack);
void VL_Stack_sub(VL_Stack* stack);
void VL_Stack_mul(VL_Stack* stack);
void VL_Stack_div(VL_Stack* stack);

void VL_Stack_to_int(VL_Stack* stack); 
void VL_Stack_to_float(VL_Stack* stack);

void VL_Stack_print(VL_Stack* self);
inline void VL_Stack_pop(VL_Stack* self){
    VL_ObjectData_clear(&self->data[self->pos], self->type[self->pos]);
    self->pos--;
}
inline void VL_Stack_println(VL_Stack* self){
    VL_Stack_print(self);
    printf("\n");
}