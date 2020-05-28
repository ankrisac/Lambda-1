#include "expr.h"

void VL_ExprAtom_init(VL_ExprAtom* self){
    self->val = VL_Object_new(VL_TYPE_NONE);
    self->module_id = 0;
}
VL_ExprAtom* VL_ExprAtom_new(){
    VL_ExprAtom* out = malloc(sizeof* out);
    VL_ExprAtom_init(out);
    return out;
}
void VL_ExprAtom_clear(VL_ExprAtom* self){
    VL_Object_delete(self->val);
}
void VL_ExprAtom_delete(VL_ExprAtom* self){
    VL_ExprAtom_clear(self);
    free(self);
}

void VL_ExprAtom_copy(VL_ExprAtom* self, const VL_ExprAtom* src){
    self->val = VL_Object_clone(src->val);
    self->begin = src->begin;
    self->end = src->end;
    self->module_id = src->module_id;
}
VL_ExprAtom* VL_ExprAtom_clone(const VL_ExprAtom* self){
    VL_ExprAtom* out = malloc(sizeof* out);
    VL_ExprAtom_copy(out, self);
    return out;
}
void VL_ExprAtom_move(VL_ExprAtom* self, VL_ExprAtom* src){
    self->val = src->val;
    self->begin = src->begin;
    self->end = src->end;
    self->module_id = src->module_id;
}
VL_ExprAtom* VL_ExprAtom_wrap(VL_ExprAtom* self){
    VL_ExprAtom* out = malloc(sizeof* out);
    VL_ExprAtom_move(out, self);
    return out;
}

void VL_ExprAtom_print(const VL_ExprAtom* self){
    VL_Object_repr(self->val);
}
void VL_ExprAtom_repr(const VL_ExprAtom* self){
    VL_Object_repr(self->val);
}
void VL_ExprAtom_print_type(const VL_ExprAtom* self){
    VL_Type_print(self->val->type);
}

void expr_allocate(VL_Expr* self, size_t len){
    len = (len != 0) ? len : 1;
    self->data = malloc(len * sizeof* self->data);
    self->reserve_len = len;
}
void expr_reserve(VL_Expr* self, size_t len){
    self->data = realloc(self->data, len * sizeof* self->data);
    self->reserve_len = len;
}
void expr_grow(VL_Expr* self){
    expr_reserve(self, self->reserve_len * 2);
}

void VL_Expr_init(VL_Expr* self, size_t len){
    len = (len != 0) ? len : 1;
    self->len = 0;
    expr_allocate(self, len);
}
VL_Expr* VL_Expr_new(size_t len){
    VL_Expr* out = malloc(sizeof(VL_Expr));
    VL_Expr_init(out, len);
    return out;
}
void VL_Expr_clear(VL_Expr* self){
    for(size_t i = 0; i < self->len; i++){
        VL_ExprAtom_clear(&self->data[i]);
    }
    free(self->data);
}
void VL_Expr_delete(VL_Expr* self){
    VL_Expr_clear(self);
    free(self);
}

void VL_Expr_copy(VL_Expr* self, const VL_Expr* src){
    self->len = src->len;
    expr_allocate(self, self->len);

    for(size_t i = 0; i < self->len; i++){
        VL_ExprAtom_copy(&self->data[i], &src->data[i]);
    }
}
VL_Expr* VL_Expr_clone(const VL_Expr* self){
    VL_Expr* out = malloc(sizeof(VL_Expr));
    VL_Expr_copy(out, self);
    return out;
}

void VL_Expr_append_Object(VL_Expr* self, VL_Object* other, VL_SrcPos begin, VL_SrcPos end, size_t module_id){
    if(self->len >= self->reserve_len){
        expr_grow(self);
    }
    self->data[self->len] = (VL_ExprAtom){ .val = other, .begin = begin, .end = end, .module_id = module_id };
    self->len++;
}
void VL_Expr_append(VL_Expr* self, VL_ExprAtom* other){
    if(self->len >= self->reserve_len){
        expr_grow(self);
    }
    self->data[self->len] = *other;
    self->len++;
}
void VL_Expr_mappend_expr(VL_Expr* self, VL_Expr* other){
    if(self->len + other->len >= self->reserve_len){
        expr_reserve(self, self->len + other->len);
    }
    for(size_t i = 0; i < other->len; i++){
        self->data[self->len + i].val = other->data[i].val;
    }
    self->len += other->len;
}
void VL_Expr_append_expr(VL_Expr* self, const VL_Expr* other){
    if(self->len + other->len >= self->reserve_len){
        expr_reserve(self, self->len + other->len);
    }
    for(size_t i = 0; i < other->len; i++){
        VL_Object_copy(self->data[self->len + i].val, other->data[i].val);
    }
    self->len += other->len;
}

VL_Object* VL_Expr_pop_Object(VL_Expr* self){
    if(self->len > 0){
        self->len--;
        return self->data[self->len].val;
    }
    return VL_Object_new(VL_TYPE_NONE);
}
VL_ExprAtom* VL_Expr_pop(VL_Expr* self){
    if(self->len > 0){
        self->len--;
        return VL_ExprAtom_wrap(&self->data[self->len]);
    }
    return VL_ExprAtom_new();
}

const VL_ExprAtom* VL_Expr_get(const VL_Expr* self, size_t i){
    return &self->data[i];
}
const VL_ExprAtom* VL_Expr_rget(const VL_Expr* self, size_t i){
    return &self->data[self->len - i - 1];
}
VL_ExprAtom* VL_Expr_mget(const VL_Expr* self, size_t i){
    return &self->data[i];
}
VL_ExprAtom* VL_Expr_mrget(const VL_Expr* self, size_t i){
    return &self->data[self->len - i - 1];
}

void VL_Expr_set(VL_Expr* self, size_t i, VL_ExprAtom* value){
    self->data[i] = *value;
}
void VL_Expr_rset(VL_Expr* self, size_t i, VL_ExprAtom* value){
    self->data[self->len - i - 1] = *value;
}

void VL_Expr_print(const VL_Expr* self){
    printf("(");
    if(self->len >= 1){
        VL_ExprAtom_print(&self->data[0]);
    
        for(size_t i = 1; i < self->len; i++){
            printf(" ");
            VL_ExprAtom_print(&self->data[i]);    
        }
    }
    printf(")");
}
void VL_Expr_repr(const VL_Expr* self){
    printf("(");
    if(self->len >= 1){
        VL_ExprAtom_repr(&self->data[0]);
    
        for(size_t i = 1; i < self->len; i++){
            printf(" ");
            VL_ExprAtom_repr(&self->data[i]);    
        }
    }
    printf(")");
}
void VL_Expr_print_type(const VL_Expr* self){
    printf("(");
    if(self->len >= 1){
        VL_ExprAtom_print_type(&self->data[0]);
    
        for(size_t i = 1; i < self->len; i++){
            printf(" ");
            VL_ExprAtom_print_type(&self->data[i]);    
        }
    }
    printf(")");
}