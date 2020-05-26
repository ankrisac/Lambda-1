#include "str.h"

void str_allocate(VL_Str* self, size_t len){
    len = (len != 0) ? len : 1;
    self->data = malloc(len * sizeof* self->data);
    self->reserve_len = len;
}
void str_reserve(VL_Str* self, size_t len){
    self->data = realloc(self->data, len * sizeof* self->data);
    self->reserve_len = len;
}
void str_grow(VL_Str* self){
    str_reserve(self, self->reserve_len * 2);
}

void VL_Str_init(VL_Str* self, size_t len){
    self->len = 0;
    str_allocate(self, len);
}
VL_Str* VL_Str_new(size_t len){
    VL_Str* out = malloc(sizeof(VL_Str));
    VL_Str_init(out, len);
    return out;
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

void VL_Str_print(const VL_Str* self){
    printf("%.*s", (int)self->len, self->data);
}
void VL_Str_repr(const VL_Str* self){
    printf("\"%.*s\"", (int)self->len, self->data);
}
void VL_Str_print_internal(const VL_Str* self){
    printf("[%zu:%zu][%.*s]", self->len, self->reserve_len, (int)self->len, self->data);
}

VL_Str* VL_Str_clone(const VL_Str* self){
    VL_Str* out = malloc(sizeof(VL_Str));
    VL_Str_copy(out, self);
    return out;
}
void VL_Str_copy(VL_Str* self, const VL_Str* src){
    self->len = src->len;
    str_allocate(self, src->len);
    memcpy(self->data, src->data, src->len);
}
void VL_Str_copy_cstr(VL_Str* self, const char* value){
    self->len = strlen(value);
    str_allocate(self, self->len);
    memcpy(self->data, value, self->len);
}

void VL_Str_concat(VL_Str* self, const VL_Str* value){
    if(self->len + value->len >= self->reserve_len){
        str_reserve(self, self->len + value->len);
    }
    memcpy(&self->data[self->len], value->data, value->len);
    self->len = self->len + value->len;
}
void VL_Str_append_char(VL_Str* self, char value){
    if(self->len >= self->reserve_len){
        str_grow(self);
    }   
    self->data[self->len] = value;
    self->len++;
}
void VL_Str_append_cstr(VL_Str* self, const char* value){
    size_t len = strlen(value);
    if(self->len + len >= self->reserve_len){
        str_reserve(self, self->len + len);
    }
    memcpy(&self->data[self->len], value, len);
    self->len = self->len + len;
}
void VL_Str_append_int(VL_Str* self, VL_Int value){
    VL_Str* int_str = VL_Str_from_int(value);
    VL_Str_concat(self, int_str);
    VL_Str_delete(int_str);
}

VL_Str* VL_Str_from_cstr(const char* str){
    VL_Str* out = malloc(sizeof(VL_Str));
    VL_Str_copy_cstr(out, str);
    return out;
}
VL_Str* VL_Str_from_int(VL_Int value){
    VL_Str* out = VL_Str_new(1);
    
    bool neg = false;
    if(value < 0){
        value *= -1;
        neg = true;
    }
    
    if(value == 0){
        VL_Str_append_char(out, '0');    
    }
    else{
        while(value > 0){
            char chr = '0' + (char)(value % 10);
            VL_Str_append_char(out, chr);
            value = (value / 10);
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

    if(file != NULL){
        VL_Str* out = VL_Str_new(0);
    
        char chr;
        while((chr = fgetc(file)) != EOF){
            VL_Str_append_char(out, chr);
        }
        fclose(file);    
        return out;
    }

    return NULL;
}
VL_Str* VL_Str_from_file(const VL_Str* file_path){
    char* path_str = VL_Str_to_cstr(file_path);
    VL_Str* out = VL_Str_from_file_cstr(path_str);
    free(path_str);
    return out;
}
VL_Str* VL_Str_from_cin(){
    char* buffer = NULL;
    size_t reserve_len = 0;
    size_t n = getline(&buffer, &reserve_len, stdin);
    buffer[n - 1] = '\0';

    VL_Str* out = VL_Str_new(n);
    VL_Str_append_cstr(out, buffer);
    free(buffer);
    return out;
}

char* VL_Str_to_cstr(const VL_Str* self){
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
short VL_Str_cmp(const VL_Str* self, const VL_Str* other){
    size_t len = (self->len > other->len) ? other->len : self->len;

    for(size_t i = 0; i < len; i++){
        if(self->data[i] < other->data[i]){
            return -1;
        }
        else if(self->data[i] > other->data[i]){
            return +1;
        }
    }
    
    if(self->len < other->len){
        return -1;
    }
    else if(self->len == other->len){
        return 0;
    }
    return 1;
}
short VL_Str_cmp_cstr(const VL_Str* self, const char* str){
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

bool VL_Str_is_char(const VL_Str* self, const char value){
    if(self->len == 1){
        return (self->data[0] == value);
    }
    return false;
}

size_t VL_Str_hash(const VL_Str* str){
    //DJB2 hash function

    size_t hash = 5381;
    for(size_t i = 0; i < str->len; i++){
        hash = ((hash << 5) + hash) + str->data[i];
    }
    return hash;
}
