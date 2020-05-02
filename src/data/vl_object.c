#include "vl_type.h"

VL_ARC_Object* VL_ARC_Object_new(VL_Object* self){
    VL_ARC_Object* out = malloc(sizeof(VL_ARC_Object));
    out->ref_count = 1;
    out->weak_ref_count = 0;
    out->data = *self;
    return out;
}
VL_ARC_Object* VL_ARC_Object_strong_copy(VL_ARC_Object* self){
    self->ref_count++;
    return self;
}
VL_ARC_Object* VL_ARC_Object_weak_copy(VL_ARC_Object* self){    
    self->weak_ref_count++;
    return self;
}
void VL_ARC_Object_strong_clear(VL_ARC_Object* self){
    switch(self->ref_count){
        case 1:{
            VL_Object_clear(&self->data);
            if(self->weak_ref_count == 0){
                free(self);
            }
            break;
        }
        case 0:
            perror("ARC: Invalid reference count");
            break;
        default:
            self->ref_count--;
            break;
    }
}
void VL_ARC_Object_weak_clear(VL_ARC_Object* self){
    if(self->weak_ref_count > 0){
        self->weak_ref_count--;
        if(self->weak_ref_count == 0 && self->ref_count == 0){
            free(self);
        }
    }
}

void VL_ARC_Object_print(VL_ARC_Object* self){
    printf("{S%zu:W%zu}[", self->ref_count, self->weak_ref_count);
    VL_Object_print(&self->data);
    printf("]");
}
void VL_ARC_Object_print_type(VL_ARC_Object* self){
    printf("[S%zu:W%zu]", self->ref_count, self->weak_ref_count);
}

void VL_ObjectData_clear(VL_ObjectData* self, VL_Type type){
    #define C(ENUM, EXPR) case VL_TYPE_##ENUM: EXPR; break;
    switch(type){
        C(SYMBOL, ) C(VARIABLE, )
        C(NONE, ) C(BOOL, ) C(INT, ) C(FLOAT, )
        
        C(STRING, VL_Str_delete(self->str))
        C(TUPLE, VL_Tuple_delete(self->tuple))
        C(EXPR, VL_Expr_delete(self->expr))

        C(ARC_STRONG, VL_ARC_Object_strong_clear(self->arc))
        C(ARC_WEAK, VL_ARC_Object_weak_clear(self->arc))
    }
    #undef C
}
void VL_ObjectData_set(VL_ObjectData* self, VL_ObjectData* src, VL_Type type){
    #define C(ENUM, EXPR) case VL_TYPE_ ## ENUM: EXPR; break;
    #define D(ENUM, TAG) case VL_TYPE_ ## ENUM: self->TAG = src->TAG; break;
    switch(type){
        C(NONE, )

        D(VARIABLE, v_varid)         
        D(SYMBOL, symbol) 
        D(BOOL, v_bool) 
        D(INT, v_int) 
        D(FLOAT, v_float)
 
        C(STRING, self->str = VL_Str_copy(src->str))
        C(TUPLE, self->tuple = VL_Tuple_copy(src->tuple))
        C(EXPR, self->expr = VL_Expr_copy(src->expr))
 
        C(ARC_STRONG, self->arc = VL_ARC_Object_strong_copy(src->arc))
        C(ARC_WEAK, self->arc = VL_ARC_Object_weak_copy(src->arc))
    }
    #undef C
    #undef D
}
void VL_ObjectData_print(const VL_ObjectData* self, VL_Type type){
    #define C(ENUM, EXPR) case VL_TYPE_ ## ENUM: EXPR; break;
    switch(type){
        C(SYMBOL, VL_Symbol_print(self->symbol)) 
        C(VARIABLE, printf("id:%zu", self->v_varid))
        C(NONE, printf("None")) 
        C(BOOL, printf((self->v_bool) ? "True" : "False"))
        
        C(INT, printf("%lli", self->v_int)) 
        C(FLOAT, printf("%f", self->v_float))
        
        C(STRING, VL_Str_print(self->str)) 
        C(TUPLE, VL_Tuple_print(self->tuple))
        C(EXPR, VL_Expr_print(self->expr))
        
        C(ARC_STRONG, printf("ARC Strong"); VL_ARC_Object_print(self->arc))
        C(ARC_WEAK, printf("ARC Weak"); VL_ARC_Object_print_type(self->arc))
    }
    #undef C
}


VL_Object* VL_Object_new(const VL_Type type){
    VL_Object* out = malloc(sizeof* out);
    out->type = type;
    out->data.v_bool = true;
    return out;
}
void VL_Object_clear(VL_Object* self){
    VL_ObjectData_clear(&self->data, self->type);
    self->type = VL_TYPE_NONE;
}
void VL_Object_delete(VL_Object* self){
    VL_Object_clear(self);
    free(self);
}

#define VL_DEF(NAME, TYPE, TYPE_ENUM, EXPR)     \
VL_Object* NAME (TYPE val){                     \
    VL_Object* out = VL_Object_new(TYPE_ENUM);  \
    EXPR                                        \
    return out;                                 \
}
VL_DEF(VL_Object_from_bool, const VL_Bool, VL_TYPE_BOOL, out->data.v_bool = val; )
VL_DEF(VL_Object_from_int, const VL_Int, VL_TYPE_INT, out->data.v_int = val; )
VL_DEF(VL_Object_from_float, const VL_Float, VL_TYPE_FLOAT, out->data.v_float = val; )
VL_DEF(VL_Object_from_symbol, const VL_Symbol, VL_TYPE_SYMBOL, out->data.symbol = val; )

VL_DEF(VL_Object_from_cstr, const char*, VL_TYPE_STRING, out->data.str = VL_Str_from_cstr(val); )
VL_DEF(VL_Object_wrap_str, VL_Str*, VL_TYPE_STRING, out->data.str = val; )
VL_DEF(VL_Object_wrap_tuple, VL_Tuple*, VL_TYPE_TUPLE, out->data.tuple = val; )
VL_DEF(VL_Object_wrap_expr, VL_Expr*, VL_TYPE_EXPR, out->data.expr = val; )

#undef VL_FROM_DEF

VL_Object* VL_Object_make_ref(VL_Object* self){    
    VL_Object* out = VL_Object_new(VL_TYPE_ARC_STRONG);    
    out->data.arc = VL_ARC_Object_new(self);
    self->type = VL_TYPE_NONE;
    return out;
}
VL_Object* VL_Object_strong_share(VL_ARC_Object* self){    
    VL_Object* out = VL_Object_new(VL_TYPE_ARC_STRONG);    
    out->data.arc = self;
    out->data.arc->ref_count++;
    return out;
}
VL_Object* VL_Object_weak_share(VL_ARC_Object* self){
    VL_Object* out = VL_Object_new(VL_TYPE_ARC_WEAK);    
    out->data.arc = self;
    out->data.arc->weak_ref_count++;
    return out;
}

void VL_Object_print(const VL_Object* self){
    VL_ObjectData_print(&self->data, self->type);
}

void VL_print(VL_Object* self){
    VL_Object_print(self);
}
void VL_println(VL_Object* self){
    VL_print(self);
    printf("\n");
}