#include "object.h"

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
    #define R_DELETE(TAG)                       \
        if(self->data.arc->weak_ref_count == 0){\
            free(self->data.arc);               \
        }

    #define RS_CLEAR(DESTRUCT, TAG)                 \
        switch(self->data.arc->ref_count){          \
            case 1:                                 \
                DESTRUCT(&self->data.arc->TAG);     \
                R_DELETE(self->data)                \
                break;                              \
            case 0:                                 \
                perror("ARC: Invalid ref count\n"); \
                break;                              \
            default:                                \
                self->data.arc->ref_count--;        \
                break;                              \
        }
    
    #define RW_CLEAR(DESTRUCT, TAG)                 \
        if(self->data.arc->weak_ref_count > 0){     \
            self->data.arc->weak_ref_count--;       \
            if(self->data.arc->weak_ref_count == 0){\
                R_DELETE(TAG)                       \
            }                                       \
        }
    
    #define C(ENUM, EXPR) case VL_TYPE_##ENUM: EXPR; break;
    
    #define RC(TYPE_ENUM, TYPE_TAG, DESTRUCT)           \
        C(RS_##TYPE_ENUM, RS_CLEAR(DESTRUCT, TYPE_TAG)) \
        C(RW_##TYPE_ENUM, RW_CLEAR(DESTRUCT, TYPE_TAG))

    #define ND(TYPE_ENUM) C(TYPE_ENUM, )

    switch(self->type){
        ND(NONE)    ND(BOOL)
        ND(INT)     ND(FLOAT)
        ND(KEYWORD)
        ND(ERROR)

        C(SYMBOL, VL_Symbol_delete(self->data.symbol))
        C(STRING, VL_Str_delete(self->data.str))
        
        RC(STRING, str, VL_Str_clear)
        RC(FUNCTION, fn, VL_Function_clear)

        C(TUPLE, VL_Tuple_delete(self->data.tuple))
        C(EXPR, VL_Expr_delete(self->data.expr))        
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

    #define R_CASE(TYPE_ENUM, EXPR)         \
        case TYPE_ENUM: {                   \
            self->data.arc = src->data.arc; \
            self->type = TYPE_ENUM;         \
            EXPR                            \
            break;                          \
        }
        
    #define R(TYPE)                             \
        R_CASE(VL_TYPE_GET_ENUM(RS_##TYPE),     \
            self->data.arc->ref_count++;)       \
        R_CASE(VL_TYPE_GET_ENUM(RW_##TYPE),     \
            self->data.arc->weak_ref_count++;)

    switch(src->type){
        C(NONE, )           D(BOOL, v_bool) 
        D(INT, v_int)       D(FLOAT, v_float)
        D(KEYWORD, keyword) D(ERROR, err)

        C(SYMBOL, self->data.symbol = VL_Symbol_clone(src->data.symbol))
        C(STRING, self->data.str = VL_Str_clone(src->data.str))
        C(TUPLE, self->data.tuple = VL_Tuple_clone(src->data.tuple))
        C(EXPR, self->data.expr = VL_Expr_clone(src->data.expr))
        
        R(FUNCTION)
        R(STRING)
    }

    #undef C
    #undef D
    #undef R
    #undef R_CASE

    self->type = src->type;
}
VL_Object* VL_Object_clone(const VL_Object* self){
    VL_Object* out = malloc(sizeof* out); 
    VL_Object_copy(out, self);
    return out;
}
void VL_Object_move(VL_Object* self, VL_Object* dest){
    *dest = *self;
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
DEF(keyword, const VL_Keyword, KEYWORD, self->data.keyword = val; )

DEF(symbol, VL_Symbol*, SYMBOL, self->data.symbol = val; )
DEF(cstr, const char*, STRING, self->data.str = VL_Str_from_cstr(val); )
DEF(str, VL_Str*, STRING, self->data.str = val; )
DEF(tuple, VL_Tuple*, TUPLE, self->data.tuple = val; )
DEF(expr, VL_Expr*, EXPR, self->data.expr = val; )
#undef DEF

VL_ARCData* VL_ARCData_malloc(){
    VL_ARCData* self = malloc(sizeof* self);
    self->ref_count = 1;
    self->weak_ref_count = 0;
    return self;
}

void VL_Object_print(const VL_Object* self){
    #define C(ENUM, EXPR) case VL_TYPE_ ## ENUM: EXPR; break;
    
    #define RS(TYPE_ENUM, TYPE_TAG, PRINT_FN)               \
        C(TYPE_ENUM, PRINT_FN(&self->data.arc->TYPE_TAG); )
    
    #define RW(TYPE_ENUM, TYPE_TAG, PRINT_FN)       \
        C(TYPE_ENUM,                                \
            if(self->data.arc->ref_count > 0){      \
                PRINT_FN(&self->data.arc->TYPE_TAG);\
            }                                       \
        )

    #define R(TYPE_ENUM, TYPE_TAG, PRINT_FN)    \
        RS(RS_##TYPE_ENUM, TYPE_TAG, PRINT_FN)  \
        RW(RW_##TYPE_ENUM, TYPE_TAG, PRINT_FN)
    
    switch(self->type){
        C(KEYWORD, VL_Keyword_print(self->data.keyword))
        C(ERROR, VL_Error_print(self->data.err))

        C(NONE, printf("None")) 
        C(BOOL, printf((self->data.v_bool) ? "True" : "False"))
        
        C(INT, printf("%lli", self->data.v_int)) 
        C(FLOAT, printf("%f", self->data.v_float))
        
        C(STRING, VL_Str_print(self->data.str)) 
        C(TUPLE, VL_Tuple_print(self->data.tuple))
        C(EXPR, VL_Expr_print(self->data.expr))

        C(SYMBOL, VL_Symbol_print(self->data.symbol)) 

        R(STRING, str, VL_Str_print)
        R(FUNCTION, fn, VL_Function_print)
    }

    #undef C
    #undef RW
    #undef RS
    #undef R
}
void VL_Object_repr(const VL_Object* self){
    #define C(ENUM, EXPR) case VL_TYPE_ ## ENUM: EXPR; break;
    
    #define RS(TYPE_ENUM, TYPE_TAG, PRINT_FN)       \
        C(TYPE_ENUM,                                \
            PRINT_FN(&self->data.arc->TYPE_TAG);)
    
    #define RW(TYPE_ENUM, TYPE_TAG, PRINT_FN)           \
        C(TYPE_ENUM,                                    \
            if(self->data.arc->ref_count > 0){          \
                PRINT_FN(&self->data.arc->TYPE_TAG);})

    #define R(TYPE_ENUM, TYPE_TAG, PRINT_FN)    \
        RS(RS_##TYPE_ENUM, TYPE_TAG, PRINT_FN)  \
        RW(RW_##TYPE_ENUM, TYPE_TAG, PRINT_FN)
    
    switch(self->type){
        C(KEYWORD, VL_Keyword_print(self->data.keyword)) 
        C(ERROR, VL_Error_repr(self->data.err))

        C(SYMBOL, VL_Symbol_print(self->data.symbol))

        C(NONE, printf("None")) 
        C(BOOL, printf((self->data.v_bool) ? "True" : "False"))
        
        C(INT, printf("%lli", self->data.v_int)) 
        C(FLOAT, printf("%f", self->data.v_float))
        
        C(STRING, VL_Str_repr(self->data.str)) 
        C(TUPLE, VL_Tuple_repr(self->data.tuple))
        C(EXPR, VL_Expr_repr(self->data.expr))
        
        R(STRING, str, VL_Str_print)
        R(FUNCTION, fn, VL_Function_repr)
    }

    #undef C
    #undef RW
    #undef RS
    #undef R
}

void VL_print(VL_Object* self){
    VL_Object_print(self);
}
void VL_println(VL_Object* self){
    VL_print(self);
    printf("\n");
}