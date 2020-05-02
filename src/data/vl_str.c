#include "vl_str.h"

void __VL_Str_malloc(VL_Str* self, size_t len){
    len = (len != 0) ? len : 1;
    self->data = malloc(len * sizeof* self->data);
    self->reserve_len = len;
}
void __VL_Str_realloc(VL_Str* self, size_t len){
    self->data = realloc(self->data, len * sizeof* self->data);
    self->reserve_len = len;
}
void __VL_Str_grow(VL_Str* self){
    __VL_Str_realloc(self, self->reserve_len * 2);
}

void VL_Str_init(VL_Str* self, size_t len){
    self->len = 0;
    __VL_Str_malloc(self, len);
}
void VL_Str_clear(VL_Str* self){
    if(self->reserve_len != 0){
        free(self->data);
        self->reserve_len = 0;
        self->data = NULL;
    }
    self->len = 0;
}
void VL_Str_delete(VL_Str* self){
    VL_Str_clear(self);
    free(self);
}

VL_Str* VL_Str_new(size_t len){
    VL_Str* out = malloc(sizeof(VL_Str));
    VL_Str_init(out, len);
    return out;
}
VL_Str* VL_Str_copy(VL_Str* self){
    VL_Str* out = malloc(sizeof(VL_Str));
    VL_Str_set(out, self);
    return out;
}

void VL_Str_set(VL_Str* self, VL_Str* src){
    self->len = src->len;
    __VL_Str_malloc(self, src->len);
    memcpy(self->data, src->data, src->len);
}
void VL_Str_set_cstr(VL_Str* self, const char* str){
    self->len = strlen(str);
    __VL_Str_malloc(self, self->len);
    memcpy(self->data, str, self->len);
}

void VL_Str_append(VL_Str* self, const VL_Str* out){
    if(self->len + out->len >= self->reserve_len){
        __VL_Str_realloc(self, self->len + out->len);
    }
    memcpy(&self->data[self->len], out->data, out->len);
    self->len = self->len + out->len;
}
void VL_Str_append_char(VL_Str* self, char value){
    if(self->len >= self->reserve_len){
        __VL_Str_grow(self);
    }   
    self->data[self->len] = value;
    self->len++;
}
void VL_Str_append_cstr(VL_Str* self, const char* str){
    size_t str_len = strlen(str);
    if(self->len + str_len >= self->reserve_len){
        __VL_Str_realloc(self, self->len + str_len);
    }
    memcpy(&self->data[self->len], str, str_len);
    self->len = self->len + str_len;
}
void VL_Str_append_int(VL_Str* self, VL_Int val){
    VL_Str* i_str = VL_Str_from_int(val);
    VL_Str_append(self, i_str);
    VL_Str_delete(i_str);
}

void VL_Str_print(VL_Str* self){
    printf("%.*s", (int)self->len, self->data);
}
void VL_Str_print_internal(VL_Str* self){
    printf("[%zu:%zu][%.*s]", self->len, self->reserve_len, (int)self->len, self->data);
}

VL_Str* VL_Str_from_cstr(const char* str){
    VL_Str* out = malloc(sizeof(VL_Str));
    VL_Str_set_cstr(out, str);
    return out;
}
VL_Str* VL_Str_from_int(VL_Int val){
    VL_Str* out = VL_Str_new(1);
    
    bool neg = false;
    if(val < 0){
        val *= -1;
        neg = true;
    }
    
    if(val == 0){
        VL_Str_append_char(out, '0');    
    }
    else{
        while(val > 0){
            char chr = '0' + (char)(val % 10);
            VL_Str_append_char(out, chr);
            val = (val / 10);
        }
        
        if(neg){
            VL_Str_append_char(out, '-');
        }
    }

    VL_Str_reverse(out);
    return out;
}
VL_Str* VL_Str_from_file_cstr(const char* file_path){
    FILE* file = fopen(file_path, "r");
    VL_Str* out = VL_Str_new(0);
    
    if(file != NULL){
        char chr;
        while((chr = fgetc(file)) != EOF){
            VL_Str_append_char(out, chr);
        }
        fclose(file);    
    }
    else{
        printf("Error: %s is an invalid directory\n", file_path);
    }

    return out;
}
VL_Str* VL_Str_from_file(VL_Str* file_path){
    char* path_str = VL_Str_to_cstr(file_path);
    VL_Str* out = VL_Str_from_file_cstr(path_str);
    free(path_str);
    return out;
}

char* VL_Str_to_cstr(VL_Str* self){
    char* out = malloc((self->len + 1) * sizeof* out);
    memcpy(out, self->data, self->len);
    out[self->len] = '\0';
    return out;
}

void VL_Str_reverse(VL_Str* self){
    for(size_t i = 0, j = self->len - 1; i < j; i++, j--){
        char temp = self->data[i];
        self->data[i] = self->data[j];
        self->data[j] = temp;
    }
}
int VL_Str_cmp_cstr(const VL_Str* self, const char* str){
    const char* j = str;
    for(size_t i = 0; i < self->len; i++, j++){
        if(*j == '\0'){
            return -1;
        }

        if(self->data[i] < *j){
            return 1;
        }
        else if(self->data[i] > *j){
            return -1;
        }     
    }
    if(*j == '\0'){
        return 0;
    }
    return 1;
}

bool VL_Str_is_char(const VL_Str* self, char val){
    if(self->len == 1){
        return (self->data[0] == val);
    }
    return false;
}