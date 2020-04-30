#include "parser.h"


char VLP_peek(VL_Parser* self, VLP_State* state){
    if(state->pos < self->stream->len){
        return self->stream->data[state->pos];
    }
    return '\0';
}
void VLP_next(VLP_State* state, char chr){    
    if(chr == '\n'){
        state->row++;
        state->col = 0;
    }
    else{
        state->col++;
    }
    state->pos++;
}
char VLP_pop(VL_Parser* self, VLP_State* state){
    if(state->pos < self->stream->len){
        char chr = self->stream->data[state->pos];
        VLP_next(state, chr);
        return chr;
    }  
    return '\0';
}

VL_Str* VLP_get_ln(const VL_Parser* self, VLP_State s_begin, VLP_State s_end, const char* cursor, const char* highlight_color){
    VL_Str* ln = VL_Str_from_cstr("Ln[");   

    VL_Str_append_int(ln, (VL_Int)(s_begin.row + 1));
    VL_Str_append_cstr(ln, ":");
    VL_Str_append_int(ln, (VL_Int)(s_begin.col + 1));
    VL_Str_append_cstr(ln, "] -> Ln[");


    VL_Str_append_int(ln, (VL_Int)(s_end.row + 1));
    VL_Str_append_cstr(ln, ":");
    VL_Str_append_int(ln, (VL_Int)(s_end.col + 1));
    VL_Str_append_cstr(ln, "] ");

    VL_Str_append(ln, self->file_path);
    VL_Str_append_cstr(ln, "\n| ");

    size_t cursor_pos = (s_end.pos < self->stream->len) ? s_end.pos : (self->stream->len - 1);

    if(s_begin.row < s_end.row){
        s_begin.col = 0;
    }
    
    size_t ln_begin = cursor_pos;
    if(self->stream->data[cursor_pos] == '\n'){
        ln_begin--;
    }
    for(; ln_begin > 0; ln_begin--){
        if(self->stream->data[ln_begin] == '\n'){
            ln_begin++;
            break;
        }
    }

    size_t pos = ln_begin;

    char val;    
    while(pos < s_begin.pos){
        val = self->stream->data[pos];
        VL_Str_append_char(ln, val);
        pos++;
    }
    VL_Str_append_cstr(ln, highlight_color);
    while(pos < cursor_pos){
        val = self->stream->data[pos];
        VL_Str_append_char(ln, val);
        pos++;
    }
    VL_Str_append_cstr(ln, VLT_RESET);
    while(pos < self->stream->len){
        val = self->stream->data[pos];
        VL_Str_append_char(ln, val);
        if(val == '\n'){
            VL_Str_append_cstr(ln, "| ");
            break;
        }
        pos++;
    }

    VL_Str_append_cstr(ln, highlight_color);
    for(size_t i = ln_begin; i < cursor_pos; i++){
        VL_Str_append_char(ln, '-');
    }

    VL_Str_append_cstr(ln, highlight_color);
    VL_Str_append_cstr(ln, cursor);
    VL_Str_append_cstr(ln, VLT_RESET);

    return ln;
}
void VLP_push_err(VL_Parser* self, const VLP_State* begin, VLP_State* end, VL_Str* str){
    VL_Str* ln = VLP_get_ln(self, *begin, *end, "^\n", VLT_BOLD VLT_RED);
    VL_Str_append(ln, str);
    VL_Str_delete(str);

    VL_Object* err = VL_Object_wrap_str(ln);
    VL_Tuple_append(self->error_stack, err);
    VL_Object_delete(err);
    end->ok = false;
}
void VLP_push_err_str(VL_Parser* self,  const VLP_State* begin, VLP_State* end, char* err_msg){
    VLP_push_err(self, begin, end, VL_Str_from_cstr(err_msg));
}
void VLP_pop_err(VL_Parser* self){
    VL_Object_delete(VL_Tuple_pop(self->error_stack));
}
void VLP_pop_errors(VL_Parser* self, size_t a, size_t b){
    for(size_t i = a; i < b; i++){
        VL_Object_delete(VL_Tuple_pop(self->error_stack));
    }
}
void VLP_error_stack(VL_Parser* self){
    for(size_t i = 0; i < self->error_stack->len; i++){
        printf(VLT_BOLD VLT_RED "Error [%zu] : " VLT_RESET, i + 1);
        VL_ObjectData_print(&self->error_stack->data[i], self->error_stack->type[i]);
        printf("\n");
    }
}
void VLP_print_state(const VL_Parser* self, const VLP_State* begin, VLP_State* end){
    VL_Str* loc = VLP_get_ln(self, *begin, *end, "^\n", VLT_BOLD VLT_BLU);
    VL_Str_print(loc);
    VL_Str_delete(loc);
}


bool VLP_match_chr(VL_Parser* self, VLP_State* state, char val){
    if(VLP_peek(self, state) == val){
        VLP_next(state, val);
        return true;  
    }
    return false;
}

bool VLP_match_space(char val){
    switch(val){
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return true;
        default:
            return false;
    }
}
void VLP_Space(VL_Parser* self, VLP_State* state){
    for(char chr; VLP_match_space(chr = VLP_peek(self, state)); VLP_next(state, chr));
}
bool VLP_SpaceSep(VL_Parser* self, VLP_State* state){
    char chr = VLP_peek(self, state);

    if(VLP_match_space(chr)){
        VLP_next(state, chr);
        for(; VLP_match_space(chr = VLP_peek(self, state)); VLP_next(state, chr));
        return true;
    }
    state->ok = false;
    return false;
}

bool VLP_match_digit(char val){
    switch(val){
        case '0' ... '9':
            return true;
        default:
            return false;
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

char VLP_match_escape_seq(char in){
    switch(in){
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'b': return '\b';
        default: return in;
    }
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
