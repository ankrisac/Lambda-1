#include "symmap.h"

void VL_SymMap_init(VL_SymMap* self, VL_SymMap* parent, size_t len){
    self->len = len;
    self->elems = 0;

    self->keys = malloc(len * sizeof* self->keys);
    self->hash = malloc(len * sizeof* self->hash);
    self->data = malloc(len * sizeof* self->data);
    self->parent = parent;

    for(size_t i = 0; i < self->len; i++){
        self->keys[i].data = NULL;
    }
}
VL_SymMap* VL_SymMap_new(VL_SymMap* parent, size_t len){
    VL_SymMap* self = malloc(sizeof* self);
    VL_SymMap_init(self, parent, len);
    return self;
}
void VL_SymMap_clear(VL_SymMap* self){
    for(size_t i = 0; i < self->len; i++){
        if(self->keys[i].data != NULL){
            VL_Str_clear(&self->keys[i]);
            VL_Object_clear(&self->data[i]);
        }
    }

    free(self->keys);
    free(self->data);
    free(self->hash);
}
void VL_SymMap_delete(VL_SymMap* self){
    VL_SymMap_clear(self);
    free(self);
}
size_t __VL_SymMap_perturb(VL_SymMap* self, const VL_Str* str, size_t str_hash){
    size_t mask = self->len; 
    size_t j = str_hash;
    size_t perturb = str_hash;
    size_t i = j % mask;

    while(self->keys[i].data != NULL && VL_Str_cmp(&self->keys[i], str) != 0){
        j = ((j << 2) + j) + 1 + perturb;
        perturb = perturb >> 5;
        i = j % mask;
    }
    return i;
}
void __VL_SymMap_resize(VL_SymMap* self, size_t len){
    VL_SymMap new;
    VL_SymMap_init(&new, self->parent, len);

    for(size_t i = 0; i < self->len; i++){
        VL_Str* key = &self->keys[i];
        size_t hash = self->hash[i];

        if(key->data != NULL){
            size_t i = __VL_SymMap_perturb(self, key, hash);
            VL_Object* obj = &new.data[i];

            if(new.keys[i].data == NULL){
                new.keys[i] = *key;
                new.data[i] = *obj;
                new.elems++;
            }
            else if(VL_Str_cmp(&new.keys[i], key) == 0){
                new.data[i] = *obj;
            }
        }
    }
    *self = new;
}
void __VL_SymMap_insert(VL_SymMap* self, const VL_Str* str, size_t str_hash, VL_Object* value){
    if(self->elems > 0.7 * self->len){
        size_t new_len = self->len * 4;
        __VL_SymMap_resize(self, new_len);   
    }
    
    size_t i = __VL_SymMap_perturb(self, str, str_hash);
    if(self->keys[i].data == NULL){
        VL_Str_copy(&self->keys[i], str);
        self->data[i] = *value;
        self->elems++;
    }
    else if(VL_Str_cmp(&self->keys[i], str) == 0){
        self->data[i] = *value;
    }
}
VL_Object* __VL_SymMap_find(VL_SymMap* self, const VL_Str* str, size_t str_hash){
    size_t i = __VL_SymMap_perturb(self, str, str_hash);
    if(self->keys[i].data != NULL && VL_Str_cmp(&self->keys[i], str) == 0){
        return &self->data[i];
    }
    else if(self->parent != NULL){
        return __VL_SymMap_find(self->parent, str, str_hash);
    }
    return NULL;
}

void VL_SymMap_insert(VL_SymMap* self, const VL_Symbol* sym, VL_Object* value){
    __VL_SymMap_insert(self, sym->label, sym->hash, value);
}
VL_Object* VL_SymMap_find(VL_SymMap* self, const VL_Symbol* sym){
    return __VL_SymMap_find(self, sym->label, sym->hash);
}

void VL_SymMap_print(VL_SymMap* self){
        
    printf("{ ");
    if(self->len > 0){
        for(size_t i = 0; i < self->len; i++){
            if(self->keys[i].data != NULL){
                VL_Str_print(&self->keys[i]);
                printf(":");
                VL_Object_print(&self->data[i]);
                printf(",");
            }
        }
        
    }
    printf(" }");

    #undef P
}
