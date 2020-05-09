#include "source.h"

VL_Module* VL_Module_from_file(const VL_Str* file_path){
    VL_Str* src = VL_Str_from_file(file_path);

    if(src != NULL){
        VL_Module* out = malloc(sizeof* out);
        out->error_stack = VL_Tuple_new(0);
        out->file_path = VL_Str_clone(file_path);
        out->source = src;
        return out;
    }
    return NULL;
}
void VL_Module_clear(VL_Module* self){
    VL_Str_delete(self->source);
    VL_Str_delete(self->file_path);
    
    VL_Tuple_delete(self->error_stack);
    
    if(self->ast_tree != NULL){
        VL_Object_delete(self->ast_tree);
    }
}
void VL_Module_delete(VL_Module* self){
    VL_Module_clear(self);
    free(self);
}

VL_Str* VL_Module_get_ln(const VL_Module* self, VL_SrcPos s_begin, VL_SrcPos s_end, const char* highlight_color){
    VL_Str* ln = VL_Str_from_cstr("Ln[");   

    VL_Str_append_int(ln, (VL_Int)(s_begin.pos));
    VL_Str_append_cstr(ln, "-");
    
    VL_Str_append_int(ln, (VL_Int)(s_begin.row + 1));
    VL_Str_append_cstr(ln, ":");
    VL_Str_append_int(ln, (VL_Int)(s_begin.col + 1));
    VL_Str_append_cstr(ln, "] -> Ln[");

    VL_Str_append_int(ln, (VL_Int)(s_end.pos));
    VL_Str_append_cstr(ln, "-");

    VL_Str_append_int(ln, (VL_Int)(s_end.row + 1));
    VL_Str_append_cstr(ln, ":");
    VL_Str_append_int(ln, (VL_Int)(s_end.col + 1));
    VL_Str_append_cstr(ln, "] ");

    VL_Str_append(ln, self->file_path);
    VL_Str_append_cstr(ln, "\n| ");

    size_t cursor_pos = (s_end.pos < self->source->len) ? s_end.pos : (self->source->len - 1);

    if(s_begin.row < s_end.row){
        s_begin.col = 0;
    }
    
    size_t ln_begin = cursor_pos;
    if(self->source->data[cursor_pos] == '\n'){
        ln_begin--;
    }
    for(; ln_begin > 0; ln_begin--){
        if(self->source->data[ln_begin] == '\n'){
            ln_begin++;
            break;
        }
    }

    size_t pos = ln_begin;

    char val = ' ';    
    for(;pos < s_begin.pos;){
        val = self->source->data[pos];
        VL_Str_append_char(ln, val);
        pos++;
    }
    VL_Str_append_cstr(ln, highlight_color);
    for(;pos < cursor_pos; pos++){
        val = self->source->data[pos];
        VL_Str_append_char(ln, val);
    }
    VL_Str_append_cstr(ln, VLT_RESET);
    for(;pos < self->source->len; pos++){
        val = self->source->data[pos];
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
void VL_Module_push_err(VL_Module* self, const VL_ParseState* begin, VL_ParseState* end, VL_Str* msg){
    VL_Str* ln = VL_Module_get_ln(self, begin->p, end->p, VLT_BOLD VLT_RED);
    VL_Str_append_cstr(ln, "| ");
    VL_Str_append(ln, msg);
    VL_Str_delete(msg);

    VL_Object* err = VL_Object_wrap_str(ln);
    VL_Tuple_append(self->error_stack, err);
    free(err);

    end->ok = false;
}
void VL_Module_push_err_str(VL_Module* self,  const VL_ParseState* begin, VL_ParseState* end, char* err_msg){
    VL_Module_push_err(self, begin, end, VL_Str_from_cstr(err_msg));
}
void VL_Module_pop_err(VL_Module* self){
    VL_Object_delete(VL_Tuple_pop(self->error_stack));
}
void VL_Module_pop_errors(VL_Module* self, size_t a, size_t b){
    for(size_t i = a; i < b; i++){
        VL_Object_delete(VL_Tuple_pop(self->error_stack));
    }
}
void VL_Module_print_error_stack(VL_Module* self){
    for(size_t i = 0; i < self->error_stack->len; i++){
        printf(VLT_BOLD VLT_RED "Error [%zu] : " VLT_RESET, i + 1);
        VL_Object_print(VL_Tuple_get(self->error_stack, i));
        printf("\n");
    }
}
void VL_Module_print_state(const VL_Module* self, VL_SrcPos begin, VL_SrcPos end){
    VL_Str* loc = VL_Module_get_ln(self, begin, end, VLT_BOLD VLT_BLU);
    VL_Str_print(loc);
    VL_Str_delete(loc);
}


char VL_Module_peek(VL_Module* self, VL_ParseState* state){
    if(state->p.pos < self->source->len){
        return self->source->data[state->p.pos];
    }
    return '\0';
}
void VL_Module_next(VL_ParseState* state, char chr){    
    if(chr == '\n'){
        state->p.row++;
        state->p.col = 0;
    }
    else{
        state->p.col++;
    }
    state->p.pos++;
}
char VL_Module_pop(VL_Module* self, VL_ParseState* state){
    if(state->p.pos < self->source->len){
        char chr = self->source->data[state->p.pos];
        VL_Module_next(state, chr);
        return chr;
    }  
    return '\0';
}
bool VL_Module_match_chr(VL_Module* self, VL_ParseState* state, char val){
    if(VL_Module_peek(self, state) == val){
        VL_Module_next(state, val);
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
            return true;
        default:
            return false;
    }
}

void VL_Module_parse_Space(VL_Module* self, VL_ParseState* state){
    for(char chr; VL_match_space(chr = VL_Module_peek(self, state)); VL_Module_next(state, chr));
}
bool VL_Module_parse_SpaceSep(VL_Module* self, VL_ParseState* state){
    char chr = VL_Module_peek(self, state);

    if(VL_match_space(chr)){
        VL_Module_next(state, chr);
        for(; VL_match_space(chr = VL_Module_peek(self, state)); VL_Module_next(state, chr));
        return true;
    }
    state->ok = false;
    return false;
}
