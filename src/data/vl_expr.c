#include "vl_expr.h"

void __VL_Expr_malloc(VL_Expr* self, size_t len){
    len = (len != 0) ? len : 1;
    self->data = malloc(len * sizeof* self->data);
    self->type = malloc(len * sizeof* self->type);
    self->p_begin = malloc(len * sizeof* self->p_begin);
    self->p_end = malloc(len * sizeof* self->p_end);
    self->reserve_len = len;
}
void __VL_Expr_realloc(VL_Expr* self, size_t len){
    self->data = realloc(self->data, len * sizeof* self->data);
    self->type = realloc(self->type, len * sizeof* self->type);
    self->p_begin = realloc(self->p_begin, len * sizeof* self->p_begin);
    self->p_end = realloc(self->p_end, len * sizeof* self->p_end);
    self->reserve_len = len;
}
void __VL_Expr_grow(VL_Expr* self){
    __VL_Expr_realloc(self, self->reserve_len * 2);
}

void VL_Expr_init(VL_Expr* self, size_t len){
    len = (len != 0) ? len : 1;
    self->len = 0;
    __VL_Expr_malloc(self, len);
}
VL_Expr* VL_Expr_new(size_t len){
    VL_Expr* out = malloc(sizeof(VL_Expr));
    VL_Expr_init(out, len);
    return out;
}
void VL_Expr_clear(VL_Expr* self){
    for(size_t i = 0; i < self->len; i++){
        VL_ObjectData_clear(&self->data[i], self->type[i]);
    }
    free(self->p_begin);
    free(self->p_end);
    free(self->data);
    free(self->type);
}
void VL_Expr_delete(VL_Expr* self){
    VL_Expr_clear(self);
    free(self);
}

void VL_Expr_set(VL_Expr* self, VL_Expr* src){
    self->len = src->len;
    __VL_Expr_malloc(self, self->len);
    
    for(size_t i = 0; i < self->len; i++){
        VL_ObjectData_set(&self->data[i], &src->data[i], src->type[i]);
        self->type[i] = src->type[i];
        self->p_begin[i] = src->p_begin[i];
        self->p_end[i] = src->p_end[i];
    }
}
VL_Expr* VL_Expr_copy(VL_Expr* self){
    VL_Expr* out = malloc(sizeof(VL_Expr));
    VL_Expr_set(out, self);
    return out;
}

void VL_Expr_append(VL_Expr* self, VL_Object* other, VLP_Pos begin, VLP_Pos end){
    if(self->len >= self->reserve_len){
        __VL_Expr_grow(self);
    }
    VL_ObjectData_set(&self->data[self->len], &other->data, other->type);
    self->type[self->len] = other->type;
    self->p_begin[self->len] = begin;
    self->p_end[self->len] = end;
    self->len++;
}
VL_Object* VL_Expr_pop(VL_Expr* self){
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

void VL_Expr_print(const VL_Expr* self){
    #define P(N) VL_ObjectData_print(&self->data[N], self->type[N]);
    printf("(");
    if(self->len >= 1){
        P(0)

        for(size_t i = 1; i < self->len; i++){
            printf(", ");
            P(i)
        }
    }
    printf(")");
    #undef P
}