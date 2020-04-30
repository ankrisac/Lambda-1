#include "vl_stack.h"

void VL_Stack_init(VL_Stack* self){
    self->len = 1024;
    self->data = malloc(self->len * sizeof* self->data);
    self->type = malloc(self->len * sizeof* self->type);
    self->pos = 0;
}
void VL_Stack_clear(VL_Stack* self){
    for(size_t i = 1; i <= self->pos; i++){
        VL_ObjectData_clear(&self->data[i], self->type[i]);
    }
    free(self->data);
    free(self->type);
}

#define DEF(NAME, ARGS, TAG, TAG_ENUM, EXPR)\
void NAME(VL_Stack* self ARGS){             \
    self->pos++;                            \
    self->data[self->pos].TAG = EXPR;       \
    self->type[self->pos] = TAG_ENUM;       \
}

#define COMMA ,
DEF(VL_Stack_push_int,   COMMA VL_Int val, v_int, VL_TYPE_INT, val)
DEF(VL_Stack_push_float, COMMA VL_Float val, v_float, VL_TYPE_FLOAT, val)
DEF(VL_Stack_push_str,   COMMA const char* val, str, VL_TYPE_STRING, VL_Str_from_cstr(val))
DEF(VL_Stack_push_tuple, COMMA const size_t len, tuple, VL_TYPE_TUPLE, VL_Tuple_new(len))
#undef DEF

#define DEF(NAME, EXPR)                                                                 \
void NAME(VL_Stack* self){                                                              \
    if(self->type[self->pos - 1] == self->type[self->pos]){                             \
        switch(self->type[self->pos]){                                                  \
            case VL_TYPE_INT:                                                           \
                EXPR(self->data[self->pos - 1].v_int, self->data[self->pos].v_int)      \
                break;                                                                  \
            case VL_TYPE_FLOAT:                                                         \
                EXPR(self->data[self->pos - 1].v_float, self->data[self->pos].v_float)  \
                break;                                                                  \
            default:                                                                    \
                printf("TypeError!\n");                                                 \
                break;                                                                  \
        }                                                                               \
        self->pos--;                                                                    \
    }                                                                                   \
}

#define ADD(X, Y) X += Y;
#define SUB(X, Y) X -= Y;
#define MUL(X, Y) X *= Y;
#define DIV(X, Y) X /= Y;

DEF(VL_Stack_add, ADD)
DEF(VL_Stack_sub, SUB)
DEF(VL_Stack_mul, MUL)
DEF(VL_Stack_div, DIV)

#undef ADD
#undef SUB
#undef MUL
#undef DIV
#undef DEF

#define DEF(NAME, IN_TYPE, IN_ENUM, OUT_TYPE, OUT_ENUM, EXPR)                       \
void NAME(VL_Stack* self){                                                          \
    switch(self->type[self->pos]){                                                  \
        case OUT_ENUM: break;                                                       \
        case IN_ENUM:                                                               \
            self->type[self->pos] = OUT_ENUM;                                       \
            self->data[self->pos].OUT_TYPE = EXPR(self->data[self->pos].IN_TYPE);   \
            break;                                                                  \
        default:                                                                    \
            printf("TypeError!\n");                                                 \
            break;                                                                  \
    }                                                                               \
}
#define TO_INT(X) (VL_Int)X
#define TO_FLOAT(X) (VL_Float)X
DEF(VL_Stack_to_int, v_float, VL_TYPE_FLOAT, v_int, VL_TYPE_INT, TO_INT)
DEF(VL_Stack_to_float, v_int, VL_TYPE_INT, v_float, VL_TYPE_FLOAT, TO_FLOAT)
#undef TO_INT
#undef TO_FLOAT
#undef DEF

void VL_Stack_print(VL_Stack* self){
    VL_ObjectData_print(&self->data[self->pos], self->type[self->pos]);
}