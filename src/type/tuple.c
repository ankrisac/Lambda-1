#include "tuple.h"

void __VL_Tuple_malloc(VL_Tuple* self, size_t len){
    len = (len != 0) ? len : 1;
    self->data = malloc(len * sizeof* self->data);
    self->reserve_len = len;
}
void __VL_Tuple_realloc(VL_Tuple* self, size_t len){
    self->data = realloc(self->data, len * sizeof* self->data);
    self->reserve_len = len;
}
void __VL_Tuple_grow(VL_Tuple* self){
    __VL_Tuple_realloc(self, self->reserve_len * 2);
}

void VL_Tuple_init(VL_Tuple* self, size_t len){
    self->len = 0;
    __VL_Tuple_malloc(self, len);
}
VL_Tuple* VL_Tuple_new(size_t len){
    VL_Tuple* out = malloc(sizeof(VL_Tuple));
    VL_Tuple_init(out, len);
    return out;
}
void VL_Tuple_clear(VL_Tuple* self){
    for(size_t i = 0; i < self->len; i++){
        VL_Object_clear(&self->data[i]);
    }
    free(self->data);
}
void VL_Tuple_delete(VL_Tuple* self){
    VL_Tuple_clear(self);
    free(self);
}

void VL_Tuple_copy(VL_Tuple* self, const VL_Tuple* src){
    self->len = src->len;
    __VL_Tuple_malloc(self, self->len);
    
    for(size_t i = 0; i < self->len; i++){
        VL_Object_copy(&self->data[i], &src->data[i]);
    }
}
VL_Tuple* VL_Tuple_clone(const VL_Tuple* self){
    VL_Tuple* out = malloc(sizeof(VL_Tuple));
    VL_Tuple_copy(out, self);
    return out;
}

void VL_Tuple_append_copy(VL_Tuple* self, const VL_Object* value){
    if(self->len >= self->reserve_len){
        __VL_Tuple_grow(self);
    }
    VL_Object_copy(&self->data[self->len], value);
    self->len++;
}
void VL_Tuple_append(VL_Tuple* self, VL_Object* value){
    if(self->len >= self->reserve_len){
        __VL_Tuple_grow(self);
    }
    self->data[self->len] = *value;
    self->len++;
}
VL_Object* VL_Tuple_pop(VL_Tuple* self){
    if(self->len > 0){
        self->len--;
        return VL_Object_move(&self->data[self->len]);
    }
    return VL_Object_new(VL_TYPE_NONE);
}

const VL_Object* VL_Tuple_get(const VL_Tuple* self, size_t i){
    return &self->data[i];
}
const VL_Object* VL_Tuple_rget(const VL_Tuple* self, size_t i){
    return &self->data[self->len - i - 1];
}
void VL_Tuple_set(VL_Tuple* self, size_t i, VL_Object* value){
    self->data[i] = *value;
}
void VL_Tuple_rset(VL_Tuple* self, size_t i, VL_Object* value){
    self->data[self->len - i - 1] = *value;
}

void VL_Tuple_print(const VL_Tuple* self){
    printf("(");
    if(self->len >= 1){
        VL_Object_print(&self->data[0]);

        for(size_t i = 1; i < self->len; i++){
            printf(", ");
            VL_Object_print(&self->data[i]);
        }
    }
    printf(")");
}