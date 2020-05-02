#include "vl_compiler.h"

VL_Compiler* VL_Compiler_new(const VL_Str* file_path){
    VL_Compiler* out = malloc(sizeof* out);
    out->error_stack = VL_Tuple_new(0);
    out->main_file = VL_Parser_new(file_path);
    out->AST = VL_Object_new(VL_TYPE_NONE);
    return out;
}
void VL_Compiler_clear(VL_Compiler* self){
    VL_Tuple_delete(self->error_stack);
    VL_Parser_delete(self->main_file);
    VL_Object_delete(self->AST);
}
void VL_Compiler_delete(VL_Compiler* self){
    VL_Compiler_clear(self);
    free(self);
}

const size_t SYMBOL_PRECEDENCE = 0;
size_t VL_infix_precedence(VL_ObjectData* atom, VL_Type type){
    if(type == VL_TYPE_SYMBOL){
        #define C(ENUM) case VL_SYM_ ## ENUM:
        #define R(PREC) return PREC;

        switch(atom->symbol){
            C(EQ) R(1)
            C(OR) R(2)
            C(AND) R(3)
            C(LTE) C(GTE) C(LT) C(GT) R(4)
            C(ADD) C(SUB) R(5)
            C(DIV) C(MUL) R(6)
            default:
                return SYMBOL_PRECEDENCE;
        }
    }
    return SYMBOL_PRECEDENCE;
}
VL_Expr* VLC_infix(VL_Compiler* self, VL_Expr* expr){
    VL_Expr* end = VL_Expr_new(expr->len);
    VL_Expr* op_stack = VL_Expr_new(expr->len / 2);

    #define BUNDLE                                                                                                                  \
        VL_ExprAtom op = VL_Expr_pop(op_stack), left = VL_Expr_pop(end), right = VL_Expr_pop(end), prefix_atom;                     \
        VL_Expr* prefix = VL_Expr_new(0); VL_Expr_append(prefix, op); VL_Expr_append(prefix, left); VL_Expr_append(prefix, right);  \
        prefix_atom.val = VL_Object_wrap_expr(prefix); VL_Expr_append(end, prefix_atom);                                            \
        VL_Object_delete(prefix_atom.val); VL_Object_delete(left.val); VL_Object_delete(right.val); VL_Object_delete(op.val);       \
        
    while(expr->len > 1){    
        VL_ExprAtom atom = VL_Expr_pop(expr);
        size_t curr_prec = VL_infix_precedence(&atom.val->data, atom.val->type);
        
        if(curr_prec == SYMBOL_PRECEDENCE){
            VL_Expr_append(end, atom);
            VL_Object_delete(atom.val);
        }
        else{
            size_t top_prec;

            while(op_stack->len > 0){
                top_prec = VL_infix_precedence(&op_stack->data[op_stack->len - 1], op_stack->type[op_stack->len - 1]);

                if(top_prec > curr_prec){
                    BUNDLE    
                }
                else{
                    break;
                }
            }
            VL_Expr_append(op_stack, atom);
            VL_Object_delete(atom.val);
        }
    }
    while(op_stack->len > 0){
        BUNDLE
    }

    VL_Expr_delete(expr);
    VL_Expr_delete(op_stack);

    VL_Object* out = VL_Expr_pop_Object(end);
    VL_Expr_delete(end);

    expr = out->data.expr;

    out->type = VL_TYPE_NONE;
    VL_Object_delete(out);

    return expr;
}
void __VLC_expand(VL_Compiler* self, VL_ObjectData* AST, VL_Type type){
    if(type == VL_TYPE_EXPR){
        VL_Expr* expr = AST->expr;       

        for(size_t i = 0; i < expr->len; i++){
            __VLC_expand(self, &expr->data[i], expr->type[i]);
        }

        if(expr->len > 0){
            if(expr->type[0] == VL_TYPE_SYMBOL){
                switch(expr->data[0].symbol){
                    case VL_SYM_INFIX:
                        AST->expr = VLC_infix(self, expr);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
void __VLC_inspect(VL_Parser* parser, VL_ObjectData* obj, VL_Type type){
    if(type == VL_TYPE_EXPR){
        VL_Expr* expr = obj->expr;
        for(size_t i = 0; i < expr->len; i++){
            if(!expr->p_begin[i].ok || !expr->p_end[i].ok){
                printf(VLT_RED "Compile Error: ");
                VL_Type_print(type);
                printf(VLT_RESET "] = " VLT_BOLD);
                VL_ObjectData_print(obj, type);
                VLP_print_state(parser, expr->p_begin[i], expr->p_end[i]);
            }
        }   
    }   
}
void VLC_inspect(VL_Compiler* self, VL_Object* AST){
    __VLC_inspect(self->main_file, &AST->data, AST->type);
    printf("\n");
}
void VLC_expand(VL_Compiler* self, VL_Object* AST){
    __VLC_expand(self, &AST->data, AST->type);

    printf("[" VLT_BLU VLT_BOLD);
    VL_Type_print(self->AST->type);
    printf(VLT_RESET "] " VLT_BOLD);
    //VL_Object_print(AST);
    printf(VLT_RESET "\n");
}

void VLC_compile(VL_Compiler* self){
    VLP_State in = { .p.ok = true, .p.pos = 0, .p.row = 0, .p.col = 0 }; 
    VLP_State out = VLP_Lisp(self->main_file, in);
    
    if(out.p.ok){
        VLP_print_state(self->main_file, in.p, out.p);

        VL_Object_delete(self->AST);
        self->AST = out.val;
        printf("[" VLT_BLU VLT_BOLD);
        VL_Type_print(self->AST->type);
        printf(VLT_RESET "] " VLT_BOLD);
        //VL_Object_print(self->AST);
        printf(VLT_RESET "\n");

        printf("--- COMPILING! ---\n");
        VLC_expand(self, self->AST);

        VLC_inspect(self, self->AST);
    }
    else{
        VLP_error_stack(self->main_file);
    }
}