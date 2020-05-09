#include "parser.h"

#define TOKEN_SPACE(X) X(' ') X('\n') X('\t') X('\r') X('\f') X('\b')
#define TOKEN_SEP(X) X(',') X(';') X(':')
#define TOKEN_BRACKET(X) X('(') X(')') X('[') X(']') X('{') X('}')
#define TOKEN_OPERATOR(X)           \
X('=') X('<') X('>')                \
X('@') X('$') X('%') X('#') X('?')  \
X('!') X('|') X('&') X('^') X('~')  \
X('+') X('-') X('*') X('/')         \

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
        TOKEN_SPACE(C) TOKEN_SEP(C) TOKEN_BRACKET(C)
            return false;
        default:
            return true;
    }
}
bool VL_Module_parse_token_symbol_label(char val){
    switch(val){
        #define C(X) case X:
        TOKEN_SPACE(C) TOKEN_SEP(C) TOKEN_BRACKET(C) TOKEN_OPERATOR(C)
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
        state->val = VL_Object_from_int(Int);
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
    state->val = VL_Object_from_float((VL_Float)Int + Frac);
    
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

    for(char chr; VL_Module_parse_token_symbol(chr = VL_Module_peek(self, state)); VL_Module_next(state, chr)){
        VL_Str_append_char(string, chr);
    }

    state->val = NULL;
    #define S(STR, SYM)                                     \
    if(VL_Str_cmp_cstr(string, STR) == 0){                  \
        state->val = VL_Object_from_symbol(VL_SYM_ ## SYM); \
    }
    switch(string->len){
        case 2:{
            S("do", DO)
            break;
        }
        case 3:{
            S("int", INT)
            else S("not", NOT)
            break;
        }
        case 4:{
            if(VL_Str_cmp_cstr(string, "True") == 0){
                state->val = VL_Object_from_bool(true);
            }
            else if(VL_Str_cmp_cstr(string, "None") == 0){
                state->val = VL_Object_new(VL_TYPE_NONE);
            }
            break;
        }
        case 5:{
            S("float", FLOAT)
            else S("infix", INFIX)
            else S("print", PRINT)
            else if(VL_Str_cmp_cstr(string, "False") == 0){
                state->val = VL_Object_from_bool(false);
            }
            break;
        }
        default:
            break;
    }
    #undef S
    if(state->val == NULL){
        state->val = VL_Object_new(VL_TYPE_NONE);
    }
    VL_Str_delete(string);
    return true;
}
bool VL_Module_parse_Operator(VL_Module* self, VL_ParseState* state){
    char chr;
    VL_ParseState begin = *state;
    
    if(VL_Module_parse_token_symbol_operator(chr = VL_Module_peek(self, state))){
        VL_Str* string = VL_Str_new(1);

        VL_Str_append_char(string, chr);
        VL_Module_next(state, chr);

        for(; VL_Module_parse_token_symbol(chr = VL_Module_peek(self, state)); VL_Module_next(state, chr)){
            VL_Str_append_char(string, chr);
        }

        state->val = NULL;
        
        #define C(CHR, SYM)                                                     \
            case CHR: { state->val = VL_Object_from_symbol(VL_SYM_ ## SYM); break; }
                
        #define S(STR, SYM)                                     \
        if(VL_Str_cmp_cstr(string, STR) == 0){                  \
            state->val = VL_Object_from_symbol(VL_SYM_ ## SYM); \
        }
        switch(string->len){
            case 1:
                switch(string->data[0]){
                    C('+', ADD) C('-', SUB) C('*', MUL) C('/', DIV)
                    C('>', GT) C('<', LT) C('=', SET)
                    default:
                        break;
                }
                break;
            case 2:
                S(">=", GTE) else S("<=", LTE) 
                else S("==", EQ) else S("!=", NEQ)
                else S("&&", AND) else S("||", OR)
                break;
            default:
                break;
        }
        #undef C
        #undef S

        if(state->val == NULL){
            state->val = VL_Object_new(VL_TYPE_NONE);
        }

        VL_Str_delete(string);
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
                if(!VL_Module_match_chr(self, state, ')')){
                    VL_Module_push_err_str(self, begin, state,
                        "Expected closing" VLT_ERR("parenthesis ')'") 
                        " of " VLT_ERR("α-expr")
                    );
                    return false;
                }
                return true;
            }
            return false;
        }
        default:{
            if(VL_Module_parse_IExpr(self, state)){
                if(!VL_Module_match_chr(self, state, ')')){
                    VL_Module_push_err_str(self, begin, state,
                        "Expected closing" VLT_ERR("parenthesis ')'") 
                        " of " VLT_ERR("α-expr")
                    );
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
        if(!VL_Module_parse_SpaceSep(self, &temp)){ break; }
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

    state->val = VL_Object_from_symbol(VL_SYM_INFIX);
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
VL_ParseState VL_Module_parse_Lisp(VL_Module* self, VL_ParseState in){
    VL_Module_parse_NExpr(self, &in);
    return in;
}
