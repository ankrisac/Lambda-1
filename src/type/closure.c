#include "closure.h"

void init(VL_Closure* self, VL_Closure* parent, size_t len){
    self->ref_count = 1;
    self->len = (len > 0) ? len : 1;
    self->elems = 0;

    self->keys = malloc(self->len * sizeof* self->keys);
    self->hash = malloc(self->len * sizeof* self->hash);
    self->data = malloc(self->len * sizeof* self->data);
    self->parent = parent;

    for(size_t i = 0; i < self->len; i++){
        self->keys[i].data = NULL;
        self->data[i].type = VL_TYPE_NONE;
    }
}
VL_Closure* VL_Closure_new(VL_Closure* parent, size_t len){
    VL_Closure* self = malloc(sizeof* self);
    init(self, parent, len);
    return self;
}
VL_Closure* VL_Closure_share(VL_Closure* self){
    if(self != NULL && self->ref_count > 0){
        self->ref_count++;
        return self;
    }
    return NULL;
}

void VL_Closure_force_delete(VL_Closure* self){
    for(size_t i = 0; i < self->len; i++){
        if(self->keys[i].data != NULL){
            VL_Str_clear(&self->keys[i]);
        }
        VL_Object_clear(&self->data[i]);
    }

    free(self->keys);
    free(self->hash);
    free(self->data);
    free(self);
}

void VL_Closure_delete(VL_Closure* self){
    if(self->ref_count > 0){
        self->ref_count--;

        if(self->ref_count == 0){
            VL_Closure_force_delete(self);
        }
    }
}

size_t perturb(const VL_Closure* self, const VL_Str* str, size_t str_hash){
    size_t mask = self->len; 
    size_t j = str_hash;
    size_t perturb = str_hash;
    size_t index = j % mask;

    while(self->keys[index].data != NULL && VL_Str_cmp(&self->keys[index], str) != 0){
        j = ((j << 2) + j) + 1 + perturb;
        perturb = perturb >> 5;
        index = j % mask;
    }
    return index;
}
void resize(VL_Closure* self, size_t len){
    VL_Closure new_map;
    
    init(&new_map, self->parent, len);
    new_map.elems = self->elems;
    new_map.ref_count = self->ref_count;

    for(size_t i = 0; i < self->len; i++){
        VL_Str* key = &self->keys[i];
        size_t hash = self->hash[i];

        if(key->data != NULL){
            size_t j = perturb(&new_map, key, hash);

            new_map.keys[j] = *key;
            new_map.hash[j] = hash;
            new_map.data[j] = self->data[i];
        }
    }
    free(self->keys);
    free(self->data);
    free(self->hash);
    *self = new_map;
}
void VL_Closure_insert_cstr(VL_Closure* self, const VL_Str* str, size_t str_hash, VL_Object* value){
    if(10 * self->elems > 7 * self->len){
        size_t new_len = self->len * 4;
        resize(self, new_len);   
    }
    
    size_t i = perturb(self, str, str_hash);
    if(self->keys[i].data == NULL){
        VL_Str_copy(&self->keys[i], str);
        VL_Object_clear(&self->data[i]);
        
        self->data[i] = *value;
        self->hash[i] = str_hash;
        self->elems++;
    }
    else if(VL_Str_cmp(&self->keys[i], str) == 0){
        VL_Object_clear(&self->data[i]);
        VL_Object_copy(&self->data[i], value);
    }
}
VL_Object* find(const VL_Closure* self, const VL_Str* str, size_t str_hash){
    size_t i = perturb(self, str, str_hash);

    if(self->keys[i].data != NULL && VL_Str_cmp(&self->keys[i], str) == 0){
        return &self->data[i];
    }
    else if(self->parent != NULL){
        return find(self->parent, str, str_hash);
    }
    return NULL;
}

void VL_Closure_insert(VL_Closure* self, const VL_Symbol* sym, VL_Object* value){
    VL_Closure_insert_cstr(self, sym->label, sym->hash, value);
}
VL_Object* VL_Closure_find_str(const VL_Closure* self, const VL_Str* str){
    return find(self, str, VL_Str_hash(str));
}
VL_Object* VL_Closure_find(const VL_Closure* self, const VL_Symbol* sym){
    return find(self, sym->label, sym->hash);
}

void VL_Closure_print(const VL_Closure* self){
    bool before = false;
    
    printf("{");
    if(self->len > 0){
        for(size_t i = 0; i < self->len; i++){
            
            if(self->keys[i].data != NULL){
                if(before){
                    printf(", ");
                }
                else{
                    before = true;
                }

                VL_Str_print(&self->keys[i]);
                printf(":");
                VL_Object_print(&self->data[i]);
            }
        }
        
    }
    printf("}");
}
