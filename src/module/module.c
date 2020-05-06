#include "module.h"


VL_Module* VL_Module_new(){
    VL_Module* self = malloc(sizeof* self);
    self->ast_tree = NULL; 
    self->src = NULL;
    return self;
}
void VL_Module_delete(VL_Module* self){
    if(self->src != NULL){
        VL_ModuleSrc_delete(self->src);
    }
    if(self->ast_tree != NULL){
        VL_Object_delete(self->ast_tree);
    }
    free(self);
}

bool VL_Module_parse(VL_Module* self, const VL_Str* file_path){
    self->src = VL_ModuleSrc_new(file_path);
    
    VL_ParseState in = { .ok = true, .p.pos = 0, .p.row = 0, .p.col = 0, .val = NULL };
    VL_ParseState out = VL_ModuleSrc_parse_Lisp(self->src, in);
    self->ast_tree = out.val;

    if(!out.ok){
        self->ast_tree = NULL;

        printf(VLT_ERR("Error") " parsing [" VLT_RED VLT_BOLD);
        VL_Str_print(self->src->file_path);
        printf(VLT_RESET "]\n");

        VL_ModuleSrc_print_error_stack(self->src);
        return true;
    }
    return false;
}
/*

typedef struct{
    VL_Tuple* var_stack;
    VL_Tuple* data_stack;
}VL_Interpreter_State;

VL_Interpreter_State* VL_Interpreter_new(){
    VL_Interpreter_State* out = malloc(sizeof* out);
    out->var_stack = VL_Tuple_new(100);
    out->data_stack = VL_Tuple_new(100);
    return out;
}
void VL_Interpreter_delete(VL_Interpreter_State* state){
    VL_Tuple_delete(state->data_stack);
    VL_Tuple_delete(state->var_stack);
    free(state);
}

void VLI_push(VL_Interpreter_State* self, const VL_Object* value){
    VL_Tuple_append_copy(self->data_stack, value);
}
void VLI_print(VL_Tuple* stack){
    VL_Object* temp = VL_Tuple_pop(stack);
    VL_Object_print(temp);
    VL_Object_delete(temp);
}

#define C(ENUM, TAG, EXPR)                                      \
case VL_TYPE_ ## ENUM: {                                        \
    stack->data[i - 1].data.TAG EXPR stack->data[i].data.TAG;   \
    stack->len--; break;                                        \
}                

#define DEF(NAME, EXPR)                                     \
void VLI_ ## NAME(VL_Tuple* stack){                         \
    if(stack->len >= 2){                                    \
        size_t i = stack->len - 1;                          \
        if(stack->data[i - 1].type == stack->data[i].type){ \
            switch(stack->data[i - 1].type){                \
                C(INT, v_int, EXPR)                         \
                C(FLOAT, v_float, EXPR)                     \
                default:                                    \
                    VL_Object_delete(VL_Tuple_pop(stack));  \
                    VL_Object_delete(VL_Tuple_pop(stack));  \
                    break;                                  \
            }                                               \
        }                                                   \
        else{ VL_Object_delete(VL_Tuple_pop(stack)); }      \
    }else{ printf("Insufficient args!"); }                  \
}

DEF(add, +=)
DEF(sub, -=)
DEF(mul, *=)
DEF(div, /=)

#undef C
#undef DEF

void VLI_eval_sym(VL_Tuple* stack, const VL_Symbol sym){
    #define C(ENUM, EXPR) case VL_SYM_ ## ENUM: { EXPR; break; }
    
    switch(sym){
        C(PRINT, VLI_print(stack); )
        
        C(ADD, VLI_add(stack); )
        C(SUB, VLI_sub(stack); )
        C(MUL, VLI_mul(stack); )
        C(DIV, VLI_div(stack); )
        
        default:
            printf("Error: Symbol not implemented!\n");
            break;
    }
    #undef C
}
void VLI_eval_obj(VL_Interpreter_State* self, const VL_Object* AST);
void VLI_eval_expr(VL_Interpreter_State* self, const VL_Expr* expr){
    if(expr->len > 0){
        const VL_Object* head = expr->data[0].val;

        switch(head->type){
            case VL_TYPE_SYMBOL:{ 
                for(size_t i = 1; i < expr->len; i++){
                    VLI_eval_obj(self, expr->data[i].val);
                }
                VLI_eval_sym(self->data_stack, head->data.symbol);
                break;
            }
            default:
                printf("Error: Functions not implemented!\n");
                break;
        }
    }
}
void VLI_eval_obj(VL_Interpreter_State* self, const VL_Object* AST){
    switch(AST->type){    
        case VL_TYPE_EXPR:
            VLI_eval_expr(self, AST->data.expr);
            break;
        default:
            VLI_push(self, AST);
            break;
    }
}
void VLI_interpret(VL_Interpreter_State* self, const VL_Object* AST){
    VLI_eval_obj(self, AST);
}

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
size_t VL_infix_precedence(const VL_ExprAtom* atom){
    if(atom->val->type == VL_TYPE_SYMBOL){
        #define C(ENUM) case VL_SYM_ ## ENUM:
        #define R(PREC) return PREC;

        switch(atom->val->data.symbol){
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

void VL_Module_infix_bundle(VL_Expr* end, VL_Expr* op_stack){
    VL_ExprAtom* op = VL_Expr_pop(op_stack);
    VL_ExprAtom* left = VL_Expr_pop(end);
    VL_ExprAtom* right = VL_Expr_pop(end);

    VL_Expr* inner_expr = VL_Expr_new(0);
    
    VL_Expr_append(inner_expr, op);
    VL_Expr_append(inner_expr, left);
    VL_Expr_append(inner_expr, right);

    VL_Object* inner_obj = VL_Object_wrap_expr(inner_expr);
    VL_Expr_append_Object(end, inner_obj, left->begin, right->end);
    
    free(op);
    free(left);
    free(right);
}
VL_Expr* VL_Module_infix(VL_Compiler* self, VL_Expr* expr){
    VL_Expr* end = VL_Expr_new(expr->len);
    VL_Expr* op_stack = VL_Expr_new(expr->len / 2);

    while(expr->len > 1){    
        VL_ExprAtom* atom = VL_Expr_pop(expr);
        size_t curr_prec = VL_infix_precedence(atom);
        
        if(curr_prec == SYMBOL_PRECEDENCE){
            VL_Expr_append(end, atom);
        }
        else{
            size_t top_prec;

            while(op_stack->len > 0){
                top_prec = VL_infix_precedence(VL_Expr_rget(op_stack, 0));

                if(top_prec > curr_prec){
                    VL_Module_infix_bundle(end, op_stack);
                }
                else{
                    break;
                }
            }
            VL_Expr_append(op_stack, atom);
        }

        free(atom);
    }
    while(op_stack->len > 0){
        VL_Module_infix_bundle(end, op_stack);
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
VL_Expr* VL_Module_ctime_simplify(VL_Compiler* self, VL_Expr* expr){
    if(expr->len > 0){
        const VL_Object* head = expr->data[0].val;

        if(head->type == VL_TYPE_SYMBOL){
            switch(head->data.symbol){
                case VL_SYM_INFIX:
                    return VL_Module_infix(self, expr);
                default:
                    break;
            }
        }
    }    
    return expr;
}
void VL_Module_expand(VL_Compiler* self, VL_Object* AST){
    if(AST->type == VL_TYPE_EXPR){
        VL_Expr* expr = AST->data.expr;       

        for(size_t i = 0; i < expr->len; i++){
           VL_Module_expand(self, expr->data[i].val);
        }

        AST->data.expr = VL_Module_ctime_simplify(self, expr);
    }
}

void VL_Module_compile(VL_Compiler* self){
    VL_ParseState in = { .ok = true, .p.pos = 0, .p.row = 0, .p.col = 0 }; 
    VL_ParseState out = VL_ModuleSrc_parse_Lisp(self->main_file, in);
    
    if(out.ok){
        VL_ModuleSrc_print_state(self->main_file, in.p, out.p);

        VL_Object_delete(self->AST);
        self->AST = out.val;
        printf("[" VLT_BLU VLT_BOLD);
        VL_Type_print(self->AST->type);
        printf(VLT_RESET "] " VLT_BOLD);
        VL_Object_print(self->AST);
        printf(VLT_RESET "\n");

        printf("--- COMPILING! ---\n");
        VL_Module_expand(self, self->AST);

        if(VL_Module_inspect(self->main_file, self->AST)){
            printf("[" VLT_BLU VLT_BOLD);   
            VL_Type_print(self->AST->type);
            printf(VLT_RESET "] " VLT_BOLD);
            VL_Object_print(self->AST);
            printf(VLT_RESET "\n");

            VL_Interpreter_State* interpreter = VL_Interpreter_new();

            printf("--- INTERPRETING ---\n");
            VLI_interpret(interpreter, self->AST);
            
            VL_Interpreter_delete(interpreter);
        }
    }
    else{
        VL_ModuleSrc_error_stack(self->main_file);
    }
}
*/