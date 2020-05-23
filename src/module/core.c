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

#define CASE(TYPE_ENUM, EXPR) case TYPE_ENUM: { EXPR break; }
#define CTYPE(TYPE_ENUM, EXPR) CASE(VL_TYPE_GET_ENUM(TYPE_ENUM), EXPR)
#define CKEYWORD(KEYWORD, EXPR) CASE(VL_KEYWORD_GET_ENUM(KEYWORD), EXPR)
#define CDEFAULT(EXPR) default: { EXPR break; }

#define DEF_FUNC(NAME, EXPR)                                    \
    void VL_Core_fn_##NAME(VL_Core* self, VL_Expr* expr){ EXPR }

#define FUNC_TYPECASE(TYPE_ENUM, EXPR)              \
    CASE(VL_TYPE_GET_ENUM(TYPE_ENUM), EXPR return;)

#define FUNC_ERR(N, EXPR)                       \
    VL_Core_error(self, VL_Expr_get(expr, 0));  \
    EXPR                                        \
    VL_Core_stack_dropn(self, N);               \
    VL_Core_stack_push_none(self);

#define FUNC_KEYWORD_ERR(N, KEYWORD, EXPR)          \
    FUNC_ERR(N, VL_Keyword_perror(KEYWORD); EXPR)
    
#define UNARY_FUNC(NAME, KEYWORD, CASES)                \
    DEF_FUNC(NAME,                                      \
        VL_Object* val = VL_Core_stack_rget(self, 0);   \
        switch(val->type){                              \
            CASES CDEFAULT() }                          \
        FUNC_KEYWORD_ERR(1, KEYWORD,                    \
            printf(" not supported on ");               \
            VL_Object_perror(val);))

#define BINARY_FUNC(NAME, KEYWORD, CASES)               \
    DEF_FUNC(NAME,                                      \
        VL_Object* lhs = VL_Core_stack_rget(self, 1);   \
        VL_Object* rhs = VL_Core_stack_rget(self, 0);   \
        if(lhs->type == rhs->type){                     \
            switch(lhs->type){                          \
                CASES CDEFAULT()}}                      \
        FUNC_KEYWORD_ERR(2, KEYWORD,                    \
            printf(" not supported between ");          \
            VL_Object_perror(lhs);                      \
            printf(" and ");                            \
            VL_Object_perror(rhs);                      \
            printf("\n");))

#define BINARY_CASE_NUM(TYPE_ENUM, TYPE_TAG, OP)   \
    FUNC_TYPECASE(TYPE_ENUM,                       \
        lhs->data.TYPE_TAG OP rhs->data.TYPE_TAG;  \
        rhs->type = VL_TYPE_NONE;                  \
        self->stack->len--;)

DEF_FUNC(input,
    VL_Object obj;
    VL_Str* str = VL_Str_from_cin();
    VL_Object_set_str(&obj, str);
    VL_Core_stack_push(self, &obj);)

DEF_FUNC(time,
    VL_Object obj;
    VL_Object_set_float(&obj, (VL_Float)clock()/CLOCKS_PER_SEC);
    VL_Core_stack_push(self, &obj);)

UNARY_FUNC(not, VL_KEYWORD_NOT, 
    FUNC_TYPECASE(BOOL, 
        val->data.v_bool = !val->data.v_bool;))

UNARY_FUNC(int, VL_KEYWORD_INT, 
    FUNC_TYPECASE(FLOAT, 
        val->data.v_int = (VL_Int)val->data.v_float;
        val->type = VL_TYPE_INT;)
    FUNC_TYPECASE(INT, ))

UNARY_FUNC(float, VL_KEYWORD_FLOAT, 
    FUNC_TYPECASE(INT, 
        val->data.v_float = (VL_Float)val->data.v_int;
        val->type = VL_TYPE_FLOAT;)
    FUNC_TYPECASE(FLOAT, ))

UNARY_FUNC(string, VL_KEYWORD_STRING, 
    FUNC_TYPECASE(STRING,
        VL_Str* str = val->data.str; 
        val->data.arc = VL_ARCData_malloc();
        val->data.arc->str = *str;
        free(str);
        val->type = VL_TYPE_RS_STRING;))


#define DEF(NAME, KEYWORD, OP)              \
    BINARY_FUNC(NAME, VL_KEYWORD_##KEYWORD, \
        BINARY_CASE_NUM(INT, v_int, OP)     \
        BINARY_CASE_NUM(FLOAT, v_float, OP))
DEF(add, ADD, +=)
DEF(sub, SUB, -=)
DEF(mul, MUL, *=)
#undef DEF


BINARY_FUNC(div, VL_KEYWORD_DIV, 
    BINARY_CASE_NUM(FLOAT, v_float, /=)
    CTYPE(INT,
        if(rhs->data.v_int != 0){
            lhs->data.v_int /= rhs->data.v_int;
            rhs->type = VL_TYPE_NONE;
            self->stack->len--;
            return;
        }
        else{
            FUNC_KEYWORD_ERR(2, VL_KEYWORD_DIV, 
                printf(" ");
                VL_Object_perror(lhs);
                printf("/");
                VL_Object_perror(rhs);
                printf(" division by zero");)}))

#define DEF(NAME, KEYWORD, OP)              \
    BINARY_FUNC(NAME, VL_KEYWORD_##KEYWORD, \
        BINARY_CASE_NUM(BOOL, v_bool, OP))
DEF(and, AND, &=)
DEF(or, OR, |=)
#undef DEF

#undef CASE_NUM


#define CMP_NUM(TYPE_ENUM, TYPE_TAG, OP)                                \
    FUNC_TYPECASE(TYPE_ENUM,                                            \
        lhs->data.v_bool = (lhs->data.TYPE_TAG OP rhs->data.TYPE_TAG);  \
        lhs->type = VL_TYPE_BOOL;                                       \
        rhs->type = VL_TYPE_NONE;                                       \
        self->stack->len--;)        

#define CMP_STR(OP)                                                 \
    CTYPE(STRING,                                                   \
        bool ok = (VL_Str_cmp(lhs->data.str, rhs->data.str) OP 0);  \
        VL_Core_stack_dropn(self, 2);                               \
        VL_Core_stack_push_bool(self, ok);)

#define DEF(NAME, KEYWORD, OP)          \
    BINARY_FUNC(NAME, VL_KEYWORD_##KEYWORD, \
        CMP_NUM(INT, v_int, OP)         \
        CMP_NUM(FLOAT, v_float, OP)     \
        CMP_STR(OP))
DEF(lt, LT, <)
DEF(lte, LTE, <=)
DEF(gt, GT, >)
DEF(gte, GTE, >=)
#undef DEF

#define DEF(NAME, KEYWORD, OP)          \
    BINARY_FUNC(NAME, VL_KEYWORD_##KEYWORD, \
        FUNC_TYPECASE(NONE,            \
            lhs->data.v_bool = (0 OP 0);\
            lhs->type = VL_TYPE_BOOL;   \
            rhs->type = VL_TYPE_NONE;   \
            self->stack->len--;)        \
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
        VL_Core_stack_push_copy(self, val);
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
            })
        CTYPE(SYMBOL,
            if(!VL_Core_eval_symbol(self, env, obj->data.symbol)){
                printf(VLT_ERR("Error:") " Cannot find ");
                VL_Object_perror(obj);
                printf("\n");
            })
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

void VL_Core_macro_expand(VL_Core* self, VL_SymMap* env, VL_Object* ast){
    while(true){
        if(ast->type != VL_TYPE_EXPR){ break; }
        VL_Expr* expr = ast->data.expr;
        
        if(expr->len == 0){ break; }
        VL_Object* head = expr->data[0].val;
        
        if(head->type != VL_TYPE_SYMBOL){ break; }
        VL_Object* fn_obj = VL_SymMap_find(env, head->data.symbol);
        
        if(fn_obj == NULL){ break; } 
        if(fn_obj->type != VL_TYPE_RS_FUNCTION){ break; }
        VL_Function* fn = fn_obj->data.fn;
        
        if(!fn->is_macro){ break; }

        if(fn->args->len + 1 != expr->len){
            VL_Core_error(self, VL_Expr_get(expr, 0));
            VL_Object_perror(VL_Expr_get(expr, 0)->val);
            printf(" macro expected %zu argument(s), not %zu!\n", fn->args->len, expr->len - 1);
            break;
        }

        VL_SymMap* new_env = VL_SymMap_new(env, 2);
        VL_Object temp;

        for(size_t i = 0; i + 1 < expr->len; i++){
            VL_Object_copy(&temp, VL_Expr_get(expr, i + 1)->val);
            VL_SymMap_insert(new_env, VL_Function_getArg(fn, i), &temp);
        }

        VL_Core_eval_obj(self, new_env, fn->body);
        
        VL_Object* stack_ret = VL_Core_stack_rget(self, 0);
        VL_Object_clear(ast);
        *ast = *stack_ret;

        self->stack->len--;
        
        VL_SymMap_delete(new_env);
    }
}

VL_Expr* VL_Core_eval_quasiquote(VL_Core* self, VL_SymMap* env, VL_Expr* expr){
    VL_Expr* out = VL_Expr_new(expr->len);

    const VL_ExprAtom* atom;
    
    for(size_t i = 0; i < expr->len; i++){
        atom = VL_Expr_get(expr, i);
        VL_Object* pair_obj = atom->val;

        bool is_pair = false;
        if(pair_obj->type == VL_TYPE_EXPR){
            VL_Expr* pair_expr = pair_obj->data.expr;

            if(pair_expr->len == 2){
                VL_Object* pair_head = pair_expr->data[0].val;
                
                if(pair_head->type == VL_TYPE_KEYWORD){
                    switch(pair_head->data.keyword){
                        CKEYWORD(UNQUOTE,
                            is_pair = true;
                            VL_Core_eval_obj(self, env, VL_Expr_get(pair_expr, 1)->val);

                            VL_Expr_append_Object(out, VL_Tuple_pop(self->stack), 
                                atom->begin, atom->end, atom->module_id);
                        )
                        CKEYWORD(UNQUOTESPLICE,
                            is_pair = true;
                            VL_Core_eval_obj(self, env, pair_expr->data[1].val);
                            
                            VL_Object splice_obj;
                            VL_Tuple_pop_to(self->stack, &splice_obj);
                            
                            if(splice_obj.type == VL_TYPE_EXPR){
                                VL_Expr_mappend_expr(out, splice_obj.data.expr);
                                splice_obj.data.expr->len = 0;
                                VL_Expr_delete(splice_obj.data.expr);
                            }
                            else{
                                VL_Object_clear(&splice_obj);
                                VL_Core_error(self, VL_Expr_get(pair_expr, 1));
                                VL_Keyword_perror(VL_KEYWORD_UNQUOTESPLICE);
                                printf(" expected argument to be expr\n");
                            }
                        )
                        CDEFAULT()
                    }
                }
            }
        }
        
        if(!is_pair){
            VL_Expr_append_Object(out, VL_Object_clone(pair_obj), 
            atom->begin, atom->end, atom->module_id);
        }
    }

    return out;
}

void VL_Core_eval_obj(VL_Core* self, VL_SymMap* env, VL_Object* obj){
    VL_Object* ast = obj;
    bool tail_call, special_form;

    #define SFORM_CASE(KEYWORD, N, EXPR)                            \
        CKEYWORD(KEYWORD,                                           \
            if(expr->len == N + 1){ EXPR }                          \
            else{   VL_Core_error(self, VL_Expr_get(expr, 0));      \
                    VL_Keyword_perror(VL_KEYWORD_GET_ENUM(KEYWORD));\
                    printf(" expected "#N" arguements!\n");         \
                    VL_Core_stack_push_none(self); })

    #define EVAL_CASE(KEYWORD, N, FUNCTION)                     \
        CKEYWORD(KEYWORD,                                       \
            size_t args = VL_Core_num_args(self, fn_ptr);       \
            if(args == N){ VL_Core_fn_##FUNCTION(self, expr); } \
            else{   FUNC_KEYWORD_ERR(N, VL_KEYWORD_##KEYWORD,   \
                    printf(" expected only "#N" argument(s)");  \
                    printf(" not %zu\n", args);)})    
        
    #define SFORM_FUNCTION(KEYWORD, IS_MACRO)                       \
        SFORM_CASE(KEYWORD, 2,                                      \
            VL_Object* obj_arg = VL_Expr_get(expr, 1)->val;         \
            if(obj_arg->type == VL_TYPE_EXPR){                      \
                VL_Function fn;                                     \
                VL_Function_init(&fn, env,                          \
                    VL_Expr_clone(obj_arg->data.expr),              \
                    VL_Object_clone(VL_Expr_get(expr, 2)->val));    \
                VL_Object obj_fn;                                   \
                obj_fn.data.arc = VL_ARCData_malloc();              \
                obj_fn.data.arc->fn = fn;                           \
                obj_fn.data.arc->fn.is_macro = IS_MACRO;            \
                obj_fn.type = VL_TYPE_RS_FUNCTION;                  \
                VL_Core_stack_push(self, &obj_fn);                  \
            }else{  VL_Core_error(self, VL_Expr_get(expr, 1));      \
                    VL_Keyword_perror(VL_KEYWORD_FN);               \
                    printf(" argument must be an expression!\n");   \
                    VL_Core_stack_push_none(self);})
                
    do{
        VL_Core_macro_expand(self, env, ast);

        tail_call = false;
        special_form = false;

        if(ast->type != VL_TYPE_EXPR){
            VL_Core_eval_ast(self, env, ast);
            break;
        } 

        VL_Expr* expr = ast->data.expr;
        if(expr->len == 0){
            VL_Core_stack_push_copy(self, obj);
            break;
        }
        
        size_t fn_ptr = self->stack->len;
        VL_Object* special_head = VL_Expr_get(expr, 0)->val; 
        if(special_head->type == VL_TYPE_KEYWORD){
            special_form = true;

            switch(special_head->data.keyword){
                CKEYWORD(DO, 
                    VL_Object* obj_expr = VL_Expr_get(expr, 1)->val;
                    ast = VL_Core_eval_do(self, env, obj_expr);
                    if(ast != NULL){
                        tail_call = true;
                    })
                SFORM_CASE(QUOTE, 1,
                    VL_Object* obj_expr = VL_Expr_get(expr, 1)->val;
                    VL_Core_stack_push_copy(self, obj_expr);)
                CKEYWORD(QUASIQUOTE, 
                    VL_Object* obj_expr = VL_Expr_get(expr, 1)->val;
                    switch(obj_expr->type){
                        CTYPE(EXPR,
                            VL_Object out_expr;
                            VL_Object_set_expr(&out_expr, 
                                VL_Core_eval_quasiquote(self, env, obj_expr->data.expr));
                            VL_Core_stack_push(self, &out_expr);)
                        CDEFAULT(
                            VL_Core_error(self, VL_Expr_get(expr, 1));
                            VL_Keyword_perror(VL_KEYWORD_QUASIQUOTE);
                            printf(" requires first argument to be an expression!\n");
                            VL_Core_stack_push_none(self);)
                    })
                SFORM_CASE(SET, 2,
                    VL_Object* label = VL_Expr_get(expr, 1)->val;
                    if(label->type == VL_TYPE_SYMBOL){
                        VL_Core_eval_obj(self, env, VL_Expr_get(expr, 2)->val);
                        VL_Object val;
                        VL_Tuple_pop_to(self->stack, &val);
                        VL_SymMap_insert(env, label->data.symbol, &val);
                    }
                    else{
                        VL_Core_error(self, VL_Expr_get(expr, 1));
                        VL_Keyword_perror(VL_KEYWORD_SET);
                        printf(" requires first argument to be a symbol!\n");
                        VL_Core_stack_push_none(self);
                    })
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
                                    VL_Core_stack_push_none(self);)
                                CTYPE(NONE, VL_Core_stack_push_none(self);)
                                CDEFAULT(
                                    ast = VL_Expr_get(expr, 2)->val;
                                    tail_call = false;)
                            })
                        CASE(4,               
                            switch(cond.type){
                                CTYPE(BOOL, 
                                    ast = VL_Expr_get(expr, (cond.data.v_bool) ? 2 : 3)->val;
                                    tail_call = true;
                                )
                                CTYPE(NONE, ast = VL_Expr_get(expr, 3)->val;)
                                CDEFAULT(ast = VL_Expr_get(expr, 2)->val;)
                            })
                        CDEFAULT(
                            VL_Core_error(self, VL_Expr_get(expr, 0));
                            VL_Keyword_perror(VL_KEYWORD_IF);
                            printf(" expected only 2-3 arguements!\n");
                            VL_Core_stack_push_none(self);)
                    })
                SFORM_CASE(WHILE, 2,
                    bool loop = true;
                    VL_Object* expr_cond = VL_Expr_get(expr, 1)->val;
                    VL_Object* expr_body = VL_Expr_get(expr, 2)->val;
                    VL_Object cond;
                    do{
                        loop = false;
                        VL_Core_eval_obj(self, env, expr_cond);
                        VL_Tuple_pop_to(self->stack, &cond);
                        switch(cond.type){
                            CTYPE(BOOL, 
                                if(cond.data.v_bool){
                                    VL_Core_eval_obj(self, env, expr_body);
                                    VL_Core_stack_drop(self);
                                    loop = true;   
                                })
                            CDEFAULT()
                        }
                    } while(loop);
                    VL_Core_stack_push_none(self);
                    tail_call = false;)
                SFORM_FUNCTION(MACRO, true)
                SFORM_FUNCTION(FN, false)
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
                        EVAL_CASE(NOT, 1, not)

                        EVAL_CASE(EQ, 2, eq)    EVAL_CASE(NEQ, 2, neq)
                        EVAL_CASE(LT, 2, lt)    EVAL_CASE(LTE, 2, lte)
                        EVAL_CASE(GT, 2, gt)    EVAL_CASE(GTE, 2, gte)
                        
                        EVAL_CASE(INT, 1, int)  EVAL_CASE(FLOAT, 1, float)
                        EVAL_CASE(STRING, 1, string)

                        EVAL_CASE(INPUT, 0, input)
                        EVAL_CASE(TIME, 0, time)

                        CKEYWORD(PRINT,       
                            size_t args = VL_Core_num_args(self, fn_ptr); 
                            for(size_t i = 0; i < args; i++){
                                VL_Object_print(VL_Core_stack_get(self, fn_ptr + i + 1));
                            }
                            VL_Core_stack_dropn(self, args);
                            VL_Core_stack_push_none(self);)    
                        CDEFAULT(
                            size_t args = VL_Core_num_args(self, fn_ptr);
                            VL_Core_error(self, VL_Expr_get(expr, 0));
                            VL_Object_perror(VL_Core_stack_get(self, fn_ptr));
                            printf(" Viper broke and we got a non function!\n");
                            VL_Core_stack_dropn(self, args);
                            return;)
                    }

                    VL_Object* ret = VL_Core_stack_rget(self, 0);
                    VL_Object* fn = VL_Core_stack_rget(self, 1);
                    *fn = *ret;
                    self->stack->len--;)
                CTYPE(RW_FUNCTION,
                    size_t args = VL_Core_num_args(self, fn_ptr);

                    VL_Core_error(self, VL_Expr_get(expr, 0));
                    VL_Object_perror(VL_Core_stack_get(self, fn_ptr + 1));
                    printf(" weak function not implemented!\n");
                    VL_Core_stack_dropn(self, args + 1);
                    VL_Core_stack_push_none(self);)
                CTYPE(RS_FUNCTION,
                    VL_Function* fn = &head->data.arc->fn;

                    if(fn->args->len + 1 == expr->len){ 
                        VL_SymMap* new_env = VL_SymMap_new(env, 2);

                        for(size_t i = 0; i + 1 < expr->len; i++){
                            VL_SymMap_insert(new_env, VL_Function_getArg(fn, i), VL_Core_stack_get(self, fn_ptr + i + 1));
                        }
                        
                        self->stack->len = self->stack->len + 1- expr->len;

                        VL_Core_eval_obj(self, new_env, fn->body);

                        VL_Object* ret = VL_Core_stack_rget(self, 0);
                        VL_Object* fn = VL_Core_stack_rget(self, 1);
                        VL_Object_clear(fn);
                        *fn = *ret;
                        self->stack->len--;
                        
                        VL_SymMap_delete(new_env);
                    }
                    else{
                        size_t args = VL_Core_num_args(self, fn_ptr);

                        VL_Core_error(self, VL_Expr_get(expr, 0));
                        VL_Object_perror(VL_Core_stack_get(self, fn_ptr + 1));
                        printf(" function expected %zu argument(s), not %zu!\n", fn->args->len, expr->len - 1);
                        VL_Core_stack_dropn(self, args + 1);
                        VL_Core_stack_push_none(self);        
                    })
                CDEFAULT(
                    size_t args = VL_Core_num_args(self, fn_ptr);
                    VL_Core_error(self, VL_Expr_get(expr, 0));
                    VL_Object_perror(VL_Core_stack_get(self, fn_ptr));
                    printf(" is not a function!\n");
                    VL_Core_stack_dropn(self, args + 1);
                    VL_Core_stack_push_none(self);)
            }
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
        
        VL_Parser_init();
        if(VL_Module_parse(main, file_path)){
            VL_Core_eval_obj(self, self->scope_global, main->ast_tree);
        }
        VL_Parser_quit();
    }
    else{
        printf(VLT_ERR("Error") ": opening file ");
        VL_Str_print(file_path);
        printf("\n");
    }
}