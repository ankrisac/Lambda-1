#include "parser.h"

VL_SymMap* builtin_keywords = NULL;

void add_value(VL_SymMap* self, const char* name, VL_Object* val){
    VL_Str* temp_str = VL_Str_from_cstr(name);
    VL_SymMap_insert_cstr(self, temp_str, VL_Str_hash(temp_str), val);
    VL_Str_delete(temp_str);
}
void add_label(VL_SymMap* self, const char* name, VL_Keyword keyword){
    VL_Object obj;
    VL_Object_set_keyword(&obj, keyword);
    add_value(self, name, &obj);
}
void VL_Parser_init(){
    builtin_keywords = VL_SymMap_new(NULL, 1);

    #define CASE(KEYWORD) add_label(builtin_keywords, VL_KEYWORD_GET_REPR(KEYWORD), VL_KEYWORD_GET_ENUM(KEYWORD));
    VL_KEYWORD_MAPPING(CASE)
    #undef CASE

    add_label(builtin_keywords, "#Q", VL_KEYWORD_QUOTE);
    add_label(builtin_keywords, "#QQ", VL_KEYWORD_QUASIQUOTE);
    add_label(builtin_keywords, "#QU", VL_KEYWORD_UNQUOTE);
    add_label(builtin_keywords, "#QS", VL_KEYWORD_UNQUOTESPLICE);

    VL_Object temp;

    temp.type = VL_TYPE_NONE;
    add_value(builtin_keywords, "None", &temp);

    VL_Object_set_bool(&temp, true);
    add_value(builtin_keywords, "True", &temp);

    VL_Object_set_bool(&temp, false);
    add_value(builtin_keywords, "False", &temp);
    #undef ADD
}
void VL_Parser_quit(){
    VL_SymMap_delete(builtin_keywords);
    builtin_keywords = NULL;
}

#define TOKEN_SPACE(X)  \
    X(' ')  X('\n')     \
    X('\t') X('\r')     \
    X('\f') X('\b')

#define TOKEN_RESERVED(X) X('"')

#define TOKEN_SEP(X)    \
    X(',') X(';')

#define TOKEN_BRACKET(X)\
    X('(') X(')')       \
    X('[') X(']')       \
    X('{') X('}')

#define TOKEN_OPERATOR(X)       \
    X('=') X('<') X('>')        \
    X('%') X('$') X(':')        \
    X('!') X('|') X('&') X('^') \
    X('+') X('-') X('*') X('/') \

char VL_Module_match_escape_seq(char in){
    switch(in){
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'b': return '\b';
        default: return in;
    }
}
bool VL_Module_parse_token_symbol(char val){
    switch(val){
        #define C(X) case X:
        TOKEN_SPACE(C) TOKEN_SEP(C) TOKEN_BRACKET(C) TOKEN_RESERVED(C)
            return false;
        case '\0':
            return false;
        default:
            return true;
    }
}
bool VL_Module_parse_token_symbol_label(char val){
    switch(val){
        #define C(X) case X:
        TOKEN_SPACE(C) TOKEN_SEP(C) TOKEN_BRACKET(C) TOKEN_RESERVED(C) TOKEN_OPERATOR(C)
            return false;
        case '\0':
            return false;
        default:
            return true;
    }
}
bool VL_Module_parse_token_symbol_operator(char val){
    switch(val){
        #define C(X) case X:
        TOKEN_OPERATOR(C)
            return true;
        default:
            return false;
        #undef C
    }
}

bool VL_Module_parse_Number(VL_Module* self, VL_ParseState* state){
    VL_ParseState begin = *state;
    
    if(!VL_match_digit(VL_Module_peek(self, state))){
        VL_Module_push_err_str(self, &begin, state,
            "Expected " VLT_ERR("digit") 
            " of " VLT_ERR("int")
        );
        return false;
    }
    
    VL_Int Int = 0;
    for(char chr; VL_match_digit(chr = VL_Module_peek(self, state)); VL_Module_next(state, chr)){
        Int = Int * 10 + (VL_Int)(chr - '0');
    }
    
    if(!VL_Module_match_chr(self, state, '.')){
        state->val = VL_Object_wrap_int(Int);
        return true;
    }   
    if(!VL_match_digit(VL_Module_peek(self, state))){
        VL_Module_push_err_str(self, &begin, state,
            "Expected " VLT_ERR("digit")
            " after " VLT_ERR("period [.]") 
            " of " VLT_ERR("float")
        );
        return false;
    }

    VL_Float pos = 1, Frac = 0;
    for(char chr; VL_match_digit(chr = VL_Module_peek(self, state)); VL_Module_next(state, chr)){
        Frac += (pos *= 0.1)* (VL_Float)(chr - '0');
    }
    state->val = VL_Object_wrap_float((VL_Float)Int + Frac);
    
    return true;
}

bool VL_Module_parse_String(VL_Module* self, VL_ParseState* state){
    VL_ParseState begin = *state;

    if(!VL_Module_match_chr(self, state, '"')){    
        VL_Module_push_err_str(self, &begin, state, 
            "Expected opening " VLT_ERR("quote [\"]") 
            " of " VLT_ERR("string")
        );
    }

    VL_Str* string = VL_Str_new(0);
    bool escaped = false;
    char chr;
    
    while(true){
        chr = VL_Module_peek(self, state);
        if(!escaped){
            if(chr != '"' && chr != '\0'){
                if(chr != '\\'){
                    VL_Str_append_char(string, chr);
                }
                else{
                    escaped = true;
                }
            }
            else{
                break;
            }
        }
        else{
            VL_Str_append_char(string, VL_Module_match_escape_seq(chr));
            escaped = false;
        }   
        VL_Module_next(state, chr);
    }

    if(!VL_Module_match_chr(self, state, '"')){
        VL_Module_push_err_str(self, &begin, state, 
            "Expected closing " VLT_ERR("quote [\"]") 
            " of " VLT_ERR("string"));
        VL_Str_delete(string);
        return false;
    }
    
    state->val = VL_Object_wrap_str(string);
    return true;
}

bool VL_Module_parse_Label(VL_Module* self, VL_ParseState* state){
    VL_ParseState begin = *state;

    if(!VL_Module_parse_token_symbol_label(VL_Module_peek(self, state))){
        VL_Module_push_err_str(self, &begin, state,
            "Expected " VLT_ERR("Label")
        );
        return false;    
    }

    VL_Str* string = VL_Str_new(1);
    for(char chr; VL_Module_parse_token_symbol_label(chr = VL_Module_peek(self, state)); VL_Module_next(state, chr)){
        VL_Str_append_char(string, chr);
    }

    VL_Object* sym = VL_SymMap_find_str(builtin_keywords, string);;
    
    if(sym == NULL){
        state->val = VL_Object_wrap_symbol(VL_Symbol_new(string));
    }
    else{
        state->val = VL_Object_clone(sym);
        VL_Str_delete(string);
    }

    return true;
}
bool VL_Module_parse_Operator(VL_Module* self, VL_ParseState* state){
    char chr;
    VL_ParseState begin = *state;
    
    if(VL_Module_parse_token_symbol_operator(chr = VL_Module_peek(self, state))){
        VL_Str* string = VL_Str_new(1);

        VL_Str_append_char(string, chr);
        VL_Module_next(state, chr);

        for(; VL_Module_parse_token_symbol_operator(chr = VL_Module_peek(self, state)); VL_Module_next(state, chr)){
            VL_Str_append_char(string, chr);
        }

        state->val = VL_Object_wrap_symbol(VL_Symbol_new(string));
                
        return true;
    }

    VL_Module_push_err_str(self, &begin, state,
        "Expected " VLT_ERR("Operator")
    );
    return false;
}
bool VL_Module_parse_AExpr(VL_Module* self, VL_ParseState* begin, VL_ParseState* state){
    #define C(X) case X:
    
    VL_Module_next(state, '(');
    VL_Module_parse_Space(self, state);

    switch(VL_Module_peek(self, state)){
        TOKEN_OPERATOR(C) {
            if(VL_Module_parse_Operator(self, state)){
                VL_Module_parse_Space(self, state);
                if(!VL_Module_match_chr(self, state, ')')){
                    VL_Module_push_err_str(self, begin, state,
                        "Expected closing " VLT_ERR("parenthesis ')'") 
                        " of " VLT_ERR("α-expr")
                    );
                    VL_Object_delete(state->val);
                    return false;
                }
                return true;
            }
            return false;
        }
        default:{
            if(VL_Module_parse_IExpr(self, state)){
                VL_Module_parse_Space(self, state);
                if(!VL_Module_match_chr(self, state, ')')){
                    VL_Module_push_err_str(self, begin, state,
                        "Expected closing " VLT_ERR("parenthesis ')'") 
                        " of " VLT_ERR("α-expr")
                    );
                    VL_Object_delete(state->val);
                    return false;
                }
                return true;
            }
            return false;
        }
    }

    #undef C
}
bool VL_Module_parse_FAtom(VL_Module* self, VL_ParseState* state){
    #define C(X) case X:
    
    VL_ParseState begin = *state;
    switch(VL_Module_peek(self, state)){
        case '"':
            return VL_Module_parse_String(self, state);
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return VL_Module_parse_Number(self, state);
        case '(':
            return VL_Module_parse_AExpr(self, &begin, state);
        case '[':
            return VL_Module_parse_LExpr(self, state);
        case '{':
            return VL_Module_parse_BExpr(self, state);
        case ']':
        case '}':  
        case ';':
        case ',':
            return false;
        TOKEN_OPERATOR(C)
            return false;
        TOKEN_SPACE(C)
            return false;
        case '\0':
            return false;
        default:
            break;
    }
    #undef C

    return VL_Module_parse_Label(self, state);
}
bool VL_Module_parse_FExpr(VL_Module* self, VL_ParseState* state){
    VL_ParseState begin = *state;
    
    if(!VL_Module_parse_FAtom(self, state)){
        return false;
    }            

    VL_Expr* expr = VL_Expr_new(1);
    VL_Expr_append_Object(expr, state->val, begin.p, state->p, self->id);

    VL_ParseState temp = *state;
    size_t i = 0;
    while(true){
        VL_Module_parse_Space(self, &temp);
        //if(!VL_Module_parse_SpaceSep(self, &temp)){ break; }
        if(!VL_Module_parse_FAtom(self, &temp)){ break; }

        VL_Expr_append_Object(expr, temp.val, state->p, temp.p, self->id);
        *state = temp;
        i++;
    }

    if(i > 0){
        state->val = VL_Object_wrap_expr(expr);
    }
    else{
        state->val = VL_Expr_pop_Object(expr);
        VL_Expr_delete(expr);    
    }

    return true;
}

bool VL_Module_parse_LExpr(VL_Module* self, VL_ParseState* state){
    const VL_ParseState begin = *state;
    VL_Expr* expr = VL_Expr_new(0);

    if(!VL_Module_match_chr(self, state, '[')){    
        VL_Expr_delete(expr);
        VL_Module_push_err_str(self, &begin, state,
            "Expected opening " VLT_ERR("brackets '['")
            " of " VLT_ERR("λ-expr")
        );
        return false;
    }
            
    VL_Module_parse_Space(self, state);
    VL_ParseState temp = *state;

    if(VL_Module_parse_IExpr(self, &temp)){
        VL_Expr_append_Object(expr, temp.val, state->p, temp.p, self->id);
        *state = temp;
    
        while(true){
            VL_Module_parse_Space(self, state);

            temp = *state;
            if(!VL_Module_match_chr(self, &temp, ',')){
                break;
            }
                
            VL_Module_parse_Space(self, &temp);

            if(!VL_Module_parse_IExpr(self, &temp)){
                VL_Expr_delete(expr);
                VL_Module_push_err_str(self, &begin, state, 
                    "Expected " VLT_ERR("ι-expr")
                    " after " VLT_ERR("comma [,]") 
                    " in " VLT_ERR("λ-expr")
                );
                return false;
            }
                
            VL_Expr_append_Object(expr, temp.val, state->p, temp.p, self->id);
            *state = temp;
        }
    }

    if(VL_Module_match_chr(self, state, ']')){
        state->val = VL_Object_wrap_expr(expr);
        return true;
    }

    VL_Expr_delete(expr);
    VL_Module_push_err_str(self, &begin, state, 
        "Expected closing " VLT_ERR("brackets ']'")
        " of " VLT_ERR("λ-expr")
    );

    return false;
}
bool VL_Module_parse_BExpr(VL_Module* self, VL_ParseState* state){
    VL_ParseState begin = *state;

    if(!VL_Module_match_chr(self, state, '{')){
        VL_Module_push_err_str(self, &begin, state, 
            "Expected opening" VLT_ERR("braces '{'")         
            " of " VLT_ERR("β-expr")
        );
        return false;
    }

    VL_Expr* expr = VL_Expr_new(0);        
    VL_ParseState temp = *state;

    while(true){
        VL_Module_parse_Space(self, &temp);
        *state = temp;

        if(!VL_Module_parse_IExpr(self, &temp)){ break; }
        
        VL_Expr_append_Object(expr, temp.val, state->p, temp.p, self->id);
        *state = temp;

        VL_Module_parse_Space(self, &temp);

        if(!VL_Module_match_chr(self, &temp, ';')){
            VL_Module_push_err_str(self, &begin, state,
                "Expected " VLT_ERR("terminator [;]") 
                " of " VLT_ERR("ι-expr")
                " in " VLT_ERR("β-expr")
            );
            VL_Expr_delete(expr);                
            return false;
        }
        
        *state = temp;
    }

    VL_Module_parse_Space(self, state);
    if(!VL_Module_match_chr(self, state, '}')){
        VL_Module_push_err_str(self, &begin, state,
            "Expected closing " VLT_ERR("braces '}'") 
            " of " VLT_ERR("β-expr")
        );
        
        VL_Expr_delete(expr);
        return false;   
    }

    state->val = VL_Object_wrap_expr(expr);
    return true; 
}
bool VL_Module_parse_IExpr(VL_Module* self, VL_ParseState* state){
    const VL_ParseState begin = *state;
    
    VL_Expr* expr = VL_Expr_new(2);

    state->val = VL_Object_wrap_keyword(VL_KEYWORD_INFIX);
    VL_Expr_append_Object(expr, state->val, begin.p, state->p, self->id);
    
    size_t p_err = self->error_stack->len;    
    if(!VL_Module_parse_FExpr(self, state)){
        VL_Expr_delete(expr);

        if(p_err == self->error_stack->len){
            VL_Module_push_err_str(self, &begin, state,
                "Expected " VLT_ERR("Atom")
            );
        }
        else{

        }
        return false;
    }

    VL_ParseState temp = *state;
    VL_Expr_append_Object(expr, state->val, begin.p, state->p, self->id);
    
    while(true){
        if(!VL_Module_parse_SpaceSep(self, &temp)){ break; }

        p_err = self->error_stack->len; 
        if(!VL_Module_parse_Operator(self, &temp)){ 
            VL_Module_pop_errors(self, p_err, self->error_stack->len);
            break;
        }

        VL_Expr_append_Object(expr, temp.val, state->p, temp.p, self->id);

        *state = temp;

        if(!VL_Module_parse_SpaceSep(self, &temp)){
            VL_Expr_delete(expr);
            VL_Module_push_err_str(self, &begin, state,
                "Expected " VLT_ERR("space")
                " after " VLT_ERR("operator")
            );
            return false;
        }

        if(!VL_Module_parse_FExpr(self, &temp)){ 
            VL_Expr_delete(expr);
            VL_Module_push_err_str(self, &begin, state,
                "Expected " VLT_ERR("ι-atom")
                " after " VLT_ERR("operator")
            );
            return false;
        }

        VL_Expr_append_Object(expr, temp.val, state->p, temp.p, self->id);
        *state = temp;
    }

    if(expr->len > 2){
        state->val = VL_Object_wrap_expr(expr);
    }
    else{
        state->val = VL_Expr_pop_Object(expr);
        VL_Expr_delete(expr);
    }

    return true;
}
void VL_Module_parse_NExpr(VL_Module* self, VL_ParseState* state){
    VL_ParseState begin = *state;
    
    VL_Module_parse_Space(self, state);
    VL_Module_parse_IExpr(self, state);
    if(!state->ok){ return; }
    VL_Module_parse_Space(self, state);

    if(!VL_Module_match_chr(self, state, ';')){
        VL_Module_push_err_str(self, &begin, state,
            "Expected closing " VLT_ERR("terminator [;]")
            " of " VLT_ERR("η-expr")
        );
        VL_Object_delete(state->val);
    }
}
VL_ParseState VL_Module_parse_Line(VL_Module* self, VL_ParseState state){
    VL_ParseState temp = state;
    
    VL_Module_parse_Space(self, &temp);
        
    VL_Module_parse_IExpr(self, &temp);    
    if(!temp.ok){ return temp; }

    VL_Module_parse_Space(self, &temp);
    VL_Module_match_chr(self, &temp, '\0');
    return temp;
}
VL_ParseState VL_Module_parse_File(VL_Module* self, VL_ParseState state){
    VL_ParseState curr = state, prev = state;
    
    VL_Expr* expr = VL_Expr_new(1);
    VL_Expr* do_expr = VL_Expr_new(2);

    VL_Expr_append_Object(do_expr, VL_Object_wrap_keyword(VL_KEYWORD_DO), curr.p, curr.p, self->id);
    VL_Expr_append_Object(do_expr, VL_Object_wrap_expr(expr), curr.p, curr.p, self->id);

    while (true){
        VL_Module_parse_NExpr(self, &curr);

        if(!curr.ok){
            VL_Module_parse_Space(self, &prev);

            if(VL_Module_match_chr(self, &prev, '\0')){
                prev.val = VL_Object_wrap_expr(do_expr);
                return prev;
            }            
            
            VL_Expr_delete(do_expr);
            break;
        }    

        VL_Expr_append_Object(expr, curr.val, prev.p, curr.p, self->id);
        prev = curr;
    } 
    return curr;
}