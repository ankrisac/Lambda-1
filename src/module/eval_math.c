#include "eval_math.h"

const size_t SYMBOL_PRECEDENCE = 0;
size_t precedence(const VL_ExprAtom* atom){
    #define R(PREC) return PREC;
    #define C(ENUM) case VL_SYM_ ## ENUM:

    if(atom->val->type == VL_TYPE_SYMBOL){
        switch(atom->val->data.symbol){
            C(EQ)   C(NEQ)                  R(1)
            C(OR)                           R(2)
            C(AND)                          R(3)
            C(LTE)  C(GTE)  C(LT)   C(GT)   R(4)
            C(ADD)  C(SUB)                  R(5)
            C(DIV)  C(MUL)                  R(6)
            default:
                printf(VLT_RED VLT_BOLD "Warning" VLT_RESET ": [");
                VL_Symbol_print(atom->val->data.symbol);
                printf("] Precedence not defined!\n");
                return SYMBOL_PRECEDENCE;    
        }
    }
    return SYMBOL_PRECEDENCE;

    #undef C
    #undef R
}

void bundle(VL_Expr* end, VL_Expr* op_stack){
    VL_ExprAtom *op = VL_Expr_pop(op_stack), *left = VL_Expr_pop(end), *right = VL_Expr_pop(end);

    VL_Expr* inner_expr = VL_Expr_new(3);
    VL_Expr_append(inner_expr, op);
    VL_Expr_append(inner_expr, left);
    VL_Expr_append(inner_expr, right);

    VL_Expr_append_Object(end, 
        VL_Object_wrap_expr(inner_expr), 
        left->begin, right->end, op->module_id);
    
    free(op);
    free(left);
    free(right);
}

VL_Expr* infix(VL_Core* self, VL_Expr* expr){
    VL_Expr* end = VL_Expr_new(3);
    VL_Expr* op_stack = VL_Expr_new(expr->len / 2 + 1);
    
    while(expr->len > 1){
        VL_ExprAtom* atom = VL_Expr_pop(expr);
        size_t curr_prec = precedence(atom);
        if(curr_prec == SYMBOL_PRECEDENCE){
            VL_Expr_append(end, atom);
        }
        else{
            for(size_t top_prec; op_stack->len > 0 
            && (top_prec = precedence(VL_Expr_rget(op_stack, 0))) > curr_prec; ){
                bundle(end, op_stack);
            }
            VL_Expr_append(op_stack, atom);
        }
        free(atom);
    }
    while(op_stack->len > 0){
        bundle(end, op_stack);   
    }

    VL_Expr_delete(expr);
    VL_Expr_delete(op_stack);

    VL_ExprAtom* out_atom = VL_Expr_pop(end);
    expr = out_atom->val->data.expr;

    out_atom->val->type = VL_TYPE_NONE;
    VL_ExprAtom_delete(out_atom);
    VL_Expr_delete(end);

    return expr;
}

VL_Expr* VL_Core_apply_infix(VL_Core* self, const VL_Expr* expr){
    return infix(self, VL_Expr_clone(expr));
}

#define ARG_CHECK(N, OP, EXPR)                      \
if(expr->len == (N + 1)){ EXPR }                    \
else{                                               \
    VL_Core_push_error(self, VL_Expr_get(expr, 0),  \
        "[" VLT_ERR(#OP) "] expected more than "    \
        VLT_ERR(#N) " argument!\n"                  \
    );                                              \
}

#define UNARY_LOAD_ARGS                             \
VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);  \
VL_Object* arg_1 = VL_Core_data_rget(self, 0);

#define BINARY_LOAD_ARGS                            \
VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);  \
VL_Core_eval_obj(self, VL_Expr_get(expr, 2)->val);  \
VL_Object* arg_1 = VL_Core_data_rget(self, 1);      \
VL_Object* arg_2 = VL_Core_data_rget(self, 0);

#define EMPTY_CASE(X) case VL_TYPE_##X: { break; }
#define BASE_CASE(X, EXPR) case VL_TYPE_##X: { EXPR break; }

#define UNDEFINED_CASE(OP, N)                       \
default:                                            \
    VL_Core_data_dropn(self, N);                    \
    VL_Core_push_error(self, VL_Expr_get(expr, 0),  \
        VLT_ERR(#OP) "not supported for"            \
    );                                              \
    VL_Type_print(VL_Expr_get(expr, 1)->val->type); \
    break;

#define BINARY_CASE(ENUM, EXPR)             \
case VL_TYPE_##ENUM: {                      \
    EXPR VL_Core_data_drop(self); break; }                      

#define OP_CASE(OP, ENUM, TAG)\
BINARY_CASE(ENUM, arg_1->data.TAG = arg_1->data.TAG OP arg_2->data.TAG; )


#define NTYPE_SWITCH(N, OP, CASES)  \
switch(arg_1->type){                \
    CASES UNDEFINED_CASE(OP, N) }

#define UNARY_TYPECHECK(OP, COND, EXPR)             \
if(COND){ EXPR }                                    \
else{                                               \
    VL_Core_error(self, VL_Expr_get(expr, 0),       \
        VLT_ERR("not") "not supported for"          \
    );                                              \
    VL_Type_print(arg_1->type);                     \
    VL_Core_data_drop(self);                        \
    VL_Core_data_push_none(self);                   \
}    

#define BINARY_TYPECHECK(OP, COND, EXPR)            \
if(COND){ EXPR }                                    \
else{                                               \
    VL_Core_error(self, VL_Expr_get(expr, 0),       \
        "[" VLT_ERR(#OP) "] not supported between " \
    );                                              \
    VL_Type_perror(arg_1->type);                    \
    printf(" and ");                                \
    VL_Type_perror(arg_2->type);                    \
    VL_Core_data_dropn(self, 2);                    \
    VL_Core_data_push_none(self);                   \
}

#define BINARY_FN(NAME, OP, TYPE_COND, EXPR)                    \
void VL_Core_apply_##NAME(VL_Core* self, const VL_Expr* expr){  \
    ARG_CHECK(2, OP,                                            \
        BINARY_LOAD_ARGS                                        \
        BINARY_TYPECHECK(OP, TYPE_COND, EXPR))}

#define NUM_CASE(OP)        \
OP_CASE(OP, INT, v_int)     \
OP_CASE(OP, FLOAT, v_float) \

#define CONVERT_FN(NAME, CASES)                                 \
void VL_Core_apply_##NAME(VL_Core* self, const VL_Expr* expr){  \
    ARG_CHECK(1, #NAME,                                         \
        UNARY_LOAD_ARGS                                         \
        NTYPE_SWITCH(1, #NAME, CASES))}

CONVERT_FN(int,
    EMPTY_CASE(INT)
    BASE_CASE(FLOAT,
        arg_1->data.v_int = (VL_Int)arg_1->data.v_float;
        arg_1->type = VL_TYPE_INT;))

CONVERT_FN(float,
    EMPTY_CASE(INT)
    BASE_CASE(FLOAT,
        arg_1->data.v_int = (VL_Int)arg_1->data.v_float;
        arg_1->type = VL_TYPE_INT;))

#define MATH_FN(NAME, OP, CASES)\
BINARY_FN(NAME, OP,             \
    arg_1->type == arg_2->type, \
    NTYPE_SWITCH(2, OP, CASES))

MATH_FN(add, "+",
    NUM_CASE(+)
    BINARY_CASE(STRING, 
        VL_Str_append(arg_1->data.str, arg_2->data.str);))
MATH_FN(sub, "-", NUM_CASE(-))
MATH_FN(mul, "*", NUM_CASE(*))
MATH_FN(div, "/", NUM_CASE(/))

void VL_Core_apply_not(VL_Core* self, const VL_Expr* expr){
    ARG_CHECK(1, "not",
        UNARY_LOAD_ARGS
        UNARY_TYPECHECK(OP,
            arg_1->type == VL_TYPE_BOOL,
            arg_1->data.v_bool = !arg_1->data.v_bool;))}

#define BOOL_FN(NAME, OP)                                           \
void VL_Core_apply_ ##NAME (VL_Core* self, const VL_Expr* expr){    \
    ARG_CHECK(2, #OP,                                               \
        BINARY_LOAD_ARGS                                            \
        BINARY_TYPECHECK(OP,                                        \
            arg_1->type == VL_TYPE_BOOL && arg_2->type == VL_TYPE_BOOL, \
            arg_1->data.v_bool = arg_1->data.v_bool OP arg_2->data.v_bool;\
            VL_Core_data_drop(self);))}

BOOL_FN(and, &&)
BOOL_FN(or, ||)

#define CMP_NONE(OP)                \
BINARY_CASE(NONE,                   \
    arg_1->data.v_bool = (0 OP 0);  \
    arg_1->type = VL_TYPE_BOOL;)

#define CMP_BASE(OP, TYPE_ENUM, TYPE_TAG)                               \
BINARY_CASE(TYPE_ENUM,                                                  \
    arg_1->data.v_bool = (arg_1->data.TYPE_TAG OP arg_2->data.TYPE_TAG);\
    arg_1->type = VL_TYPE_BOOL;)

#define CMP_STRING(OP)                                              \
case VL_TYPE_STRING: {                                              \
    bool val = (VL_Str_cmp(arg_1->data.str, arg_2->data.str) OP 0); \
    VL_Core_data_dropn(self, 2);                                    \
    VL_Core_data_push_bool(self, val);                              \
    break; }    

#define CMP_FN(NAME, OP)            \
BINARY_FN(NAME, OP,                 \
    arg_1->type == arg_2->type,     \
    NTYPE_SWITCH(2, OP,             \
        CMP_NONE(OP)                \
        CMP_BASE(OP, BOOL, v_bool)  \
        CMP_BASE(OP, INT, v_int)    \
        CMP_BASE(OP, FLOAT, v_float)\
        CMP_STRING(OP)))

CMP_FN(eq, ==)
CMP_FN(lt, <)
CMP_FN(lte, <=)
CMP_FN(gt, >)
CMP_FN(gte, >=)
CMP_FN(neq, <)