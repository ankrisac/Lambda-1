#include "eval_math.h"

void VL_Core_apply_int(VL_Core* self, const VL_Expr* expr){
    if(expr->len == 2){
        VL_Core_eval_obj(self, VL_Expr_get(expr, 2)->val);
        VL_Object* val = VL_Core_data_rget(self, 0);

        switch(val->type){
            case VL_TYPE_INT:{
                break;
            }
            case VL_TYPE_FLOAT:{
                val->data.v_int = (VL_Int)val->data.v_float;
                val->type = VL_TYPE_INT;    
                break;
            }
            default:
                VL_Core_push_error(self, VL_Expr_get(expr, 1),          
                    VLT_ERR("float") " not supported for [" VLT_BOLD VLT_RED
                );                                                      
                VL_Type_print(val->type);                                    
                printf("]" VLT_RESET);                                  
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("int") "expected only " VLT_ERR("1") " argument!"
        );
    }
}
void VL_Core_apply_float(VL_Core* self, const VL_Expr* expr){
    if(expr->len == 2){
        VL_Core_eval_obj(self, VL_Expr_get(expr, 2)->val);
        VL_Object* val = VL_Core_data_rget(self, 0);

        switch(val->type){
            case VL_TYPE_INT:{
                val->data.v_float = (VL_Float)val->data.v_int;
                val->type = VL_TYPE_FLOAT;    
                break;
            }
            case VL_TYPE_FLOAT:
                break;
            default:
                VL_Core_push_error(self, VL_Expr_get(expr, 1),          
                    VLT_ERR("float") " not supported for [" VLT_BOLD VLT_RED
                );                                                      
                VL_Type_print(val->type);                                    
                printf("]" VLT_RESET);                                  
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("float") "expected only " VLT_ERR("1") " argument!"
        );
    }
}

#define BINARY_CASE(OPERATOR, TYPE, TAG, EXPR)              \
VL_Object* lhs = VL_Core_data_rget(self, 0);                \
VL_Core_eval_obj(self, VL_Expr_get(expr, 2)->val);          \
VL_Object* rhs = VL_Core_data_rget(self, 0);                \
if(rhs->type == TYPE){                                      \
    EXPR(lhs->data.TAG, rhs->data.TAG)                      \
    VL_Core_data_drop(self);                                \
}                                                           \
else{                                                       \
    VL_Core_push_error(self, VL_Expr_get(expr, 1),          \
        "[" VLT_ERR(OPERATOR) "] not supported between ["   \
    );                                                      \
    VL_Type_print(TYPE);                                    \
    printf("] and [" VLT_RED VLT_BOLD);                     \
    VL_Type_print(rhs->type);                               \
    printf(VLT_RESET "]\n");                                \
    break;                                                  \
}                                                           \

void VL_Core_apply_add(VL_Core* self, const VL_Expr* expr){
    #define ADD(X, Y) X += Y;
    #define CONCAT(X, Y) VL_Str_append(X, Y);
    #define C(TYPE, TAG, EXPR) case TYPE: { BINARY_CASE("+", TYPE, TAG, EXPR) break; }
    
    if(expr->len == 3){
        VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);
        
        switch(VL_Core_data_rget(self, 0)->type){
            C(VL_TYPE_INT, v_int, ADD)
            C(VL_TYPE_FLOAT, v_float, ADD)
            C(VL_TYPE_STRING, str, CONCAT)
            default:
                VL_Core_push_error(self, VL_Expr_get(expr, 1), 
                    VLT_ERR("+") "not supported for"
                );
                VL_Type_print(VL_Expr_get(expr, 1)->val->type);
                break;
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("+") "expected more than " VLT_ERR("1") " argument!\n"
        );
    }
    #undef ADD
    #undef CONCAT
    #undef C
}
void VL_Core_apply_sub(VL_Core* self, const VL_Expr* expr){
    #define SUB(X, Y) X -= Y;
    #define C(TYPE, TAG, EXPR) case TYPE: { BINARY_CASE("-", TYPE, TAG, EXPR) break; }
    
    if(expr->len == 3){
        VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);
        
        switch(VL_Core_data_rget(self, 0)->type){
            C(VL_TYPE_INT, v_int, SUB)
            C(VL_TYPE_FLOAT, v_float, SUB)
            default:
                VL_Core_push_error(self, VL_Expr_get(expr, 1), 
                    VLT_ERR("+") "not supported for"
                );
                VL_Type_print(VL_Expr_get(expr, 1)->val->type);
                break;
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("-") "expected " VLT_ERR("2") " arguments!\n"
        );
    }
    #undef SUB
    #undef C
}
void VL_Core_apply_mul(VL_Core* self, const VL_Expr* expr){
    #define MUL(X, Y) X *= Y;
    #define C(TYPE, TAG, EXPR) case TYPE: { BINARY_CASE("*", TYPE, TAG, EXPR) break; }
    
    if(expr->len == 3){
        VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);
        
        switch(VL_Core_data_rget(self, 0)->type){
            C(VL_TYPE_INT, v_int, MUL)
            C(VL_TYPE_FLOAT, v_float, MUL)
            default:
                VL_Core_push_error(self, VL_Expr_get(expr, 1), 
                    VLT_ERR("+") "not supported for"
                );
                VL_Type_print(VL_Expr_get(expr, 1)->val->type);
                break;
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("*") "expected " VLT_ERR("2") " argument!\n"
        );
    }
    #undef MUL
    #undef C
}
void VL_Core_apply_div(VL_Core* self, const VL_Expr* expr){
    #define DIV(X, Y) X /= Y;
    #define C(TYPE, TAG, EXPR) case TYPE: { BINARY_CASE("/", TYPE, TAG, EXPR) break; }
    
    if(expr->len == 3){
        VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);
        
        switch(VL_Core_data_rget(self, 0)->type){
            C(VL_TYPE_INT, v_int, DIV)
            C(VL_TYPE_FLOAT, v_float, DIV)
            default:
                VL_Core_push_error(self, VL_Expr_get(expr, 1), 
                    VLT_ERR("+") "not supported for"
                );
                VL_Type_print(VL_Expr_get(expr, 1)->val->type);
                break;
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("*") "expected more than " VLT_ERR("1") " argument!\n"
        );
    }
    #undef DIV
    #undef C
}

void VL_Core_apply_and(VL_Core* self, const VL_Expr* expr){
    #define AND(X, Y) X &= Y;
    #define C(TYPE, TAG, EXPR) case TYPE: { BINARY_CASE("&&", TYPE, TAG, EXPR) break; }
    
    if(expr->len == 3){
        VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);
        
        switch(VL_Core_data_rget(self, 0)->type){
            C(VL_TYPE_BOOL, v_bool, AND)
            default:
                VL_Core_push_error(self, VL_Expr_get(expr, 1), 
                    VLT_ERR("&&") "not supported for"
                );
                VL_Type_print(VL_Expr_get(expr, 1)->val->type);
                break;
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("&&") "expected more than " VLT_ERR("1") " argument!\n"
        );
    }
    #undef AND
    #undef C
}
void VL_Core_apply_or(VL_Core* self, const VL_Expr* expr){
    #define OR(X, Y) X |= Y;
    #define C(TYPE, TAG, EXPR) case TYPE: { BINARY_CASE("||", TYPE, TAG, EXPR) break; }
    
    if(expr->len == 3){
        VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);
        
        switch(VL_Core_data_rget(self, 0)->type){
            C(VL_TYPE_BOOL, v_bool, OR)
            default:
                VL_Core_push_error(self, VL_Expr_get(expr, 1), 
                    VLT_ERR("+") "not supported for"
                );
                VL_Type_print(VL_Expr_get(expr, 1)->val->type);
                break;
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("||") "expected more than " VLT_ERR("1") " argument!\n"
        );
    }
    #undef OR
    #undef C
}
void VL_Core_apply_not(VL_Core* self, const VL_Expr* expr){
    if(expr->len == 2){
        VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);
        VL_Object* val = VL_Core_data_rget(self, 0);

        switch(val->type){
            case VL_TYPE_BOOL:
                val->data.v_bool = !val->data.v_bool;
                break;
            default:
                VL_Core_push_error(self, VL_Expr_get(expr, 1), 
                    VLT_ERR("+") "not supported for"
                );
                VL_Type_print(VL_Expr_get(expr, 1)->val->type);
                break;
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("*") "expected more than " VLT_ERR("1") " argument!\n"
        );
    }
    #undef MUL
    #undef C
}


#define COMPARE_NONE(OP)            \
case VL_TYPE_NONE: {                \
    lhs->data.v_bool = (0 OP 0);    \
    lhs->type = VL_TYPE_BOOL;       \
    VL_Core_data_drop(self);        \
    break;                          \
}    

#define COMPARE_BASE(OP, TYPE_ENUM, TYPE_TAG)                       \
case VL_TYPE_ ## TYPE_ENUM: {                                       \
    lhs->data.v_bool = (lhs->data.TYPE_TAG OP rhs->data.TYPE_TAG);  \
    lhs->type = VL_TYPE_BOOL;                                       \
    VL_Core_data_drop(self);                                        \
    break;                                                          \
}    

#define COMPARE_STRING(OP)                                      \
case VL_TYPE_STRING: {                                          \
    bool val = (VL_Str_cmp(lhs->data.str, rhs->data.str) OP 0); \
    VL_Core_data_dropn(self, 2);                                \
    VL_Core_data_push_bool(self, val);                          \
    break;                                                      \
}    

#define COMPARE_FN(NAME, OP)                                            \
void VL_Core_apply_##NAME(VL_Core* self, const VL_Expr* expr){          \
    if(expr->len == 3){                                                 \
        VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);              \
        VL_Core_eval_obj(self, VL_Expr_get(expr, 2)->val);              \
        VL_Object* lhs = VL_Core_data_rget(self, 1);                    \
        VL_Object* rhs = VL_Core_data_rget(self, 0);                    \
        if(lhs->type != rhs->type){                                     \
            VL_Core_error(self, VL_Expr_get(expr, 1),                   \
                VLT_ERR(#OP) " not supported between "                  \
            );                                                          \
            VL_Type_perror(lhs->type);                                  \
            printf(" and ");                                            \
            VL_Type_perror(rhs->type);                                  \
            printf("\n");                                               \
            VL_Core_data_dropn(self, 2);                                \
            VL_Core_data_push_none(self);                               \
        }                                                               \
        else{                                                           \
            switch(lhs->type){                                          \
                COMPARE_NONE(OP)                                        \
                COMPARE_BASE(OP, BOOL, v_bool)                          \
                COMPARE_BASE(OP, INT, v_int)                            \
                COMPARE_BASE(OP, FLOAT, v_float)                        \
                COMPARE_STRING(OP)                                      \
                default:                                                \
                    VL_Core_data_dropn(self, 2);                        \
                    VL_Core_push_error(self, VL_Expr_get(expr, 1),      \
                        VLT_ERR(#OP) " not supported for"               \
                    );                                                  \
                    VL_Type_perror(VL_Expr_get(expr, 1)->val->type);    \
                    printf("\n");                                       \
                    break;                                              \
            }                                                           \
        }                                                               \
    }                                                                   \
    else{                                                               \
        VL_Core_push_error(self, VL_Expr_get(expr, 0),                  \
            VLT_ERR(#OP) "expected more than "                          \
            VLT_ERR("1") " argument!\n"                                 \
        );                                                              \
    }                                                                   \
}

COMPARE_FN(eq, ==)
COMPARE_FN(lt, <)
COMPARE_FN(lte, <=)
COMPARE_FN(gt, >)
COMPARE_FN(gte, >=)
COMPARE_FN(neq, <)

#undef MATH_CMP
#undef STR_CMP
#undef NONE_CMP
#undef COMPARE_CASE
#undef COMPARE_FN