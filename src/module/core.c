#include "core.h"

VL_Core* VL_Core_new(){
    VL_Core* self = malloc(sizeof* self);
    self->stack.data = VL_Tuple_new(100);
    self->stack.var = VL_Tuple_new(100);
    self->modules = VL_ModuleList_new(1);
    return self;
}
void VL_Core_delete(VL_Core* self){
    VL_Tuple_delete(self->stack.data);
    VL_Tuple_delete(self->stack.var);
    VL_ModuleList_delete(self->modules);
    free(self);
}

void VL_Core_data_push_none(VL_Core* self){
    VL_Object temp;
    temp.type = VL_TYPE_NONE;
    VL_Tuple_append(self->stack.data, &temp);
}
void VL_Core_data_push_bool(VL_Core* self, bool val){
    VL_Object temp;
    temp.type = VL_TYPE_BOOL;
    temp.data.v_bool = val;
    VL_Tuple_append(self->stack.data, &temp);
}
void VL_Core_data_pop_to(VL_Core* self, VL_Object* dest){
    VL_Tuple_pop_to(self->stack.data, dest);
}
VL_Object* VL_Core_data_pop(VL_Core* self){
    return VL_Tuple_pop(self->stack.data);
}
VL_Object* VL_Core_data_rget(VL_Core* self, size_t i){
    return &self->stack.data->data[self->stack.data->len - i - 1];
}
void VL_Core_data_drop(VL_Core* self){
    if(self->stack.data->len > 0){
        VL_Object_clear(&self->stack.data->data[self->stack.data->len - 1]);
        self->stack.data->len--;
    }
}
void VL_Core_data_dropn(VL_Core* self, size_t n){
    size_t min = (n > self->stack.data->len) ? self->stack.data->len : n;
    for(size_t i = 0; i < min; i++){
        VL_Object_clear(&self->stack.data->data[self->stack.data->len - 1]);
        self->stack.data->len--;
    }
}
void VL_Core_data_print(VL_Core* self){
    VL_Object_print(VL_Tuple_rget(self->stack.data, 0));
}



void VL_Core_eval_expr_args(VL_Core* self, const VL_Expr* expr){
    for(size_t i = 1; i < expr->len; i++){
        VL_Core_eval_obj(self, VL_Expr_get(expr, i)->val);
    }
}
void VL_Core_print_location(VL_Core* self, const VL_ExprAtom* atom){
    VL_Module* mod = VL_ModuleList_get_module(self->modules, atom->module_id);
    VL_Module_print_state(mod, atom->begin, atom->end);
}
void VL_Core_error(VL_Core* self, const VL_ExprAtom* source, const char* msg){
    printf(VLT_ERR("\nRuntime Error: "));
    VL_Core_print_location(self, source);
    printf("%s", msg);
}
void VL_Core_push_error(VL_Core* self, const VL_ExprAtom* source, const char* msg){
    VL_Core_error(self, source, msg);
    VL_Core_data_push_none(self);
}

void VL_Core_apply_print(VL_Core* self, const VL_Expr* expr){
    for(size_t i = 1; i < expr->len; i++){
        VL_Core_eval_obj(self, VL_Expr_get(expr, i)->val);
        VL_Object_print(VL_Tuple_rget(self->stack.data, 0));
        VL_Core_data_drop(self);
    }
}
void VL_Core_apply_input(VL_Core* self, const VL_Expr* expr){
    for(size_t i = 1; i < expr->len; i++){
        VL_Core_eval_obj(self, VL_Expr_get(expr, i)->val);
        VL_Object_print(VL_Tuple_rget(self->stack.data, 0));
        VL_Core_data_drop(self);
    }
    VL_Object temp;
    VL_Object_set_str(&temp, VL_Str_from_cin());
    VL_Tuple_append(self->stack.data, &temp);
}
void VL_Core_apply_time(VL_Core* self, const VL_Expr* expr){
    VL_Object time;
    time.type = VL_TYPE_FLOAT;
    time.data.v_float = (VL_Float)clock() / CLOCKS_PER_SEC;
    VL_Tuple_append(self->stack.data, &time);
}

void tuple_args(VL_Core* self, VL_Expr* expr){
    VL_Tuple* tuple = VL_Tuple_new(expr->len);
    
    VL_Object temp;
    for(size_t i = 0; i < expr->len; i++){
        VL_Core_eval_obj(self, VL_Expr_get(expr, i)->val);
        
        VL_Core_data_pop_to(self, &temp);
        VL_Tuple_append(tuple, &temp);    
    }
    VL_Object_set_tuple(&temp, tuple);
    VL_Tuple_append(self->stack.data, &temp);
}
void VL_Core_apply_tuple(VL_Core* self, const VL_Expr* expr){
    if(expr->len == 2){
        const VL_Object* inner = VL_Expr_get(expr, 1)->val;

        switch(inner->type){
            case VL_TYPE_EXPR:
                tuple_args(self, inner->data.expr);
                break;
            case VL_TYPE_TUPLE:
                VL_Core_eval_obj(self, inner);
                break;
            default:
                VL_Core_push_error(self, VL_Expr_get(expr, 0), 
                    VLT_ERR("tuple") " is not defined for "
                );
                VL_Type_perror(VL_Expr_get(expr, 1)->val->type);
                break;
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("tuple") "expected only " VLT_ERR("1") " argument!"
        );
    }
}
void do_args(VL_Core* self, const VL_Expr* expr){
    for(size_t i = 0; i + 1 < expr->len; i++){
        VL_Core_eval_obj(self, VL_Expr_get(expr, i)->val);
        VL_Core_data_drop(self);
    }
    if(expr->len > 0){
        VL_Core_eval_obj(self, VL_Expr_rget(expr, 0)->val);
    }
}
void VL_Core_apply_do(VL_Core* self, const VL_Expr* expr){
    if(expr->len == 2){
        const VL_Object* inner = VL_Expr_get(expr, 1)->val;
        
        if(inner->type == VL_TYPE_EXPR){
            do_args(self, inner->data.expr);
        }
        else{
            VL_Core_eval_obj(self, VL_Expr_rget(expr, 0)->val);
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("do") "expected only " VLT_ERR("1") " argument!"
        );
    }
}
void VL_Core_apply_if(VL_Core* self, const VL_Expr* expr){
    if(expr->len == 4){
        VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);
        VL_Object* cond = VL_Core_data_rget(self, 0);

        if(cond->type == VL_TYPE_BOOL){
            bool ok = cond->data.v_bool;
            VL_Core_data_drop(self);
            
            if(ok){
                VL_Core_eval_obj(self, VL_Expr_get(expr, 2)->val);
            }
            else{
                VL_Core_eval_obj(self, VL_Expr_get(expr, 3)->val);
            }
        }
        else{
            VL_Core_data_drop(self);
            VL_Core_push_error(self, VL_Expr_get(expr, 1),
                VLT_ERR("if") " condition is not a " VLT_ERR("boolean")
            );
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("if") "expected " VLT_ERR("3") " arguments!"
        );
    }
}
void VL_Core_apply_while(VL_Core* self, const VL_Expr* expr){
    if(expr->len == 3){
        while(true){
            VL_Core_eval_obj(self, VL_Expr_get(expr, 1)->val);
            VL_Object* cond = VL_Core_data_rget(self, 0);
            
            if(cond->type == VL_TYPE_BOOL){
                bool ok = cond->data.v_bool;
                VL_Core_data_drop(self);

                if(ok){
                    VL_Core_eval_obj(self, VL_Expr_get(expr, 2)->val);
                }
                else{
                    break;
                }
            }
            else{
                VL_Core_data_drop(self);
                VL_Core_push_error(self, VL_Expr_get(expr, 1),
                    VLT_ERR("while") " condition is not a " VLT_ERR("boolean")
                );
            }
        }
    }
    else{
        VL_Core_push_error(self, VL_Expr_get(expr, 0), 
            VLT_ERR("while") "expected " VLT_ERR("2") " arguments!"
        );
    }
}

void VL_Core_eval_symbol(VL_Core* self, VL_Symbol symbol, const VL_Expr* expr){
    #define C(ENUM, FUNCTION) case VL_SYM_##ENUM: VL_Core_apply_##FUNCTION(self, expr); break;
    
    switch(symbol){
        C(PRINT, print) C(INPUT, input) C(TIME, time)
        C(DO, do)       C(IF, if)       C(WHILE, while)
        C(INT, int)     C(FLOAT, float) C(TUPLE, tuple)
        C(ADD, add)     C(SUB, sub)     C(MUL, mul)     C(DIV, div)
        C(LT, lt)       C(LTE, lte)     C(GT, gt)       C(GTE, gte)
        C(AND, and)     C(OR, or)       C(NOT, not)
        C(EQ, eq)       C(NEQ, neq)
        default:{
            VL_Core_push_error(self, VL_Expr_get(expr, 0), 
                "Symbol [" VLT_RED VLT_BOLD
            );
            VL_Symbol_print(symbol);
            printf(VLT_RESET "] not implemented!\n");
            break;
        }
    }       

    #undef C
}
void VL_Core_eval_expr(VL_Core* self, const VL_Expr* expr){
    const VL_Object* head = VL_Expr_get(expr, 0)->val;
    
    switch(head->type){
        case VL_TYPE_SYMBOL:
            if(head->data.symbol == VL_SYM_INFIX){
                VL_Expr* expand = VL_Core_apply_infix(self, expr);
                VL_Core_eval_expr(self, expand);
                VL_Expr_delete(expand);
            }
            else{
                VL_Core_eval_symbol(self, head->data.symbol, expr);
            }
            break;
        default:
            VL_Core_push_error(self, VL_Expr_get(expr, 0), 
                VLT_RED VLT_BOLD
            );
            VL_Type_print(head->type);
            printf(VLT_RESET " cannot be used as a function\n");
            break;
    }
}
void VL_Core_eval_obj(VL_Core* self, const VL_Object* ast){
    switch(ast->type){
        case VL_TYPE_EXPR:
            VL_Core_eval_expr(self, ast->data.expr);
            break;
        default:
            VL_Tuple_append_copy(self->stack.data, ast);
            break;
    }
}


void VL_Core_exec(VL_Core* self, const VL_Object* ast){
    VL_Core_eval_obj(self, ast);
}
void VL_Core_exec_file(VL_Core* self, const VL_Str* file_path){
    VL_Module* main = VL_ModuleList_add_module(self->modules, file_path);
    
    if(main != NULL){
        main->id = 0;
        
        if(VL_Module_parse(main, file_path)){
            printf("--- Abstract Syntax Tree ---\n");
            VL_Object_repr(main->ast_tree);
        
            printf("\n--- Executed file ---\n");
            VL_Core_eval_obj(self, main->ast_tree);

            printf("\n--- Quitting ---\n");
        }
    }
    else{
        printf(VLT_ERR("Error") ": opening file ");
        VL_Str_print(file_path);
        printf("\n");
    }
}