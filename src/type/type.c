#include "type.h"

void VL_Type_print(const VL_Type type){
    #define CASE(TYPE) case VL_TYPE_GET_ENUM(TYPE): { printf(VL_TYPE_GET_REPR(TYPE)); break; }
    switch(type){ VL_TYPE_MAPPING(CASE) }
    #undef CASE
}
void VL_Type_perror(const VL_Type type){
    printf("[" VLT_RED VLT_BOLD);
    VL_Type_print(type);
    printf(VLT_RESET "]");
}

void VL_Keyword_print(const VL_Keyword keyword){
    #define CASE(KEYWORD) case VL_KEYWORD_GET_ENUM(KEYWORD): { printf(VL_KEYWORD_GET_REPR(KEYWORD)); break; }
    switch(keyword){ VL_KEYWORD_MAPPING(CASE) }
    #undef CASE
}
void VL_Keyword_perror(const VL_Keyword type){
    printf("[" VLT_RED VLT_BOLD);
    VL_Keyword_print(type);
    printf(VLT_RESET "]");
}

void VL_Error_print(const VL_Error err){
    switch(err){
        case VL_ERROR_ARG_MISMATCH:
            printf("ARG MISMATCH");
            break;
        case VL_ERROR_SYMBOL_UNDEFINED:
            printf("SYMBOL UNDEFINED");
            break;
        case VL_ERROR_TYPE_ERROR:
            printf("TYPE ERROR");
            break;
        default:
            printf("UNDEFINED ERROR");
            break;
    }
}
void VL_Error_repr(const VL_Error err){
    VL_Error_print(err);
}

void VL_Symbol_init(VL_Symbol* self, VL_Str* label){
    self->label = label;
    self->hash = VL_Str_hash(self->label);
}
VL_Symbol* VL_Symbol_new(VL_Str* label){
    VL_Symbol* self = malloc(sizeof* self);
    VL_Symbol_init(self, label);
    return self;
}
void VL_Symbol_copy(VL_Symbol* self, VL_Symbol* src){
    self->label = VL_Str_clone(src->label);
    self->hash = src->hash;
}
VL_Symbol* VL_Symbol_clone(VL_Symbol* self){
    VL_Symbol* out = malloc(sizeof* out);
    VL_Symbol_copy(out, self);
    return out;
}

void VL_Symbol_clear(VL_Symbol* self){
    VL_Str_delete(self->label);
}
void VL_Symbol_delete(VL_Symbol* self){
    VL_Symbol_clear(self);
    free(self);
}


void VL_Symbol_print(const VL_Symbol* self){
    VL_Str_print(self->label);
}

void VL_Object_perror(const VL_Object* self){
    printf("[");
    VL_Type_print(self->type);
    printf(VLT_RESET ":" VLT_RED VLT_BOLD);
    VL_Object_repr(self);
    printf(VLT_RESET "]");
}