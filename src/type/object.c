#include "object.h"

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


void VL_Object_init(VL_Object* self, const VL_Type type){
    self->type = type;
    self->data.v_bool = true;
}
VL_Object* VL_Object_new(const VL_Type type){
    VL_Object* self = malloc(sizeof* self);
    VL_Object_init(self, type);
    return self;
}
void VL_Object_clear(VL_Object* self){
    #define C(ENUM, EXPR) case VL_TYPE_##ENUM: EXPR; break;
    switch(self->type){
        C(SYMBOL, ) C(VARIABLE, )
        C(NONE, ) C(BOOL, ) C(INT, ) C(FLOAT, )
        
        C(STRING, VL_Str_delete(self->data.str))
        C(TUPLE, VL_Tuple_delete(self->data.tuple))
        C(EXPR, VL_Expr_delete(self->data.expr))

        C(ARC_STRONG, VL_ARC_Object_strong_clear(self->data.arc))
        C(ARC_WEAK, VL_ARC_Object_weak_clear(self->data.arc))
    }
    #undef C

    self->type = VL_TYPE_NONE;
}
void VL_Object_delete(VL_Object* self){
    VL_Object_clear(self);
    free(self);
}

void VL_Object_copy(VL_Object* self, const VL_Object* src){
    #define C(ENUM, EXPR) case VL_TYPE_ ## ENUM: EXPR; break;
    #define D(ENUM, TAG) case VL_TYPE_ ## ENUM: self->data.TAG = src->data.TAG; break;
    
    switch(src->type){
        C(NONE, )

        D(VARIABLE, v_varid)         
        D(SYMBOL, symbol) 
        D(BOOL, v_bool) 
        D(INT, v_int) 
        D(FLOAT, v_float)
 
        C(STRING, self->data.str = VL_Str_clone(src->data.str))
        C(TUPLE, self->data.tuple = VL_Tuple_clone(src->data.tuple))
        C(EXPR, self->data.expr = VL_Expr_clone(src->data.expr))
 
        C(ARC_STRONG, self->data.arc = VL_ARC_Object_strong_copy(src->data.arc))
        C(ARC_WEAK, self->data.arc = VL_ARC_Object_weak_copy(src->data.arc))
    }

    #undef C
    #undef D

    self->type = src->type;
}
VL_Object* VL_Object_clone(const VL_Object* self){
    VL_Object* out = malloc(sizeof* out); 
    VL_Object_copy(out, self);
    return out;
}
void VL_Object_move(VL_Object* self, VL_Object* dest){
    dest->data = self->data;
    dest->type = self->type;
    self->type = VL_TYPE_NONE;
}
VL_Object* VL_Object_move_ref(VL_Object* self){
    VL_Object* out = malloc(sizeof* out);
    VL_Object_move(self, out);
    return out;
}


#define DEF(NAME, TYPE, TYPE_ENUM, EXPR)                    \
VL_Object* VL_Object_wrap_##NAME (TYPE val){                \
    VL_Object* self = VL_Object_new(VL_TYPE_##TYPE_ENUM);   \
    EXPR return self;                                       \
}                                                           \
void VL_Object_set_##NAME (VL_Object* self, TYPE val){      \
    self->type = VL_TYPE_##TYPE_ENUM; EXPR                  \
}

DEF(bool, const VL_Bool, BOOL, self->data.v_bool = val; )
DEF(int, const VL_Int, INT, self->data.v_int = val; )
DEF(float, const VL_Float, FLOAT, self->data.v_float = val; )
DEF(symbol, const VL_Symbol, SYMBOL, self->data.symbol = val; )

DEF(cstr, const char*, STRING, self->data.str = VL_Str_from_cstr(val); )
DEF(str, VL_Str*, STRING, self->data.str = val; )
DEF(tuple, VL_Tuple*, TUPLE, self->data.tuple = val; )
DEF(expr, VL_Expr*, EXPR, self->data.expr = val; )
#undef DEF

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
    #define C(ENUM, EXPR) case VL_TYPE_ ## ENUM: EXPR; break;
    switch(self->type){
        C(SYMBOL, VL_Symbol_print(self->data.symbol)) 
        C(VARIABLE, printf("id:%zu", self->data.v_varid))
        C(NONE, printf("None")) 
        C(BOOL, printf((self->data.v_bool) ? "True" : "False"))
        
        C(INT, printf("%lli", self->data.v_int)) 
        C(FLOAT, printf("%f", self->data.v_float))
        
        C(STRING, VL_Str_print(self->data.str)) 
        C(TUPLE, VL_Tuple_print(self->data.tuple))
        C(EXPR, VL_Expr_print(self->data.expr))
        
        C(ARC_STRONG, printf("ARC Strong"); VL_ARC_Object_print(self->data.arc))
        C(ARC_WEAK, printf("ARC Weak"); VL_ARC_Object_print_type(self->data.arc))
    }
    #undef C
}
void VL_Object_repr(const VL_Object* self){
    #define C(ENUM, EXPR) case VL_TYPE_ ## ENUM: EXPR; break;
    switch(self->type){
        C(SYMBOL, VL_Symbol_print(self->data.symbol)) 
        C(VARIABLE, printf("id:%zu", self->data.v_varid))
        C(NONE, printf("None")) 
        C(BOOL, printf((self->data.v_bool) ? "True" : "False"))
        
        C(INT, printf("%lli", self->data.v_int)) 
        C(FLOAT, printf("%f", self->data.v_float))
        
        C(STRING, VL_Str_repr(self->data.str)) 
        C(TUPLE, VL_Tuple_repr(self->data.tuple))
        C(EXPR, VL_Expr_repr(self->data.expr))
        
        C(ARC_STRONG, printf("ARC Strong"); VL_ARC_Object_print(self->data.arc))
        C(ARC_WEAK, printf("ARC Weak"); VL_ARC_Object_print_type(self->data.arc))
    }
    #undef C
}

void VL_print(VL_Object* self){
    VL_Object_print(self);
}
void VL_println(VL_Object* self){
    VL_print(self);
    printf("\n");
}