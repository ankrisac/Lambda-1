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

void trace(VL_Core* self, const VL_ExprAtom* atom){
    VL_Module* mod = VL_ModuleList_get_module(self->modules, atom->module_id);
    VL_Module_print_state(mod, atom->begin, atom->end);
}
void error(VL_Core* self, const VL_ExprAtom* source){
    printf(VLT_ERR("Runtime Error: "));
    trace(self, source);
}

void stack_push(VL_Core* self, VL_Object* obj){
    VL_Tuple_append(self->stack, obj);
}
void stack_push_copy(VL_Core* self, const VL_Object* obj){
    VL_Tuple_append_copy(self->stack, obj);
}

void stack_push_none(VL_Core* self){
    VL_Tuple_append(self->stack, 
        &(VL_Object){ .type = VL_TYPE_NONE });
}
void stack_push_error(VL_Core* self, VL_Error error){
    VL_Tuple_append(self->stack, 
        &(VL_Object){ .type = VL_TYPE_ERROR, .data.err = error });
}
void stack_push_bool(VL_Core* self, VL_Bool val){
    VL_Tuple_append(self->stack, 
        &(VL_Object){ .type = VL_TYPE_BOOL, .data.v_bool = val });
}
void stack_push_int(VL_Core* self, VL_Int val){
    VL_Tuple_append(self->stack, 
        &(VL_Object){ .type = VL_TYPE_INT, .data.v_int = val });
}

VL_Object* VL_Core_stack_get(VL_Core* self, size_t i){
    return VL_Tuple_mget(self->stack, i);
}
VL_Object* VL_Core_stack_rget(VL_Core* self, size_t i){
    return VL_Tuple_mrget(self->stack, i);
}

void VL_Core_stack_pop_to(VL_Core* self, VL_Object* obj){
    VL_Tuple_pop_to(self->stack, obj);
}

void stack_drop(VL_Core* self){
    VL_Tuple_drop(self->stack);
}
void stack_dropn(VL_Core* self, size_t n){
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

#define DEF_FUNC(NAME) \
    void fn_##NAME(VL_Core* self, VL_Expr* expr)

#define FUNC_TYPECASE(TYPE_ENUM, EXPR)              \
    case VL_TYPE_GET_ENUM(TYPE_ENUM): { EXPR return; }

void keyword_error(VL_Core* self, const VL_Expr* expr, size_t n, VL_Error error){
    printf("\n");
    trace(self, VL_Expr_get(expr, n));
    stack_dropn(self, expr->len - 1);
    stack_push_error(self, error);
}

void keyword_unary_type_error(VL_Core* self, const VL_Expr* expr, VL_Keyword keyword, VL_Object* val){
    VL_Keyword_perror(keyword);                     
    
    printf(" not defined on ");
    VL_Object_perror(val);                          
    
    keyword_error(self, expr, 1, VL_ERROR_TYPE_ERROR);
}

#define UNARY_FUNC(NAME, KEYWORD, CASES)                \
    DEF_FUNC(NAME){                                     \
        VL_Object* val = VL_Core_stack_rget(self, 0);   \
        switch(val->type){                              \
            CASES                                       \
            default:                                    \
                keyword_unary_type_error(self, expr,    \
                    KEYWORD, val);                      \
                break;                                  \
        }                                               \
    }

void keyword_binary_type_error(VL_Core* self, const VL_Expr* expr, VL_Keyword keyword, VL_Object* lhs, VL_Object* rhs){
    VL_Keyword_perror(keyword);

    printf(" not defined on (");
    VL_Object_perror(lhs);
    printf(", ");
    VL_Object_perror(rhs);
    printf(")");

    keyword_error(self, expr, 0, VL_ERROR_TYPE_ERROR);
}
#define BINARY_FUNC(NAME, KEYWORD, CASES)               \
    DEF_FUNC(NAME){                                     \
        VL_Object* lhs = VL_Core_stack_rget(self, 1);   \
        VL_Object* rhs = VL_Core_stack_rget(self, 0);   \
        if(lhs->type == rhs->type){                     \
            switch(lhs->type){                          \
                CASES                                   \
                default:                                \
                    keyword_binary_type_error(self,     \
                        expr, KEYWORD, lhs, rhs);       \
                    break;                              \
            }                                           \
        }                                               \
        else{                                           \
            keyword_binary_type_error(self,             \
                expr, KEYWORD, lhs, rhs);               \
        }                                               \
    }
            
#define BINARY_CASE_NUM(TYPE_ENUM, TYPE_TAG, OP)    \
    FUNC_TYPECASE(TYPE_ENUM,                        \
        lhs->data.TYPE_TAG OP rhs->data.TYPE_TAG;   \
        rhs->type = VL_TYPE_NONE;                   \
        self->stack->len--;                         \
    )

DEF_FUNC(input){
    stack_push(self,
        &(VL_Object){ 
            .type = VL_TYPE_STRING, .data.str = VL_Str_from_cin() 
        }
    );
}
DEF_FUNC(time){
    stack_push(self, 
        &(VL_Object){ 
            .type = VL_TYPE_FLOAT, .data.v_float = clock()/CLOCKS_PER_SEC 
        }
    );
}
    
DEF_FUNC(seqget){
    VL_Object* index = VL_Core_stack_rget(self, 0);
    VL_Object* seq = VL_Core_stack_rget(self, 1);

    if(index->type == VL_TYPE_INT){
        VL_Int i = index->data.v_int;

        switch(seq->type){
            case VL_TYPE_RS_STRING: {
                VL_Str* str = &seq->data.arc->str;
                if(0 <= i && i < str->len){
                    char chr = str->data[i];
                    stack_dropn(self, 2);
                    stack_push(self, 
                        &(VL_Object){ .data.v_char = chr, .type = VL_TYPE_CHAR }
                    );
                }
                else if(-str->len <= i){
                    char chr = str->data[str->len + i];
                    stack_dropn(self, 2);
                    stack_push(self, 
                        &(VL_Object){ .data.v_char = chr, .type = VL_TYPE_CHAR }
                    );
                }
                else{
                    VL_Keyword_perror(VL_KEYWORD_SEQGET);
                    printf(" index [%lli] out of range [%zu]", i, str->len);
                    keyword_error(self, expr, 2, VL_ERROR_UNDEFINED);
                }
                break;
            }
            case VL_TYPE_RS_TUPLE: {
                VL_Tuple* tuple = &seq->data.arc->tuple;
                if(0 <= i && i < tuple->len){
                    VL_Object* elem = &tuple->data[i];
                    stack_dropn(self, 2);
                    stack_push_copy(self, elem);
                }
                else if(-tuple->len <= i){
                    VL_Object* elem = &tuple->data[tuple->len + i];
                    stack_dropn(self, 2);
                    stack_push_copy(self, elem);
                }
                else{
                    VL_Keyword_perror(VL_KEYWORD_SEQGET);
                    printf(" index [%lli] out of range [%zu]", i, tuple->len);
                    keyword_error(self, expr, 2, VL_ERROR_UNDEFINED);
                }
            }
            default:
                VL_Keyword_perror(VL_KEYWORD_SEQGET);
                printf(" not defined on ");
                VL_Object_perror(seq);
                
                keyword_error(self, expr, 1, VL_ERROR_UNDEFINED);
                break;
        }
    }
    else{
        VL_Keyword_perror(VL_KEYWORD_SEQGET);
        printf(" index must be an integer, not ");
        VL_Object_perror(index);
        keyword_error(self, expr, 2, VL_ERROR_UNDEFINED);
    }
}
DEF_FUNC(seqset){
    VL_Object* val = VL_Core_stack_rget(self, 0);
    VL_Object* index = VL_Core_stack_rget(self, 1);
    VL_Object* seq = VL_Core_stack_rget(self, 2);

    if(index->type == VL_TYPE_INT){
        VL_Int i = index->data.v_int;

        switch(seq->type){
            case VL_TYPE_RS_STRING: {
                VL_Str* str = &seq->data.arc->str;
                if(i < str->len){
                    if(val->type == VL_TYPE_CHAR){
                        str->data[i] = val->data.v_char;
                        stack_dropn(self, 3);
                        stack_push_none(self);
                    }
                    else{
                        VL_Keyword_perror(VL_KEYWORD_SEQGET);
                        printf(" value must be ");
                        VL_Type_perror(VL_TYPE_CHAR);
                        printf(", not ");
                        VL_Object_perror(val);
                        keyword_error(self, expr, 3, VL_ERROR_TYPE_ERROR);
                    }
                }
                else{
                    VL_Keyword_perror(VL_KEYWORD_SEQGET);
                    printf(" index [%lli] out of range [%zu]", i, str->len);
                    keyword_error(self, expr, 2, VL_ERROR_UNDEFINED);
                }
                break;
            }
            case VL_TYPE_RS_TUPLE: {
                VL_Tuple* tuple = &seq->data.arc->tuple;
                if(0 <= i && i < tuple->len){
                    tuple->data[i] = *val;
                    self->stack->len -= 2;
                    stack_drop(self);
                    stack_push_none(self);
                }
                else if(-tuple->len <= i){
                    tuple->data[tuple->len + i] = *val;
                    self->stack->len -= 2;
                    stack_drop(self);
                    stack_push_none(self);
                }
                else{
                    VL_Keyword_perror(VL_KEYWORD_SEQGET);
                    printf(" index [%lli] out of range [%zu]", i, tuple->len);
                    keyword_error(self, expr, 2, VL_ERROR_UNDEFINED);
                }
                break;
            }
            default:
                VL_Keyword_perror(VL_KEYWORD_SEQGET);
                printf(" not defined for ");
                VL_Type_perror(seq->type);
                keyword_error(self, expr, 1, VL_ERROR_TYPE_ERROR);
                break;
        }
    }
    else{
        VL_Keyword_perror(VL_KEYWORD_SEQGET);
        printf(" index must be an integer, not ");
        VL_Object_perror(index);
        keyword_error(self, expr, 2, VL_ERROR_TYPE_ERROR);
    }
}

UNARY_FUNC(seqlen, VL_KEYWORD_SEQLEN, 
    FUNC_TYPECASE(STRING, 
        size_t len = val->data.str->len;
        stack_drop(self);
        stack_push_int(self, len);
    )
    FUNC_TYPECASE(RS_STRING, 
        size_t len = val->data.arc->str.len;
        stack_drop(self);
        stack_push_int(self, len);
    )
    FUNC_TYPECASE(RS_TUPLE, 
        size_t len = val->data.arc->tuple.len;
        stack_drop(self);
        stack_push_int(self, len);
    )
    FUNC_TYPECASE(EXPR, 
        size_t len = val->data.expr->len;
        stack_drop(self);
        stack_push_int(self, len);
    )
)

UNARY_FUNC(not, VL_KEYWORD_NOT, 
    FUNC_TYPECASE(BOOL, 
        val->data.v_bool = !val->data.v_bool;
    )
)

UNARY_FUNC(char, VL_KEYWORD_CHAR, 
    FUNC_TYPECASE(STRING,
        if(val->data.str->len == 1){
            char chr = (VL_Char)val->data.str->data[0];
            VL_Str_delete(val->data.str);
            val->data.v_char = chr;
            val->type = VL_TYPE_CHAR;
        }
        else{
            stack_drop(self);
            stack_push_error(self, VL_ERROR_TYPE_ERROR);
        }
    )
    FUNC_TYPECASE(INT, 
        val->data.v_char = (VL_Char)val->data.v_int;
        val->type = VL_TYPE_CHAR;
    )
    FUNC_TYPECASE(CHAR, )
)

UNARY_FUNC(int, VL_KEYWORD_INT, 
    FUNC_TYPECASE(FLOAT, 
        val->data.v_int = (VL_Int)val->data.v_float;
        val->type = VL_TYPE_INT;
    )
    FUNC_TYPECASE(INT, )
)

UNARY_FUNC(float, VL_KEYWORD_FLOAT, 
    FUNC_TYPECASE(INT, 
        val->data.v_float = (VL_Float)val->data.v_int;
        val->type = VL_TYPE_FLOAT;
    )
    FUNC_TYPECASE(FLOAT, )
)

UNARY_FUNC(string, VL_KEYWORD_STRING, 
    FUNC_TYPECASE(STRING,
        VL_Str* str = val->data.str; 
        val->data.arc = VL_ARCData_malloc();
        val->data.arc->str = *str;
        val->type = VL_TYPE_RS_STRING;
        free(str);
    )
)

#define DEF(NAME, KEYWORD, OP)                      \
    BINARY_FUNC(NAME, VL_KEYWORD_GET_ENUM(KEYWORD), \
        BINARY_CASE_NUM(INT, v_int, OP)             \
        BINARY_CASE_NUM(FLOAT, v_float, OP)         \
    )
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
            VL_Keyword_perror(VL_KEYWORD_DIV);
            printf(": ");
            VL_Object_perror(lhs);
            printf("/");
            VL_Object_perror(rhs);
            printf(" division by zero");
            keyword_error(self, expr, 2, VL_ERROR_UNDEFINED);
        }
    )
)

#define DEF(NAME, KEYWORD, OP)                      \
    BINARY_FUNC(NAME, VL_KEYWORD_GET_ENUM(KEYWORD), \
        BINARY_CASE_NUM(BOOL, v_bool, OP)           \
    )
DEF(and, AND, &=)
DEF(or, OR, |=)
#undef DEF

#undef CASE_NUM


#define CMP_NUM(TYPE_ENUM, TYPE_TAG, OP)                                \
    FUNC_TYPECASE(TYPE_ENUM,                                            \
        lhs->data.v_bool = (lhs->data.TYPE_TAG OP rhs->data.TYPE_TAG);  \
        lhs->type = VL_TYPE_BOOL;                                       \
        self->stack->len--;)        

#define CMP_STR(OP)                                                 \
    FUNC_TYPECASE(STRING,                                           \
        bool ok = (VL_Str_cmp(lhs->data.str, rhs->data.str) OP 0);  \
        stack_dropn(self, 2);                               \
        stack_push_bool(self, ok);)

#define DEF(NAME, KEYWORD, OP)                      \
    BINARY_FUNC(NAME, VL_KEYWORD_GET_ENUM(KEYWORD), \
        CMP_NUM(INT, v_int, OP)                     \
        CMP_NUM(FLOAT, v_float, OP)                 \
        CMP_STR(OP)                                 \
    )

DEF(lt,     LT,     <)
DEF(lte,    LTE,    <=)
DEF(gt,     GT,     >)
DEF(gte,    GTE,    >=)
#undef DEF

#define DEF(NAME, KEYWORD, OP)                          \
    DEF_FUNC(NAME){                                     \
        VL_Object* lhs = VL_Core_stack_rget(self, 1);   \
        VL_Object* rhs = VL_Core_stack_rget(self, 0);   \
        if(lhs->type == rhs->type){                     \
            switch(lhs->type){                          \
                case VL_TYPE_NONE:                      \
                    lhs->data.v_bool = (0 OP 0);        \
                    lhs->type = VL_TYPE_BOOL;           \
                    self->stack->len--;                 \
                    break;                              \
                CMP_NUM(BOOL, v_bool, OP)               \
                CMP_NUM(INT, v_int, OP)                 \
                CMP_NUM(FLOAT, v_float, OP)             \
                CMP_STR(OP)                             \
                default:                                \
                    stack_dropn(self, 2);               \
                    stack_push_bool(self, false);       \
                    break;                              \
            }                                           \
        }                                               \
        else{                                           \
            stack_dropn(self, 2);                       \
            stack_push_bool(self, false);               \
        }                                               \
    }

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
        switch(val->type){
            case VL_TYPE_RS_TUPLE:
                val->type = VL_TYPE_RW_TUPLE;
                break;
            default:
                break;
        }

        stack_push_copy(self, val);
        return true;
    }
    stack_push_error(self, VL_ERROR_SYMBOL_UNDEFINED);
    return false;
}

size_t VL_Core_num_args(const VL_Core* self, size_t fn_ptr){
    if(self->stack->len > fn_ptr){
        return self->stack->len - fn_ptr - 1;
    }
    return 0;
}

bool macro_expand(VL_Core* self, VL_SymMap* env, VL_Object* ast){
    if(ast->type != VL_TYPE_EXPR){ return false; }                             

    VL_Expr* expr = ast->data.expr;                                     
    if(expr->len == 0){ return false; }                                        

    VL_Object* head = expr->data[0].val;                                
    if(head->type != VL_TYPE_SYMBOL){ return false; } 

    if(env == NULL){ printf("Unexpected empty environment\n"); return false; }
    
    VL_Object* fn_obj = VL_SymMap_find(env, head->data.symbol);   
    if(fn_obj == NULL || fn_obj->type != VL_TYPE_RS_FUNCTION){ return false; } 

    VL_Function* fn = fn_obj->data.fn;                                  
    if(!fn->is_macro){ return false; }

    if(fn->args->len + 1 != expr->len){                                
        VL_Object_perror(VL_Expr_get(expr, 0)->val);                   
        printf(" macro expected %zu argument(s), not %zu!\n",           
            fn->args->len, expr->len - 1);                              
        error(self, VL_Expr_get(expr, 0));                     
        return false;                                                         
    }                 

    VL_SymMap* new_env = VL_SymMap_new(env, 4);                         
    VL_Object temp;                                                     
    
    for(size_t i = 0; i + 1 < expr->len; i++){                          
        VL_Object_copy(&temp, VL_Expr_get(expr, i + 1)->val);   
        VL_SymMap_insert(new_env, VL_Function_getArg(fn, i), &temp);    
    }                                                              
    
    VL_Core_eval_obj(self, new_env, fn->body);                          
    VL_Object_clear(ast);                                               
    *ast = *VL_Core_stack_rget(self, 0);                                
    self->stack->len--;                                                 
    
    VL_SymMap_delete(new_env);

    return true;
}

void VL_Core_macro_expandn(VL_Core* self, VL_SymMap* env, VL_Object* ast, size_t n){
    for(size_t i = 0; macro_expand(self, env, ast) && i < n; i++);
}
void VL_Core_macro_expand(VL_Core* self, VL_SymMap* env, VL_Object* ast){
    while(macro_expand(self, env, ast));                                         
}

VL_Expr* VL_Core_eval_quasiquote(VL_Core* self, VL_SymMap* env, const VL_Expr* expr){
    #define DEFER(CONDITION)                \
        if(CONDITION){                      \
            VL_Expr_append_Object(out,      \
                VL_Object_clone(pair_obj),  \
                atom->begin, atom->end,     \
                atom->module_id);           \
            continue;                       \
        }
    VL_Expr* out = VL_Expr_new(expr->len);

    for(size_t i = 0; i < expr->len; i++){
        const VL_ExprAtom* atom = VL_Expr_get(expr, i);
        const VL_Object* pair_obj = atom->val;
        DEFER(pair_obj->type != VL_TYPE_EXPR)

        const VL_Expr* pair_expr = pair_obj->data.expr;
        if(pair_expr->len != 2){
            VL_Expr_append_Object(out, 
                VL_Object_wrap_expr(VL_Core_eval_quasiquote(self, env, pair_expr)),
                atom->begin, atom->end, atom->module_id
            );
            continue;
        }

        const VL_Object* pair_head = VL_Expr_get(pair_expr, 0)->val;
        const VL_ExprAtom* pair_atom = VL_Expr_get(pair_expr, 1);            
        if(pair_head->type != VL_TYPE_KEYWORD){
            VL_Expr_append_Object(out,
                VL_Object_wrap_expr(VL_Core_eval_quasiquote(self, env, pair_expr)),
                atom->begin, atom->end, atom->module_id
            );
            continue;
        }

        switch(pair_head->data.keyword){
            CKEYWORD(UNQUOTE,
                VL_Core_eval_obj(self, env, pair_atom->val);
                VL_Expr_append_Object(out, VL_Tuple_pop(self->stack), 
                    pair_atom->begin, pair_atom->end, pair_atom->module_id);)
            CKEYWORD(UNQUOTESPLICE,
                VL_Core_eval_obj(self, env, pair_atom->val);    
                VL_Object splice_obj;
                VL_Core_stack_pop_to(self, &splice_obj);
                if(splice_obj.type == VL_TYPE_EXPR){
                    VL_Expr_mappend_expr(out, splice_obj.data.expr);
                    splice_obj.data.expr->len = 0;
                    VL_Expr_delete(splice_obj.data.expr);
                }
                else{
                    VL_Object_clear(&splice_obj);
                    VL_Keyword_perror(VL_KEYWORD_UNQUOTESPLICE);
                    printf(" expected argument to be expr\n");
                    error(self, VL_Expr_get(pair_expr, 1));
                })
            CKEYWORD(QUASIQUOTE,
                VL_Expr_append_Object(out, 
                    VL_Object_clone(pair_obj),
                    atom->begin, atom->end,
                    atom->module_id
                );
            )
            default:
                VL_Expr_append_Object(out, 
                    VL_Object_wrap_expr(VL_Core_eval_quasiquote(self, env, pair_expr)),
                    atom->begin, atom->end,
                    atom->module_id
                );
                break;
        }   
    }
    #undef DEFER

    return out;
}

#define VL_COMMA ,

#include <assert.h>


VL_Object* VL_Core_eval_do(VL_Core* self, VL_SymMap* env, const VL_Object* obj){
    switch(obj->type){                                              
        case VL_TYPE_EXPR: { 
            const VL_Expr* expr = obj->data.expr; 

            if(expr->len > 0){
                for(size_t i = 0; (i + 1) < expr->len; i++){
                    VL_Core_eval_obj(self, env, VL_Expr_get(expr, i)->val);
                    
                    if(VL_Core_stack_rget(self, 0)->type == VL_TYPE_ERROR){
                        return NULL;
                    }
                    stack_drop(self);
                }
                return VL_Expr_rget(expr, 0)->val;
            }
            break;
        }
        case VL_TYPE_SYMBOL: {                                               
            if(!VL_Core_eval_symbol(self, env, obj->data.symbol)){  
                printf(VLT_ERR("Symbol Error:") " Cannot find ");   
                VL_Object_perror(obj);                              
                printf("\n"); 
            }
            break;
        }                           
        default:{                                                    
            stack_push_copy(self, obj);                             
            break;                                                 
        }
    }
    return NULL;
}

const size_t stack_limit = 20;

VL_Object* VL_Core_eval_ast(VL_Core* self, VL_SymMap* env, VL_Object* ast){
    #define KEYWORD_ERROR(KEYWORD, N, MSG)                  \
        VL_Keyword_perror(VL_KEYWORD_GET_ENUM(KEYWORD));    \
        printf(MSG);                                        \
        error(self, VL_Expr_get(expr, N));                  \
        stack_push_error(self, VL_ERROR_TYPE_ERROR);    

    #define SPECIAL_FORM(KEYWORD, N, EXPR)                      \
        CKEYWORD(KEYWORD,                                       \
            if(expr->len == N + 1){                             \
                EXPR                                            \
            }                                                   \
            else{                                               \
                printf(VLT_ERR("Error:"));                      \
                VL_Keyword_perror(VL_KEYWORD_GET_ENUM(KEYWORD));\
                printf(" expected "#N" argument(s)!");          \
                printf(" not %zu\n", expr->len - 1);            \
                trace(self, VL_Expr_get(expr, 0));              \
                stack_push_error(self,                          \
                    VL_ERROR_ARG_MISMATCH);                     \
            }                                                   \
        )

    #define SFORM_FUNCTION(KEYWORD, IS_MACRO)                       \
        SPECIAL_FORM(KEYWORD, 2,                                    \
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
                stack_push(self, &obj_fn);                          \
            }                                                       \
            else{                                                   \
                VL_Keyword_perror(VL_KEYWORD_GET_ENUM(KEYWORD));    \
                printf(" first argument must be argument list!\n"); \
                error(self, VL_Expr_get(expr, 1));                  \
                stack_push_error(self, VL_ERROR_TYPE_ERROR);        \
            }                                                       \
            return NULL;                                            \
        )

    #define SFORM_TYPECHECK(KEYWORD, N, OBJECT_TYPE, TYPE_ENUM, EXPR)   \
        if(OBJECT_TYPE == VL_TYPE_GET_ENUM(TYPE_ENUM)){                 \
            EXPR                                                        \
        }                                                               \
        else{                                                           \
            VL_Keyword_perror(VL_KEYWORD_GET_ENUM(KEYWORD));            \
            printf(" requires argument to be ");                        \
            VL_Type_perror(VL_TYPE_GET_ENUM(TYPE_ENUM));                \
            printf("\n");                                               \
            trace(self, VL_Expr_get(expr, N));                          \
            stack_push_error(self, VL_ERROR_TYPE_ERROR);                \
        }
    
    bool tail_call;
    do{
        tail_call = false;

        switch(ast->type){
            case VL_TYPE_EXPR:{
                const VL_Expr* expr = ast->data.expr;
                VL_Object* special_head = VL_Expr_get(expr, 0)->val; 
            
                if(special_head->type == VL_TYPE_KEYWORD){
                    switch(special_head->data.keyword){
                        SPECIAL_FORM(DO, 1, 
                            ast = VL_Core_eval_do(self, env, VL_Expr_get(expr, 1)->val);
                            if(ast != NULL){
                                tail_call = true;
                            }
                            else{
                                return NULL;
                            }
                        )
                        SPECIAL_FORM(MACROEXPAND, 2, 
                            VL_Object* n_expand = VL_Expr_get(expr, 1)->val;
                            VL_Object macro_expr;
                            VL_Object_copy(&macro_expr, VL_Expr_get(expr, 2)->val);
                            
                            SFORM_TYPECHECK(MACROEXPAND, 1, n_expand->type, INT, 
                                SFORM_TYPECHECK(MACROEXPAND, 2, macro_expr.type, EXPR,
                                    VL_Core_macro_expandn(self, env, &macro_expr, n_expand->data.v_int);
                                    stack_push(self, &macro_expr);
                                )
                            )
                            return NULL;
                        )
                        SPECIAL_FORM(QUOTE, 1, 
                            stack_push_copy(self, VL_Expr_get(expr, 1)->val);
                            return NULL;
                        )
                        SPECIAL_FORM(QUASIQUOTE, 1,
                            VL_Object* obj_expr = VL_Expr_get(expr, 1)->val;
                            switch(obj_expr->type){
                                CTYPE(EXPR,
                                    VL_Object out_expr;
                                    VL_Object_set_expr(&out_expr, 
                                        VL_Core_eval_quasiquote(self, env, obj_expr->data.expr));
                                    stack_push(self, &out_expr);
                                )
                                default:
                                    KEYWORD_ERROR(QUASIQUOTE, 1, 
                                        " requires argument to be an expression!\n"
                                    )
                            }
                            return NULL;
                        )
                        SPECIAL_FORM(SET, 2,
                            VL_Object* label = VL_Expr_get(expr, 1)->val;

                            SFORM_TYPECHECK(SET, 1, label->type, SYMBOL,
                                VL_Core_eval_obj(self, env, VL_Expr_get(expr, 2)->val);

                                VL_Object val; 
                                VL_Core_stack_pop_to(self, &val);

                                VL_SymMap_insert(env, label->data.symbol, &val);
                                stack_push_none(self);
                            )
                            return NULL;
                        )
                        case VL_KEYWORD_IF: {
                            VL_Object cond;
                            VL_Core_eval_obj(self, env, VL_Expr_get(expr, 1)->val);
                            VL_Core_stack_pop_to(self, &cond);
                            
                            switch(expr->len){
                                case 3:
                                    SFORM_TYPECHECK(IF, 1, cond.type, BOOL,
                                        if(cond.data.v_bool){
                                            ast = VL_Expr_get(expr, 2)->val;
                                            tail_call = true;
                                        }
                                        else{
                                            return NULL;
                                        }
                                    )
                                    break;
                                case 4:               
                                    SFORM_TYPECHECK(IF, 1, cond.type, BOOL,
                                        ast = VL_Expr_get(expr, (cond.data.v_bool) ? 2 : 3)->val;
                                        tail_call = true;
                                    )
                                    break;
                                default:
                                    KEYWORD_ERROR(IF, 0, " expected only 2-3 arguments!\n")
                                    return NULL;
                            }
                            break;
                        }
                        SPECIAL_FORM(WHILE, 2,
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
                                            stack_drop(self);
                                            loop = true;   
                                        }
                                    )
                                    default:
                                        break;
                                }
                            } while(loop);
                            stack_push_none(self);
                            return NULL;
                        )
                        SFORM_FUNCTION(MACRO, true)
                        SFORM_FUNCTION(FN, false)
                        default:{
                            for(size_t i = 0; i < expr->len; i++){
                                VL_Core_eval_obj(self, env, VL_Expr_get(expr, i)->val);
                            }
                            return ast; 
                        }
                    }
                }
                else{
                    for(size_t i = 0; i < expr->len; i++){
                        VL_Core_eval_obj(self, env, VL_Expr_get(expr, i)->val);
                    } 
                }
                break;
            }
            case VL_TYPE_SYMBOL:{                                      
                if(!VL_Core_eval_symbol(self, env, ast->data.symbol)){  
                    printf(VLT_ERR("Symbol Error:") " Cannot find ");   
                    VL_Object_perror(ast);                              
                    printf("\n"); 
                }
                return NULL;     
            }    
            default:{
                stack_push_copy(self, ast);                   
                return NULL;                                            
            }
        }
    }while(tail_call);

    return ast;
} 

void VL_Core_eval_obj(VL_Core* self, VL_SymMap* env, VL_Object* ast){
    #define EVAL_CASE(KEYWORD, N, FUNCTION)                     \
        CKEYWORD(KEYWORD,                                       \
            size_t args = VL_Core_num_args(self, fn_ptr);       \
            if(args == N){                                      \
                fn_##FUNCTION(self, expr);                      \
            }                                                   \
            else{                                               \
                VL_Keyword_perror(VL_KEYWORD_##KEYWORD);        \
                printf(" expected only "#N" argument(s)");      \
                printf(" not %zu\n", args);                     \
                keyword_error(self, expr,                       \
                    0, VL_ERROR_ARG_MISMATCH);                  \
            }                                                   \
        )    
    
    bool tail_call;

    VL_Core_macro_expand(self, env, ast);
    VL_Expr* expr = ast->data.expr;

    size_t fn_ptr = self->stack->len;
    ast = VL_Core_eval_ast(self, env, ast);                

    while(true){
        if(ast == NULL){
            break;
        }

        tail_call = false;
        VL_Object* head = VL_Core_stack_get(self, fn_ptr);

        switch(head->type){
            case VL_TYPE_KEYWORD: {
                switch(head->data.keyword){
                    EVAL_CASE(ADD, 2, add)  EVAL_CASE(SUB, 2, sub)
                    EVAL_CASE(MUL, 2, mul)  EVAL_CASE(DIV, 2, div)
        
                    EVAL_CASE(AND, 2, and)  EVAL_CASE(OR, 2, or)
                    EVAL_CASE(NOT, 1, not)

                    EVAL_CASE(EQ, 2, eq)    EVAL_CASE(NEQ, 2, neq)
                    EVAL_CASE(LT, 2, lt)    EVAL_CASE(LTE, 2, lte)
                    EVAL_CASE(GT, 2, gt)    EVAL_CASE(GTE, 2, gte)
                    
                    EVAL_CASE(INT, 1, int)  EVAL_CASE(FLOAT, 1, float)
                    EVAL_CASE(CHAR, 1, char)
                    EVAL_CASE(STRING, 1, string)
                    
                    EVAL_CASE(SEQGET, 2, seqget)
                    EVAL_CASE(SEQSET, 3, seqset)
                    EVAL_CASE(SEQLEN, 1, seqlen)

                    EVAL_CASE(INPUT, 0, input)
                    EVAL_CASE(TIME, 0, time)

                    case VL_KEYWORD_PRINT:{       
                        size_t args = VL_Core_num_args(self, fn_ptr); 
                    
                        for(size_t i = 0; i < args; i++){
                            VL_Object_print(VL_Core_stack_get(self, fn_ptr + i + 1));
                        }
                        
                        stack_dropn(self, args);
                        stack_push_none(self);
                        break;
                    }
                    case VL_KEYWORD_TUPLE:{
                        size_t args = VL_Core_num_args(self, fn_ptr);
                        
                        VL_Tuple tuple;
                        VL_Tuple_init(&tuple, args);
                        for(size_t i = 0; i < args; i++){
                            VL_Tuple_append(&tuple, VL_Core_stack_get(self, fn_ptr + i + 1));
                        }
                        self->stack->len -= args;
                        
                        VL_Object obj_tuple;
                        obj_tuple.data.arc = VL_ARCData_malloc();
                        obj_tuple.data.arc->tuple = tuple;
                        obj_tuple.type = VL_TYPE_RS_TUPLE;
                        
                        stack_push(self, &obj_tuple);
                        break;
                    }
                    default: {
                        size_t args = VL_Core_num_args(self, fn_ptr);
            
                        printf("Undefined keyword ");
                        VL_Object_perror(VL_Core_stack_get(self, fn_ptr));
                        
                        error(self, VL_Expr_get(expr, 0));
                        stack_dropn(self, args + 1);
                        stack_push_error(self, VL_ERROR_ARG_MISMATCH);       
                        return;
                    }
                }

                VL_Object* ret = VL_Core_stack_rget(self, 0);
                VL_Object* fn = VL_Core_stack_rget(self, 1);
                *fn = *ret;
                self->stack->len--;
                break;
            }
            case VL_TYPE_RS_FUNCTION: {
                VL_Function* fn = &head->data.arc->fn;

                if(fn->args->len + 1 == expr->len){ 
                    VL_SymMap* new_env = VL_SymMap_new(env, 4);

                    for(size_t i = 0; i + 1 < expr->len; i++){
                        VL_SymMap_insert(new_env, VL_Function_getArg(fn, i), 
                            VL_Core_stack_get(self, fn_ptr + i + 1));
                    }

                    self->stack->len = self->stack->len - expr->len + 1;

                    ast = VL_Core_eval_ast(self, new_env, fn->body);
                    VL_SymMap_delete(new_env);
                    
                    VL_Object_clear(VL_Core_stack_get(self, fn_ptr));
                    for(size_t i = fn_ptr + 1; i < self->stack->len; i++){
                        self->stack->data[i - 1] = self->stack->data[i];
                    }
                    self->stack->len--;
                    
                    tail_call = true;
                }
                else{
                    size_t args = VL_Core_num_args(self, fn_ptr);
                    
                    VL_Object_perror(VL_Core_stack_get(self, fn_ptr));
                    printf(" cannot be called as a function");

                    error(self, VL_Expr_get(expr, 0));
                    stack_dropn(self, args + 1);
                    stack_push_error(self, VL_ERROR_TYPE_ERROR);       
                }
                break;
            }
            default: {
                size_t args = VL_Core_num_args(self, fn_ptr);
                
                VL_Object_perror(VL_Core_stack_get(self, fn_ptr));
                printf(" is not defined ");
                error(self, VL_Expr_get(expr, 0));
                stack_dropn(self, args + 1);
                stack_push_error(self, VL_ERROR_UNDEFINED);       
                break;
            }
        }

        if(!tail_call){
            break;
        }
    }
}
void VL_Core_eval(VL_Core* self, VL_SymMap* env){
    VL_Core_eval_obj(self, env, VL_Core_stack_get(self, 0));
}

void VL_Core_exec_file(VL_Core* self, const VL_Str* file_path){
    VL_Module* main = VL_ModuleList_add_modulefile(self->modules, file_path);
    
    if(main != NULL){
        main->id = 0;
        
        VL_Parser_init();
        if(VL_Module_parse_file(main, main->file_path)){
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
VL_Module* VL_ModuleList_add_module(VL_ModuleList* self, VL_Str* input){
    if(self->len >= self->reserve_len){
        self->reserve_len *= 2;
        self->data = realloc(self->data, self->reserve_len * sizeof* self->data);
    }

    VL_Module obj;
    obj.ast_tree = NULL;
    obj.error_stack = VL_Tuple_new(0);
    obj.source = input;
    obj.id = self->len;

    obj.file_path = VL_Str_from_cstr(VLT_BLU VLT_BOLD "λ[");
    VL_Str_append_int(obj.file_path, self->len);
    VL_Str_append_cstr(obj.file_path, "]" VLT_RESET);
    
    self->data[self->len] = obj;
    self->len++;
    
    return &self->data[self->len - 1];
}
void VL_Core_repl(VL_Core* self){
    VL_Parser_init();

    printf(VLT_BLU VLT_BOLD "Viper [0.1]" VLT_RESET "\n\n");    
    while(true){
        printf(VLT_BLU VLT_BOLD "λ[%zu]: " VLT_RESET, self->modules->len);
        VL_Module* main = VL_ModuleList_add_module(self->modules, VL_Str_from_cin());

        if(VL_Str_cmp_cstr(main->source, "quit") == 0){
            VL_Str_delete(main->source);
            main->source = NULL;
            break;
        }
        else if(VL_Str_cmp_cstr(main->source, "@stack") == 0){
            VL_Core_stack_debug(self);
        }
        else if(VL_Str_cmp_cstr(main->source, "@global") == 0){
            printf("Globals: ");
            VL_SymMap_print(self->scope_global);
            printf("\n");
        }
        else{
            if(VL_Module_parse_line(main, main->source)){
                VL_Core_eval_obj(self, self->scope_global, main->ast_tree);

                VL_Object ret;
                VL_Core_stack_pop_to(self, &ret);
                    
                if(ret.type != VL_TYPE_NONE){
                    printf(VLT_BLU "[");
                    VL_Type_print(ret.type);
                    printf("] => " VLT_RESET);   
                    
                    VL_Object_repr(&ret);
                    VL_Object_clear(&ret);
                    printf("\n");
                }
            }
            else{
                printf(VLT_ERR("Syntax Error:") "\n");
                VL_Module_print_error_stack(main);
                VL_Tuple_dropn(main->error_stack, main->error_stack->len);
            }
        }
    }
 
    VL_Parser_quit();
}