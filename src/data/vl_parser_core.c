#include "vl_parser_core.h"

char VLP_peek(VL_Parser* self, VLP_State* state){
    if(state->p.pos < self->stream->len){
        return self->stream->data[state->p.pos];
    }
    return '\0';
}
void VLP_next(VLP_State* state, char chr){    
    if(chr == '\n'){
        state->p.row++;
        state->p.col = 0;
    }
    else{
        state->p.col++;
    }
    state->p.pos++;
}
char VLP_pop(VL_Parser* self, VLP_State* state){
    if(state->p.pos < self->stream->len){
        char chr = self->stream->data[state->p.pos];
        VLP_next(state, chr);
        return chr;
    }  
    return '\0';
}

VL_Str* VLP_get_ln(const VL_Parser* self, VLP_Pos s_begin, VLP_Pos s_end, const char* cursor, const char* highlight_color){
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

    char val = ' ';    
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
    if(val != '\n'){
        VL_Str_append_cstr(ln, "\n| ");
    }


    VL_Str_append_cstr(ln, highlight_color);
    for(pos = ln_begin; pos < s_begin.pos; pos++){
        VL_Str_append_char(ln, ' ');
    }
    for(; pos < cursor_pos; pos++){
        VL_Str_append_char(ln, '~');
    }

    VL_Str_append_cstr(ln, highlight_color);
    VL_Str_append_cstr(ln, cursor);
    VL_Str_append_cstr(ln, VLT_RESET);

    return ln;
}
void VLP_push_err(VL_Parser* self, const VLP_State* begin, VLP_State* end, VL_Str* str){
    VL_Str* ln = VLP_get_ln(self, begin->p, end->p, "^\n", VLT_BOLD VLT_RED);
    VL_Str_append(ln, str);
    VL_Str_delete(str);

    VL_Object* err = VL_Object_wrap_str(ln);
    VL_Tuple_append(self->error_stack, err);
    VL_Object_delete(err);
    end->p.ok = false;
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
void VLP_print_state(const VL_Parser* self, VLP_Pos begin, VLP_Pos end){
    VL_Str* loc = VLP_get_ln(self, begin, end, "^\n", VLT_BOLD VLT_BLU);
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
bool VLP_match_digit(char val){
    switch(val){
        case '0' ... '9':
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
    state->p.ok = false;
    return false;
}
