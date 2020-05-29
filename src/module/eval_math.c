#include "eval_math.h"

void add_prec(VL_Closure* ptable, const char* symbol, int prec){
    VL_Tuple desc;
    VL_Tuple_init(&desc, 3);
    VL_Tuple_append(&desc, &(VL_Object){ .type = VL_TYPE_INT, .data.v_int = prec });

    VL_Object obj;
    VL_Object_init(&obj, VL_TYPE_RS_TUPLE);
    obj.data.arc = VL_ARCData_malloc();
    obj.data.arc->tuple = desc;

    VL_Str* str = VL_Str_from_cstr(symbol);
    VL_Closure_insert_cstr(ptable, str, VL_Str_hash(str), &obj);
    VL_Str_delete(str);
}

VL_Closure* init_ptable(){
    VL_Closure* ptable = VL_Closure_new(NULL, 2);

    size_t n = 1;

    add_prec(ptable, ":=", n);
    add_prec(ptable, "=", n);
    add_prec(ptable, "<-", n);

    add_prec(ptable, "&&", n);
    add_prec(ptable, "||", n++);

    add_prec(ptable, "==", n);
    add_prec(ptable, "!=", n);
    add_prec(ptable, "<", n);
    add_prec(ptable, ">", n);
    add_prec(ptable, "<=", n);
    add_prec(ptable, ">=", n++);

    add_prec(ptable, "+", n);
    add_prec(ptable, "-", n++);

    add_prec(ptable, "*", n);
    add_prec(ptable, "/", n++);

    add_prec(ptable, "$", n);
    add_prec(ptable, ".", n++);
    add_prec(ptable, ":", n++);
    add_prec(ptable, "=>", n++);

    return ptable;
}

const size_t SYMBOL_PRECEDENCE = 0;
VL_Int prec(const VL_Core* self, const VL_ExprAtom* atom){
    if(atom->val->type == VL_TYPE_SYMBOL){
        VL_Object* obj = VL_Closure_find(self->ptable, atom->val->data.symbol);

        if(obj == NULL){
            return SYMBOL_PRECEDENCE;
        }

        return obj->data.tuple->data[0].data.v_int;
    } 
    return SYMBOL_PRECEDENCE;
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
        VL_Int curr_prec = prec(self, atom);

        if(curr_prec == SYMBOL_PRECEDENCE){
            VL_Expr_append(end, atom);
        }
        else{
            for(size_t top_prec; op_stack->len > 0 
            && (top_prec = prec(self, VL_Expr_rget(op_stack, 0))) > curr_prec; ){
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