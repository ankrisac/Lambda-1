#include "vl_tuple.h"

void __VL_Tuple_malloc(VL_Tuple* self, size_t len){
    len = (len != 0) ? len : 1;
    self->data = malloc(len * sizeof* self->data);
    self->type = malloc(len * sizeof* self->type);
    self->reserve_len = len;
}
void __VL_Tuple_realloc(VL_Tuple* self, size_t len){
    self->data = realloc(self->data, len * sizeof* self->data);
    self->type = realloc(self->type, len * sizeof* self->type);
    self->reserve_len = len;
}
void __VL_Tuple_grow(VL_Tuple* self){
    __VL_Tuple_realloc(self, self->reserve_len * 2);
}

void VL_Tuple_init(VL_Tuple* self, size_t len){
    len = (len != 0) ? len : 1;
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
        VL_ObjectData_clear(&self->data[i], self->type[i]);
        self->type[i] = VL_TYPE_NONE;
    }
    free(self->data);
    free(self->type);
}
void VL_Tuple_delete(VL_Tuple* self){
    VL_Tuple_clear(self);
    free(self);
}

void VL_Tuple_set(VL_Tuple* self, VL_Tuple* src){
    self->len = src->len;
    __VL_Tuple_malloc(self, self->len);
    memcpy(self->type, src->type, self->len);

    for(size_t i = 0; i < self->len; i++){
        VL_ObjectData_set(&self->data[i], &src->data[i], src->type[i]);
        self->type[i] = src->type[i];
    }
}
VL_Tuple* VL_Tuple_copy(VL_Tuple* self){
    VL_Tuple* out = malloc(sizeof(VL_Tuple));
    VL_Tuple_set(out, self);
    return out;
}

void VL_Tuple_append(VL_Tuple* self, VL_Object* other){
    if(self->len >= self->reserve_len){
        __VL_Tuple_grow(self);
    }
    VL_ObjectData_set(&self->data[self->len], &other->data, other->type);
    self->type[self->len] = other->type;
    self->len++;
}
VL_Object* VL_Tuple_pop(VL_Tuple* self){
    VL_Object* out;

    if(self->len > 0){
        self->len--;
        out = VL_Object_new(self->type[self->len]);
        out->data = self->data[self->len];
    }
    else{
        out = VL_Object_new(VL_TYPE_NONE);
    }

    return out;
}

void VL_Tuple_print(const VL_Tuple* self){
    printf("(");
    if(self->len >= 1){
        VL_ObjectData_print(&self->data[0], self->type[0]);

        for(size_t i = 1; i < self->len; i++){
            printf(", ");
            VL_ObjectData_print(&self->data[i], self->type[i]);
        }
    }
    printf(")");
}