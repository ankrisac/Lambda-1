#include "eval_math.h"

/*
const size_t SYMBOL_PRECEDENCE = 0;
size_t precedence(const VL_ExprAtom* atom){
    #define R(PREC) return PREC;
    #define C(ENUM) case VL_SYM_ ## ENUM:

    if(atom->val->type == VL_TYPE_KEYWORD){
        switch(atom->val->data.keyword){
            C(EQ)   C(NEQ)                  R(1)
            C(OR)                           R(2)
            C(AND)                          R(3)
            C(LTE)  C(GTE)  C(LT)   C(GT)   R(4)
            C(ADD)  C(SUB)                  R(5)
            C(DIV)  C(MUL)                  R(6)
            default:
                printf(VLT_RED VLT_BOLD "Warning" VLT_RESET ": [");
                VL_Keyword_print(atom->val->data.keyword);
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
    //printf("Infix!\n");
    return infix(self, VL_Expr_clone(expr));
}
*/