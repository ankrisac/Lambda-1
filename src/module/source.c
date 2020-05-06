#include "source.h"

VL_ModuleSrc* VL_ModuleSrc_new(const VL_Str* file_path){
    VL_ModuleSrc* out = malloc(sizeof* out);
    out->error_stack = VL_Tuple_new(0);
    out->file_path = VL_Str_clone(file_path);
    out->stream = VL_Str_from_file(file_path);
    return out;
}
void VL_ModuleSrc_clear(VL_ModuleSrc* self){
    VL_Tuple_delete(self->error_stack);
    VL_Str_delete(self->stream);
    VL_Str_delete(self->file_path);
}
void VL_ModuleSrc_delete(VL_ModuleSrc* self){
    VL_ModuleSrc_clear(self);
    free(self);
}

VL_Str* VL_ModuleSrc_get_ln(const VL_ModuleSrc* self, VL_SrcPos s_begin, VL_SrcPos s_end, const char* highlight_color){
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
    for(;pos < s_begin.pos;){
        val = self->stream->data[pos];
        VL_Str_append_char(ln, val);
        pos++;
    }
    VL_Str_append_cstr(ln, highlight_color);
    for(;pos < cursor_pos; pos++){
        val = self->stream->data[pos];
        VL_Str_append_char(ln, val);
    }
    VL_Str_append_cstr(ln, VLT_RESET);
    for(;pos < self->stream->len; pos++){
        val = self->stream->data[pos];
        VL_Str_append_char(ln, val);
        if(val == '\n'){
            VL_Str_append_cstr(ln, "| ");
            break;
        }
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
    VL_Str_append_cstr(ln, "^" VLT_RESET "\n");

    return ln;
}
void VL_ModuleSrc_push_err(VL_ModuleSrc* self, const VL_ParseState* begin, VL_ParseState* end, VL_Str* msg){
    VL_Str* ln = VL_ModuleSrc_get_ln(self, begin->p, end->p, VLT_BOLD VLT_RED);
    VL_Str_append_cstr(ln, "| ");
    VL_Str_append(ln, msg);
    VL_Str_delete(msg);

    VL_Object* err = VL_Object_wrap_str(ln);
    VL_Tuple_append(self->error_stack, err);
    free(err);

    end->ok = false;
}
void VL_ModuleSrc_push_err_str(VL_ModuleSrc* self,  const VL_ParseState* begin, VL_ParseState* end, char* err_msg){
    VL_ModuleSrc_push_err(self, begin, end, VL_Str_from_cstr(err_msg));
}
void VL_ModuleSrc_pop_err(VL_ModuleSrc* self){
    VL_Object_delete(VL_Tuple_pop(self->error_stack));
}
void VL_ModuleSrc_pop_errors(VL_ModuleSrc* self, size_t a, size_t b){
    for(size_t i = a; i < b; i++){
        VL_Object_delete(VL_Tuple_pop(self->error_stack));
    }
}
void VL_ModuleSrc_print_error_stack(VL_ModuleSrc* self){
    for(size_t i = 0; i < self->error_stack->len; i++){
        printf(VLT_BOLD VLT_RED "Error [%zu] : " VLT_RESET, i + 1);
        VL_Object_print(VL_Tuple_get(self->error_stack, i));
        printf("\n");
    }
}
void VL_ModuleSrc_print_state(const VL_ModuleSrc* self, VL_SrcPos begin, VL_SrcPos end){
    VL_Str* loc = VL_ModuleSrc_get_ln(self, begin, end, VLT_BOLD VLT_BLU);
    VL_Str_print(loc);
    VL_Str_delete(loc);
}


char VL_ModuleSrc_peek(VL_ModuleSrc* self, VL_ParseState* state){
    if(state->p.pos < self->stream->len){
        return self->stream->data[state->p.pos];
    }
    return '\0';
}
void VL_ModuleSrc_next(VL_ParseState* state, char chr){    
    if(chr == '\n'){
        state->p.row++;
        state->p.col = 0;
    }
    else{
        state->p.col++;
    }
    state->p.pos++;
}
char VL_ModuleSrc_pop(VL_ModuleSrc* self, VL_ParseState* state){
    if(state->p.pos < self->stream->len){
        char chr = self->stream->data[state->p.pos];
        VL_ModuleSrc_next(state, chr);
        return chr;
    }  
    return '\0';
}
bool VL_ModuleSrc_match_chr(VL_ModuleSrc* self, VL_ParseState* state, char val){
    if(VL_ModuleSrc_peek(self, state) == val){
        VL_ModuleSrc_next(state, val);
        return true;  
    }
    return false;
}
bool VL_match_space(char val){
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
bool VL_match_digit(char val){
    switch(val){
        case '0' ... '9':
            return true;
        default:
            return false;
    }
}

void VL_ModuleSrc_parse_Space(VL_ModuleSrc* self, VL_ParseState* state){
    for(char chr; VL_match_space(chr = VL_ModuleSrc_peek(self, state)); VL_ModuleSrc_next(state, chr));
}
bool VL_ModuleSrc_parse_SpaceSep(VL_ModuleSrc* self, VL_ParseState* state){
    char chr = VL_ModuleSrc_peek(self, state);

    if(VL_match_space(chr)){
        VL_ModuleSrc_next(state, chr);
        for(; VL_match_space(chr = VL_ModuleSrc_peek(self, state)); VL_ModuleSrc_next(state, chr));
        return true;
    }
    state->ok = false;
    return false;
}
