#ifndef __DATA_H__
#define __DATA_H__
#include <stdlib.h>
#include <stdbool.h>

#include <stdio.h>
#include <string.h>



typedef bool VL_Bool;
typedef long long int VL_Int;
typedef double VL_Float;

typedef union VL_ObjectData VL_ObjectData;
typedef struct VL_ARC_Object VL_ARC_Object;
typedef struct VL_Object VL_Object;

typedef enum{
    VL_SYM_ADD,
    VL_SYM_SUB,
    VL_SYM_MUL,
    VL_SYM_DIV,

    VL_SYM_AND,
    VL_SYM_OR,
    VL_SYM_NOT,

    VL_SYM_LTE,
    VL_SYM_GTE,
    VL_SYM_LT,
    VL_SYM_GT,
    VL_SYM_EQ,

    VL_SYM_FLOAT,
    VL_SYM_INT,

    VL_SYM_PRINT,
    VL_SYM_INFIX,
}VL_Symbol;

typedef enum{
    VL_TYPE_SYMBOL,
    VL_TYPE_LITERAL,

    VL_TYPE_NONE,
    VL_TYPE_BOOL,
    
    VL_TYPE_INT,
    VL_TYPE_FLOAT,

    VL_TYPE_STRING,
    VL_TYPE_TUPLE,

    VL_TYPE_ARC_STRONG,
    VL_TYPE_ARC_WEAK,
}VL_Type;

void VL_Type_print(const VL_Type type);
void VL_Symbol_print(const VL_Symbol symbol);

typedef struct{
    char* data;
    size_t len;
    size_t reserve_len;
}VL_Str;

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



typedef struct{
    VL_ObjectData* data;
    VL_Type* type;
    
    size_t len;
    size_t reserve_len;    
}VL_Tuple;

void VL_Tuple_init(VL_Tuple* self, size_t len);
void VL_Tuple_clear(VL_Tuple* self);
void VL_Tuple_delete(VL_Tuple* self);
VL_Tuple* VL_Tuple_new(size_t len);

void VL_Tuple_append(VL_Tuple* self, VL_Object* other);
VL_Object* VL_Tuple_pop(VL_Tuple* self);

void VL_Tuple_print(const VL_Tuple* self);



union VL_ObjectData{
    VL_Symbol symbol;

    VL_Bool v_bool;
    VL_Int v_int;
    VL_Float v_float;

    VL_Str* str;
    VL_Tuple* tuple;
    
    VL_ARC_Object* arc;
};

void VL_ObjectData_clear(VL_ObjectData* self, VL_Type type);
void VL_ObjectData_set(VL_ObjectData* self, VL_ObjectData* src, VL_Type type);
void VL_ObjectData_print(const VL_ObjectData* self, VL_Type type);

struct VL_Object{
    VL_ObjectData data;
    VL_Type type;
};

struct VL_ARC_Object{
    VL_Object data;

    size_t ref_count;
    size_t weak_ref_count;
};
VL_ARC_Object* VL_ARC_Object_new(VL_Object* self);
void VL_ARC_Object_strong_clear(VL_ARC_Object* self);
void VL_ARC_Object_weak_clear(VL_ARC_Object* self);
void VL_ARC_Object_print(VL_ARC_Object* self);

VL_Object* VL_Object_new(const VL_Type type);

VL_Object* VL_Object_from_bool(const VL_Bool val);
VL_Object* VL_Object_from_int(const VL_Int val);
VL_Object* VL_Object_from_float(const VL_Float val);
VL_Object* VL_Object_from_symbol(const VL_Symbol val);
VL_Object* VL_Object_from_cstr(const char* val);

VL_Object* VL_Object_wrap_str(VL_Str* val);
VL_Object* VL_Object_wrap_tuple(VL_Tuple* val);

VL_Object* VL_Object_new_Tuple(const size_t len); 

VL_Object* VL_Object_make_ref(VL_Object* self);
VL_Object* VL_Object_strong_share(VL_ARC_Object* self);
VL_Object* VL_Object_weak_share(VL_ARC_Object* self);

void VL_Object_clear(VL_Object* self);
void VL_Object_delete(VL_Object* self);

void VL_Object_print(const VL_Object* self);




void VL_print(VL_Object* self);
void VL_println(VL_Object* self);

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

#endif