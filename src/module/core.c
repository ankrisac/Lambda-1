#include "core.h"

VL_Core* VL_Core_new(){
    VL_Core* self = malloc(sizeof* self);
    self->stack = VL_Tuple_new(100);
    self->modules = VL_ModuleList_new(1);
    self->scope_global = VL_SymMap_new(NULL, 10);
    return self;
}
void VL_Core_delete(VL_Core* self){
    VL_Tuple_delete(self->stack);
    VL_ModuleList_delete(self->modules);
    VL_SymMap_delete(self->scope_global);
    free(self);
}

void VL_Core_print_location(VL_Core* self, const VL_ExprAtom* atom){
    VL_Module* mod = VL_ModuleList_get_module(self->modules, atom->module_id);
    VL_Module_print_state(mod, atom->begin, atom->end);
}
void VL_Core_error(VL_Core* self, const VL_ExprAtom* source){
    printf(VLT_ERR("\nRuntime Error: "));
    VL_Core_print_location(self, source);
}

void VL_Core_stack_push(VL_Core* self, VL_Object* obj){
    VL_Tuple_append(self->stack, obj);
}
void VL_Core_stack_push_copy(VL_Core* self, VL_Object* obj){
    VL_Tuple_append_copy(self->stack, obj);
}
void VL_Core_stack_push_none(VL_Core* self){
    VL_Object none;
    none.type = VL_TYPE_NONE;
    VL_Tuple_append(self->stack, &none);
}
void VL_Core_stack_push_bool(VL_Core* self, bool val){
    VL_Object bool_obj;
    bool_obj.type = VL_TYPE_BOOL;
    bool_obj.data.v_bool = val;
    VL_Tuple_append(self->stack, &bool_obj);
}
VL_Object* VL_Core_stack_get(VL_Core* self, size_t i){
    return VL_Tuple_mget(self->stack, i);
}
VL_Object* VL_Core_stack_rget(VL_Core* self, size_t i){
    return VL_Tuple_mrget(self->stack, i);
}
void VL_Core_stack_drop(VL_Core* self){
    VL_Tuple_drop(self->stack);
}
void VL_Core_stack_dropn(VL_Core* self, size_t n){
    VL_Tuple_dropn(self->stack, n);
}
void VL_Core_stack_print(VL_Core* self){
    VL_Tuple_repr(self->stack);
}
void VL_Core_stack_debug(VL_Core* self){
    printf("Stack: ");
    VL_Core_stack_print(self);
    printf("\n");
}

#define CASE(ENUM, EXPR) case ENUM: { EXPR break; }
#define CTYPE(ENUM, EXPR) CASE(VL_TYPE_##ENUM, EXPR)
#define CKEYWORD(ENUM, EXPR) CASE(VL_SYM_##ENUM, EXPR)
#define CDEFAULT(EXPR) default: { EXPR break; }

#define DEF_FUNC(NAME, EXPR)\
void VL_Core_fn_##NAME(VL_Core* self, VL_Expr* expr){ EXPR }

#define BINARY_LOAD_ARGS                            \
    VL_Object* lhs = VL_Core_stack_rget(self, 1);   \
    VL_Object* rhs = VL_Core_stack_rget(self, 0);

#define BINARY_ERR_MSG(KEYWORD)                 \
    VL_Core_error(self, VL_Expr_get(expr, 0));  \
    VL_Keyword_perror(KEYWORD);                 \
    printf(" not supported between ");          \
    VL_Object_perror(lhs);                      \
    printf(" and ");                            \
    VL_Object_perror(rhs);
    
#define BINARY_FUNC(NAME, KEYWORD, CASES)   \
    DEF_FUNC(NAME,                          \
        BINARY_LOAD_ARGS                    \
        if(lhs->type == rhs->type){         \
            switch(lhs->type){              \
                CASES CDEFAULT()}}          \
        BINARY_ERR_MSG(KEYWORD)             \
        VL_Core_stack_dropn(self, 2);       \
        VL_Core_stack_push_none(self);)

#define CASE_NUM(TYPE_ENUM, TYPE_TAG, OP)           \
    CTYPE(TYPE_ENUM,                                \
        lhs->data.TYPE_TAG OP rhs->data.TYPE_TAG;   \
        rhs->type = VL_TYPE_NONE;                   \
        self->stack->len--;                         \
        return;)

#define DEF(NAME, KEYWORD, OP)          \
    BINARY_FUNC(NAME, VL_SYM_##KEYWORD, \
        CASE_NUM(INT, v_int, OP)        \
        CASE_NUM(FLOAT, v_float, OP))

DEF(add, ADD, +=)
DEF(sub, SUB, -=)
DEF(mul, MUL, *=)

BINARY_FUNC(div, VL_SYM_DIV, 
    CASE_NUM(FLOAT, v_float, /=)
    CTYPE(INT,
        if(rhs->data.v_int != 0){
            lhs->data.v_int /= rhs->data.v_int;
            rhs->type = VL_TYPE_NONE;
            self->stack->len--;
            return;}))

BINARY_FUNC(and, VL_SYM_AND, CASE_NUM(BOOL, v_bool, &=))
BINARY_FUNC(or, VL_SYM_OR, CASE_NUM(BOOL, v_bool, |=))

#undef DEF
#undef CASE_NUM

#define CMP_NUM(TYPE_ENUM, TYPE_TAG, OP)                                \
    CTYPE(TYPE_ENUM,                                                    \
        lhs->data.v_bool = (lhs->data.TYPE_TAG OP rhs->data.TYPE_TAG);  \
        lhs->type = VL_TYPE_BOOL;                                       \
        rhs->type = VL_TYPE_NONE;                                       \
        self->stack->len--;                                             \
        return;)        
#define CMP_STR(OP)                                                 \
    CTYPE(STRING,                                                   \
        bool ok = (VL_Str_cmp(lhs->data.str, rhs->data.str) OP 0);  \
        VL_Core_stack_dropn(self, 2);                              \
        VL_Core_stack_push_bool(self, ok);)

#define DEF(NAME, KEYWORD, OP)          \
    BINARY_FUNC(NAME, VL_SYM_##KEYWORD, \
        CMP_NUM(INT, v_int, OP)        \
        CMP_NUM(FLOAT, v_float, OP)    \
        CMP_STR(OP))

DEF(lt, LT, <)
DEF(lte, LTE, <=)
DEF(gt, GT, >)
DEF(gte, GTE, >=)
#undef DEF

#define DEF(NAME, KEYWORD, OP)          \
    BINARY_FUNC(NAME, VL_SYM_##KEYWORD, \
        CTYPE(NONE,                     \
            lhs->data.v_bool = (0 OP 0);\
            lhs->type = VL_TYPE_BOOL;   \
            rhs->type = VL_TYPE_NONE;   \
            self->stack->len--;         \
            return;)                    \
        CMP_NUM(BOOL, v_bool, OP)       \
        CMP_NUM(INT, v_int, OP)         \
        CMP_NUM(FLOAT, v_float, OP)     \
        CMP_STR(OP))

DEF(eq, EQ, ==)
DEF(neq, NEQ, !=)
#undef DEF
#undef CMP_NUM
#undef CMP_STR

void VL_Core_eval_obj(VL_Core* self, VL_SymMap* env, VL_Object* obj);
void VL_Core_eval(VL_Core* self, VL_SymMap* env);

bool VL_Core_eval_symbol(VL_Core* self, VL_SymMap* env, const VL_Symbol* sym){
    VL_Object* val = VL_SymMap_find(env, sym);
    if(val != NULL){
        VL_Core_stack_push(self, val);
        return true;
    }
    VL_Core_stack_push_none(self);
    return false;
}
void VL_Core_eval_ast(VL_Core* self, VL_SymMap* env, VL_Object* obj){    
    switch(obj->type){
        CTYPE(EXPR,
            VL_Expr* expr = obj->data.expr;

            for(size_t i = 0; i < expr->len; i++){
                VL_Core_eval_obj(self, env, VL_Expr_get(expr, i)->val);
            }
        )
        CTYPE(SYMBOL, 
            if(!VL_Core_eval_symbol(self, env, obj->data.symbol)){
                printf(VLT_ERR("Error:") " Cannot find ");
                VL_Object_perror(obj);
                printf("\n");
            }
        )
        CDEFAULT(VL_Core_stack_push_copy(self, obj);)
    }
}
VL_Object* VL_Core_eval_do(VL_Core* self, VL_SymMap* env, VL_Object* obj){
    switch(obj->type){
        CTYPE(EXPR,
            VL_Expr* expr = obj->data.expr;           
            
            if(expr->len > 0){
                for(size_t i = 0; (i + 1) < expr->len; i++){
                    VL_Core_eval_obj(self, env, VL_Expr_get(expr, i)->val);
                    VL_Core_stack_drop(self);
                }
                return VL_Expr_rget(expr, 0)->val;
            } 
        )
        CTYPE(SYMBOL,
            if(!VL_Core_eval_symbol(self, env, obj->data.symbol)){
                printf(VLT_ERR("Error:") " Cannot find ");
                VL_Object_perror(obj);
                printf("\n");
                self->panic = true;
            }
        )
        CDEFAULT(VL_Core_stack_push_copy(self, obj);)
    }
    return NULL;
}
size_t VL_Core_num_args(VL_Core* self, size_t fn_ptr){
    if(self->stack->len > fn_ptr){
        return self->stack->len - fn_ptr - 1;
    }
    return 0;
}
void VL_Core_eval_obj(VL_Core* self, VL_SymMap* env, VL_Object* obj){
    VL_Object* ast = obj;
    bool tail_call, special_form;

    #define BASE_KEYWORD(ENUM, N, EXPR)                         \
        CASE(VL_SYM_##ENUM,                                     \
            size_t args = VL_Core_num_args(self, fn_ptr);       \
            if(args == N){ EXPR }                               \
            else{   VL_Core_error(self, VL_Expr_get(expr, 0));  \
                    VL_Keyword_perror(VL_SYM_##ENUM);           \
                    printf(" expected only " #N " argument(s)");\
                    printf(" not %zu\n", args);                 \
                    VL_Core_stack_dropn(self, args);            \
                    VL_Core_stack_push_none(self);})

    #define EVAL_CASE(KEYWORD, ARGS, FUNCTION)\
        BASE_KEYWORD(KEYWORD, ARGS, VL_Core_fn_##FUNCTION(self, expr);)

    do{
        tail_call = false;
        special_form = false;

        switch(ast->type){
            CTYPE(EXPR, 
                VL_Expr* expr = ast->data.expr;

                if(expr->len == 0){
                    VL_Core_stack_push_copy(self, obj);
                }
                else{
                    size_t fn_ptr = self->stack->len;
                    VL_Object* special_head = VL_Expr_get(expr, 0)->val; 
                    
                    if(special_head->type == VL_TYPE_KEYWORD){
                        special_form = true;

                        switch(special_head->data.keyword){
                            CKEYWORD(SET, 
                                if(expr->len == 3){
                                    VL_Object* label = VL_Expr_get(expr, 1)->val;

                                    if(label->type == VL_TYPE_SYMBOL){
                                        VL_Core_eval_obj(self, env, VL_Expr_get(expr, 2)->val);
                                        VL_Object val;
                                        VL_Tuple_pop_to(self->stack, &val);
                                        VL_SymMap_insert(env, label->data.symbol, &val);
                                    }
                                    else{
                                        printf("set expected label!\n");
                                    }
                                }
                                else{
                                    VL_Core_error(self, VL_Expr_get(expr, 0));
                                    VL_Keyword_perror(VL_SYM_SET);
                                    printf(" requires 2 arguements!\n");
                                    VL_Core_stack_push_none(self);
                                }
                            )
                            CKEYWORD(DO, 
                                VL_Object* obj_expr = VL_Expr_get(expr, 1)->val;
                                tail_call = ((ast = VL_Core_eval_do(self, env, obj_expr)) != NULL);
                            )
                            CKEYWORD(IF,
                                VL_Object cond;
                                VL_Core_eval_obj(self, env, VL_Expr_get(expr, 1)->val);
                                VL_Tuple_pop_to(self->stack, &cond);
                                
                                switch(expr->len){
                                    CASE(3,
                                        switch(cond.type){
                                            CTYPE(BOOL, 
                                                if(cond.data.v_bool){
                                                    ast = VL_Expr_get(expr, 2)->val;
                                                    tail_call = true;
                                                }
                                                VL_Core_stack_push_none(self);
                                            )
                                            CTYPE(NONE, VL_Core_stack_push_none(self);)
                                            CDEFAULT(
                                                ast = VL_Expr_get(expr, 2)->val;
                                                tail_call = false;
                                            )
                                        }
                                    )
                                    CASE(4,               
                                        switch(cond.type){
                                            CTYPE(BOOL, 
                                                ast = VL_Expr_get(expr, (cond.data.v_bool) ? 2 : 3)->val;
                                                tail_call = true;
                                            )
                                            CTYPE(NONE, ast = VL_Expr_get(expr, 3)->val;)
                                            CDEFAULT(ast = VL_Expr_get(expr, 2)->val;)
                                        }
                                    )
                                    CDEFAULT(
                                        VL_Core_error(self, VL_Expr_get(expr, 0));
                                        VL_Keyword_perror(VL_SYM_IF);
                                        printf(" expected only 2-3 arguements!\n");
                                        VL_Core_stack_push_none(self);
                                    )
                                }
                            )
                            CKEYWORD(FN,
                                if(expr->len == 3){
                                    VL_Object* obj_arg = VL_Expr_get(expr, 1)->val;

                                    if(obj_arg->type == VL_TYPE_EXPR){
                                        VL_Function* fn = VL_Function_new(env, 
                                            VL_Expr_clone(obj_arg->data.expr),
                                            VL_Object_clone(VL_Expr_get(expr, 2)->val)); 

                                        VL_Object obj_fn;
                                        VL_Object_set_fn(&obj_fn, fn);

                                        VL_Object_perror(&obj_fn);

                                        VL_Core_stack_push(self, &obj_fn);
                                    }
                                    else{
                                        VL_Core_error(self, VL_Expr_get(expr, 1));
                                        VL_Keyword_perror(VL_SYM_FN);
                                        printf(" argument must be an expression!\n");
                                        VL_Core_stack_push_none(self);        
                                    }
                                }
                                else{
                                    VL_Core_error(self, VL_Expr_get(expr, 0));
                                    VL_Keyword_perror(VL_SYM_FN);
                                    printf(" requires 2 arguements!\n");
                                    VL_Core_stack_push_none(self);
                                }    
                            )
                            CDEFAULT(special_form = false;)
                        }
                    }

                    if(!special_form){
                        VL_Core_eval_ast(self, env, ast);                
                        VL_Object* head = VL_Core_stack_get(self, fn_ptr);

                        switch(head->type){
                            CTYPE(KEYWORD,
                                switch(head->data.keyword){
                                    EVAL_CASE(ADD, 2, add)  EVAL_CASE(SUB, 2, sub)
                                    EVAL_CASE(MUL, 2, mul)  EVAL_CASE(DIV, 2, div)
                                    EVAL_CASE(AND, 2, and)  EVAL_CASE(OR, 2, or)
                                    EVAL_CASE(EQ, 2, eq)    EVAL_CASE(NEQ, 2, neq)
                                    EVAL_CASE(LT, 2, lt)    EVAL_CASE(LTE, 2, lte)
                                    EVAL_CASE(GT, 2, gt)    EVAL_CASE(GTE, 2, gte)

                                    BASE_KEYWORD(PRINT, 1,
                                        VL_Object_print(VL_Core_stack_rget(self, 0));
                                        VL_Core_stack_drop(self);
                                        VL_Core_stack_push_none(self);
                                    )
                                    CDEFAULT(
                                        VL_Keyword_perror(head->data.keyword);
                                        printf(" not defined!\n");
                                    )
                                }

                                VL_Object* ret = VL_Core_stack_rget(self, 0);
                                VL_Object* fn = VL_Core_stack_rget(self, 1);
                                *fn = *ret;
                                self->stack->len--;    
                            )
                            CTYPE(FUNCTION, printf("Functions not defined!\n");)
                            CDEFAULT(
                                size_t args = VL_Core_num_args(self, fn_ptr);
                                VL_Core_error(self, VL_Expr_get(expr, 0));
                                VL_Object_perror(VL_Expr_get(expr, 0)->val);
                                printf(" cannot be called as a function!\n");
                                VL_Core_stack_dropn(self, args + 1);
                                VL_Core_stack_push_none(self);
                            )
                        }
                    }
                }
            )
            CDEFAULT(VL_Core_eval_ast(self, env, ast);)
        }
    } while (tail_call);
}
void VL_Core_eval(VL_Core* self, VL_SymMap* env){
    VL_Core_eval_obj(self, env, VL_Core_stack_get(self, 0));
}

void VL_Core_exec_file(VL_Core* self, const VL_Str* file_path){
    VL_Module* main = VL_ModuleList_add_module(self->modules, file_path);
    
    if(main != NULL){
        main->id = 0;
        
        if(VL_Module_parse(main, file_path)){
            printf("--- Abstract Syntax Tree ---\n");
            VL_Object_repr(main->ast_tree);
        
            printf("\n--- Executed file ---\n");

            VL_Core_eval_obj(self, self->scope_global, main->ast_tree);


            printf("\n--- Quitting ---\n");
        }
    }
    else{
        printf(VLT_ERR("Error") ": opening file ");
        VL_Str_print(file_path);
        printf("\n");
    }
}