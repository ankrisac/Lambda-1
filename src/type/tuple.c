#include "tuple.h"

void tuple_allocate(VL_Tuple* self, size_t len){
    len = (len != 0) ? len : 1;
    self->data = malloc(len * sizeof* self->data);
    self->reserve_len = len;
}
void tuple_reserve(VL_Tuple* self, size_t len){
    self->data = realloc(self->data, len * sizeof* self->data);
    self->reserve_len = len;
}
void tuple_grow(VL_Tuple* self){
    tuple_reserve(self, self->reserve_len * 2);
}

void VL_Tuple_init(VL_Tuple* self, size_t len){
    self->len = 0;
    tuple_allocate(self, len);
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
    tuple_allocate(self, self->len);
    
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
        tuple_grow(self);
    }
    VL_Object_copy(&self->data[self->len], value);
    self->len++;
}
void VL_Tuple_append(VL_Tuple* self, VL_Object* value){
    if(self->len >= self->reserve_len){
        tuple_grow(self);
    }
    self->data[self->len] = *value;
    self->len++;
}

void VL_Tuple_pop_to(VL_Tuple* self, VL_Object* dest){
    if(self->len > 0){
        self->len--;
        VL_Object_move(&self->data[self->len], dest);
    }
    else{
        dest->type = VL_TYPE_NONE;
    }
}
VL_Object* VL_Tuple_pop(VL_Tuple* self){
    if(self->len > 0){
        self->len--;
        return VL_Object_move_ref(&self->data[self->len]);
    }
    return VL_Object_new(VL_TYPE_NONE);
}

void VL_Tuple_drop(VL_Tuple* self){
    if(self->len > 0){
        self->len--;
        VL_Object_clear(&self->data[self->len]);
    }
}
void VL_Tuple_dropn(VL_Tuple* self, size_t n){
    if(self->len >= n){
        for(size_t i = self->len - n; i < self->len; i++){
            VL_Object_clear(&self->data[i]);
        }
        self->len -= n;
    }
}


const VL_Object* VL_Tuple_get(const VL_Tuple* self, size_t i){
    return &self->data[i];
}
const VL_Object* VL_Tuple_rget(const VL_Tuple* self, size_t i){
    return &self->data[self->len - i - 1];
}
VL_Object* VL_Tuple_mget(VL_Tuple* self, size_t i){
    return &self->data[i];
}
VL_Object* VL_Tuple_mrget(VL_Tuple* self, size_t i){
    return &self->data[self->len - i - 1];
}

void VL_Tuple_set(VL_Tuple* self, size_t i, VL_Object* value){
    self->data[i] = *value;
}
void VL_Tuple_rset(VL_Tuple* self, size_t i, VL_Object* value){
    self->data[self->len - i - 1] = *value;
}

void VL_Tuple_print(const VL_Tuple* self){
    printf("[");
    if(self->len >= 1){
        VL_Object_print(&self->data[0]);

        for(size_t i = 1; i < self->len; i++){
            printf(", ");
            VL_Object_print(&self->data[i]);
        }
    }
    printf("]");
}
void VL_Tuple_repr(const VL_Tuple* self){
    printf("[");
    if(self->len >= 1){
        VL_Object_repr(&self->data[0]);

        for(size_t i = 1; i < self->len; i++){
            printf(", ");
            VL_Object_repr(&self->data[i]);
        }
    }
    printf("]");
}