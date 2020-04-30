#include "vl_parser.h"

#define TOKEN_SPACE(X) X(' ') X('\n') X('\t') X('\r') X('\f') X('\b')
#define TOKEN_SEP(X) X(',') X(';') X(':')
#define TOKEN_BRACKET(X) X('(') X(')') X('[') X(']') X('{') X('}')
#define TOKEN_OPERATOR(X)           \
X('=') X('<') X('>')                \
X('@') X('$') X('%') X('#') X('?')  \
X('!') X('|') X('&') X('^') X('~')  \
X('+') X('-') X('*') X('/')         \

char VLP_match_escape_seq(char in){
    switch(in){
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'b': return '\b';
        default: return in;
    }
}
bool VLP_token_symbol_label(char val){
    switch(val){
        #define C(X) case X:
        TOKEN_SPACE(C) TOKEN_SEP(C) TOKEN_BRACKET(C) TOKEN_OPERATOR(C)
            return false;
        default:
            return true;
    }
}
bool VLP_token_symbol(char val){
    switch(val){
        #define C(X) case X:
        TOKEN_SPACE(C) TOKEN_SEP(C) TOKEN_BRACKET(C)
            return false;
        default:
            return true;
    }
}
bool VLP_Number(VL_Parser* self, VLP_State* state){
    VLP_State begin = *state;
    
    if(!VLP_match_digit(VLP_peek(self, state))){
        VLP_push_err_str(self, &begin, state,
            "Expected " VLT_ERR("digit") 
            " of " VLT_ERR("int")
        );
        return false;
    }
    
    VL_Int Int = 0;
    for(char chr; VLP_match_digit(chr = VLP_peek(self, state)); VLP_next(state, chr)){
        Int = Int * 10 + (VL_Int)(chr - '0');
    }
    
    if(!VLP_match_chr(self, state, '.')){
        state->val = VL_Object_from_int(Int);
        return true;
    }   
    if(!VLP_match_digit(VLP_peek(self, state))){
        VLP_push_err_str(self, &begin, state,
            "Expected " VLT_ERR("digit") 
            " after " VLT_ERR("period [.]") 
            " of " VLT_ERR("float")
        );
        return false;
    }

    VL_Float pos = 1, Frac = 0;
    for(char chr; VLP_match_digit(chr = VLP_peek(self, state)); VLP_next(state, chr)){
        Frac += (pos *= 0.1)* (VL_Float)(chr - '0');
    }
    state->val = VL_Object_from_float((VL_Float)Int + Frac);
    return true;
}

bool VLP_String(VL_Parser* self, VLP_State* state){
    VLP_State begin = *state;

    if(!VLP_match_chr(self, state, '"')){    
        VLP_push_err_str(self, &begin, state, 
            "Expected opening " VLT_ERR("quote [\"]") 
            " of " VLT_ERR("string")
        );
    }

    VL_Str* string = VL_Str_new(0);
    bool escaped = false;
    char chr;
    
    while(true){
        chr = VLP_peek(self, state);
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
            VL_Str_append_char(string, VLP_match_escape_seq(chr));
            escaped = false;
        }   
        VLP_next(state, chr);
    }

    if(!VLP_match_chr(self, state, '"')){
        VLP_push_err_str(self, &begin, state, 
            "Expected closing " VLT_ERR("quote [\"]") 
            " of " VLT_ERR("string"));
        VL_Str_delete(string);
    }
    
    state->val = VL_Object_wrap_str(string);
    return true;
}

bool VLP_Label(VL_Parser* self, VLP_State* state){
    VLP_State begin = *state;
    
    if(!VLP_token_symbol_label(VLP_peek(self, state))){
        VLP_push_err_str(self, &begin, state,
            "Expected " VLT_ERR("Label")
        );
        return false;    
    }

    VL_Str* string = VL_Str_new(1);

    for(char chr; VLP_token_symbol(chr = VLP_peek(self, state)); VLP_next(state, chr)){
        VL_Str_append_char(string, chr);
    }

    state->val = VL_Object_from_symbol(VL_SYM_OR);
    VL_Str_delete(string);
    return true;
}
bool VLP_token_symbol_operator(char val){
    switch(val){
        #define C(X) case X:
        TOKEN_OPERATOR(C)
            return true;
        default:
            return false;
    }
}
bool VLP_Operator(VL_Parser* self, VLP_State* state){
    char chr;
    VLP_State begin = *state;
    
    if(VLP_token_symbol_operator(chr = VLP_peek(self, state))){
        VL_Str* string = VL_Str_new(1);

        VL_Str_append_char(string, chr);
        VLP_next(state, chr);

        for(; VLP_token_symbol(chr = VLP_peek(self, state)); VLP_next(state, chr)){
            VL_Str_append_char(string, chr);
        }

        state->val = VL_Object_from_symbol(VL_SYM_AND);
        VL_Str_delete(string);
        return true;
    }

    VLP_push_err_str(self, &begin, state,
        "Expected " VLT_ERR("Operator")
    );
    return false;
}

bool VLP_FAtom(VL_Parser* self, VLP_State* state){
    VLP_State begin = *state;
    switch(VLP_peek(self, state)){
        case '"':
            return VLP_String(self, state);
        case '0' ... '9':
            return VLP_Number(self, state);
        case '(':
            VLP_next(state, '(');
            if(VLP_IExpr(self, state)){
                if(!VLP_match_chr(self, state, ')')){
                    VLP_push_err_str(self, &begin, state,
                        "Expected closing" VLT_ERR("parenthesis ')'") 
                        " of " VLT_ERR("α-expr")
                    );
                    return false;
                }
                return true;
            }
            return false;
        
        case '[':
            return VLP_LExpr(self, state);
        case '{':
            return VLP_BExpr(self, state);
        
        case ']':
        case '}':  
        case ';':
        case ',':
            return false;

        #define C(X) case X:
        TOKEN_OPERATOR(C)
            return false;
        TOKEN_SPACE(C)
            return false;
        #undef C

        default:
            return VLP_Label(self, state);
    }
    return false;
}
bool VLP_FExpr(VL_Parser* self, VLP_State* state){
    VLP_State begin = *state;
    
    if(!VLP_FAtom(self, state)){
        VLP_push_err_str(self, &begin, state,
            "Expected " VLT_ERR("name") 
            " of " VLT_ERR("function") 
        );
        return false;
    }            

    VL_Tuple* tuple = VL_Tuple_new(1);
    VL_Tuple_append(tuple, state->val);
    VL_Object_delete(state->val);    
    
    VLP_State temp = *state;
    size_t i = 0;
    while(true){
        if(!VLP_SpaceSep(self, &temp)){ break; }

        if(!VLP_FAtom(self, &temp)){ break; }

        VL_Tuple_append(tuple, temp.val);
        VL_Object_delete(temp.val);    
        *state = temp;
        i++;
    }

    if(i > 0){
        state->val = VL_Object_wrap_tuple(tuple);
        return true;
    }

    VL_Tuple_delete(tuple);
    state->ok = false;
    return false;
}

bool VLP_LExpr(VL_Parser* self, VLP_State* state){
    VLP_State begin = *state;
    VL_Tuple* tuple = VL_Tuple_new(0);

    if(!VLP_match_chr(self, state, '[')){    
        VL_Tuple_delete(tuple);
        VLP_push_err_str(self, &begin, state,
            "Expected opening " VLT_ERR("brackets '['")
            " of " VLT_ERR("λ-expr")
        );
        return false;
    }
            
    VLP_Space(self, state);
    VLP_State temp = *state;

    if(VLP_IExpr(self, &temp)){
        *state = temp;
        VL_Tuple_append(tuple, state->val);
        VL_Object_delete(state->val);
    
        while(true){
            VLP_Space(self, state);

            temp = *state;
            if(!VLP_match_chr(self, &temp, ',')){
                break;
            }
                
            VLP_Space(self, &temp);

            if(!VLP_IExpr(self, &temp)){
                VL_Tuple_delete(tuple);
                VLP_push_err_str(self, &begin, state, 
                    "Expected " VLT_ERR("ι-expr")
                    " after " VLT_ERR("comma [,]") 
                    " in " VLT_ERR("λ-expr")
                );
                return false;
            }
                
            *state = temp;
            VL_Tuple_append(tuple, state->val);
            VL_Object_delete(state->val);
        }
    }

    if(VLP_match_chr(self, state, ']')){
        state->val = VL_Object_wrap_tuple(tuple);
        return true;
    }

    VL_Tuple_delete(tuple);
    VLP_push_err_str(self, &begin, state, 
        "Expected closing " VLT_ERR("brackets ']'")
        " of " VLT_ERR("λ-expr")
    );

    return false;
}
bool VLP_BExpr(VL_Parser* self, VLP_State* state){
    VLP_State begin = *state;

    if(!VLP_match_chr(self, state, '{')){
        VLP_push_err_str(self, &begin, state, 
            "Expected opening" VLT_ERR("braces '{'")         
            " of " VLT_ERR("β-expr")
        );
        return false;
    }

    VL_Tuple* tuple = VL_Tuple_new(1);        
    VLP_State temp = *state;

    while(true){
        VLP_Space(self, &temp);
        if(!VLP_IExpr(self, &temp)){ break; }
        VL_Tuple_append(tuple, temp.val);
        VL_Object_delete(temp.val);    
        *state = temp;

        VLP_Space(self, &temp);

        if(!VLP_match_chr(self, &temp, ';')){
            VLP_push_err_str(self, &begin, state,
                "Expected " VLT_ERR("terminator [;]") 
                " of " VLT_ERR("ι-expr")
                " in " VLT_ERR("β-expr")
            );
            VL_Tuple_delete(tuple);                
            return false;
        }
        
        *state = temp;
    }

    VLP_Space(self, state);
    if(!VLP_match_chr(self, state, '}')){
        VLP_push_err_str(self, &begin, state,
            "Expected closing " VLT_ERR("braces '}'") 
            " of " VLT_ERR("β-expr")
        );
        
        VL_Tuple_delete(tuple);
        return false;   
    }

    state->val = VL_Object_wrap_tuple(tuple);
    return true; 
}
bool VLP_IAtom(VL_Parser* self, VLP_State* state){
    VLP_State begin = *state;
    
    size_t p_err = self->error_stack->len;

    if(VLP_FExpr(self, state)){ 
        return true; 
    }

    VLP_pop_errors(self, p_err, self->error_stack->len);
    *state = begin;
    return VLP_FAtom(self, state);
}
bool VLP_IExpr(VL_Parser* self, VLP_State* state){
    const VLP_State begin = *state;
    
    VL_Tuple* tuple = VL_Tuple_new(1);

    state->val = VL_Object_from_symbol(VL_SYM_INFIX);
    VL_Tuple_append(tuple, state->val);
    VL_Object_delete(state->val);

    size_t p_err = self->error_stack->len;    
    if(!VLP_IAtom(self, state)){
        VL_Tuple_delete(tuple);
        
        if(p_err == self->error_stack->len){
            VLP_push_err_str(self, &begin, state,
                "Expected " VLT_ERR("Atom")
            );
        }
        else{

        }
        return false;
    }

    VLP_State temp = *state;
    
    VL_Tuple_append(tuple, state->val);
    VL_Object_delete(state->val);
    
    while(true){
        if(!VLP_SpaceSep(self, &temp)){ break; }

        p_err = self->error_stack->len; 
        if(!VLP_Operator(self, &temp)){ 
            VLP_pop_errors(self, p_err, self->error_stack->len);
            break;
        }
        VL_Tuple_append(tuple, temp.val);
        VL_Object_delete(temp.val);
        *state = temp;

        if(!VLP_SpaceSep(self, &temp)){
            VL_Tuple_delete(tuple);
            VLP_push_err_str(self, &begin, state,
                "Expected " VLT_ERR("space")
                " after " VLT_ERR("operator")
            );
            return false;
        }

        if(!VLP_IAtom(self, &temp)){ 
            VL_Tuple_delete(tuple);
            VLP_push_err_str(self, &begin, state,
                "Expected " VLT_ERR("ι-atom")
                " after " VLT_ERR("operator")
            );
            return false;
        }

        VL_Tuple_append(tuple, temp.val);
        VL_Object_delete(temp.val);

        *state = temp;
    }
    state->val = VL_Object_wrap_tuple(tuple);
    return true;
}
void VLP_NExpr(VL_Parser* self, VLP_State* state){
    VLP_State begin = *state;
    
    VLP_Space(self, state);
    VLP_IExpr(self, state);
    if(!state->ok){ return; }
    VLP_Space(self, state);

    if(!VLP_match_chr(self, state, ';')){
        VLP_push_err_str(self, &begin, state,
            "Expected closing " VLT_ERR("terminator [;]")
            " of " VLT_ERR("η-expr")
        );
        VL_Object_delete(state->val);
    }
}
VLP_State VLP_Lisp(VL_Parser* self, VLP_State in){
    VLP_NExpr(self, &in);
    return in;
}
