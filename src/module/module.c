#include "module.h"

bool VL_Module_parse(VL_Module* self, const VL_Str* file_path){
    if(self->source != NULL){
        VL_ParseState in = { .ok = true, .p.pos = 0, .p.row = 0, .p.col = 0, .val = NULL };
        VL_ParseState out = VL_Module_parse_Lisp(self, in);
        self->ast_tree = out.val;

        if(!out.ok){
            self->ast_tree = NULL;

            printf(VLT_ERR("Error") " parsing [" VLT_RED VLT_BOLD);
            VL_Str_print(self->file_path);
            printf(VLT_RESET "]\n");

            VL_Module_print_error_stack(self);
            return false;
        }

        return true;
    }
    else{
        printf(VLT_ERR("Error") ": ");
        VL_Str_print(file_path);
        printf(" is an invalid directory\n");
    }

    return false;
}

VL_ModuleList* VL_ModuleList_new(size_t len){
    len = (len != 0) ? len : 1;
    VL_ModuleList* self = malloc(sizeof* self);
    
    self->data = malloc(len * sizeof* self->data);
    self->len = 0;
    self->reserve_len = len;
    
    return self;
}
void VL_ModuleList_delete(VL_ModuleList* self){
    for(size_t i = 0; i < self->len; i++){
        VL_Module_clear(&self->data[i]);
    }
    free(self->data);
    free(self);
}
VL_Module* VL_ModuleList_add_module(VL_ModuleList* self, const VL_Str* file_path){
    for(size_t i = 0; i < self->len; i++){
        if(VL_Str_cmp(file_path, self->data[i].file_path) == 0){
            return NULL;        
        }
    }

    if(self->len >= self->reserve_len){
        self->reserve_len *= 2;
        self->data = realloc(self->data, self->reserve_len * sizeof* self->data);
    }

    VL_Module* obj = VL_Module_from_file(file_path);
    self->data[self->len] = *obj;
    self->data[self->len].id = self->len;
    free(obj);
    self->len++;
    
    return &self->data[self->len - 1];
}
VL_Module* VL_ModuleList_get_module(VL_ModuleList* self, size_t id){
    return &self->data[id];
}