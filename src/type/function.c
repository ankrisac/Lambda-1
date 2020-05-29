#include "function.h"

void VL_Function_init(VL_Function* self, VL_Closure* parent_env, VL_Expr* args, VL_Object* body){
    self->env = parent_env;
    self->args = args;
    self->body = body;
    self->is_macro = false;
}
VL_Function* VL_Function_new(VL_Closure* parent_env, VL_Expr* args, VL_Object* body){
    VL_Function* self = malloc(sizeof* self);
    VL_Function_init(self, parent_env, args, body);
    return self;
}
void VL_Function_clear(VL_Function* self){
    if(self->env != NULL){
        VL_Closure_delete(self->env);
    }
    VL_Expr_delete(self->args);
    VL_Object_delete(self->body);
}
void VL_Function_delete(VL_Function* self){
    VL_Function_clear(self);
    free(self);
}

void VL_Function_copy(VL_Function* self, const VL_Function* src){
    self->env = VL_Closure_new(VL_Closure_share(src->env->parent), src->env->elems);
    self->args = VL_Expr_clone(src->args);
    self->body = VL_Object_clone(src->body);
}
VL_Function* VL_Function_clone(const VL_Function* self){
    VL_Function* out = malloc(sizeof* out);
    VL_Function_copy(out, self);
    return out;
}

const VL_Symbol* VL_Function_getArg(const VL_Function* self, const size_t i){
    return self->args->data[i].val->data.symbol;
}

void VL_Function_print(const VL_Function* self){
    if(self->is_macro){
        printf("macro ");
    }
    else{
        printf("fn ");
    }
    VL_Expr_print(self->args);
    printf(" ");
    VL_Object_print(self->body);
}
void VL_Function_repr(const VL_Function* self){
    if(self->is_macro){
        printf("macro ");
    }
    else{
        printf("fn ");
    }
    VL_Expr_repr(self->args);
    printf(" ");
    VL_Object_repr(self->body);
}
void VL_Function_print_type(const VL_Function* self){
    if(self->is_macro){
        printf("macro ");
    }
    else{
        printf("fn ");
    }
    VL_Expr_print(self->args);
}