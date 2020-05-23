#include "symmap.h"

void VL_SymMap_init(VL_SymMap* self, VL_SymMap* parent, size_t len){
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
VL_SymMap* VL_SymMap_new(VL_SymMap* parent, size_t len){
    VL_SymMap* self = malloc(sizeof* self);
    VL_SymMap_init(self, parent, len);
    return self;
}
void VL_SymMap_clear(VL_SymMap* self){
    for(size_t i = 0; i < self->len; i++){
        if(self->keys[i].data != NULL){
            VL_Str_clear(&self->keys[i]);
        }
        VL_Object_clear(&self->data[i]);
    }

    free(self->keys);
    free(self->data);
    free(self->hash);
}
void VL_SymMap_delete(VL_SymMap* self){
    VL_SymMap_clear(self);
    free(self);
}
size_t perturb(const VL_SymMap* self, const VL_Str* str, size_t str_hash){
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
void resize(VL_SymMap* self, size_t len){
    VL_SymMap new_map;
    
    VL_SymMap_init(&new_map, self->parent, len);
    new_map.elems = self->elems;

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
void VL_SymMap_insert_cstr(VL_SymMap* self, const VL_Str* str, size_t str_hash, VL_Object* value){
    if(self->elems > 0.7 * self->len){
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
        self->data[i] = *value;
    }
}
VL_Object* find(const VL_SymMap* self, const VL_Str* str, size_t str_hash){
    size_t i = perturb(self, str, str_hash);

    if(self->keys[i].data != NULL && VL_Str_cmp(&self->keys[i], str) == 0){
        return &self->data[i];
    }
    else if(self->parent != NULL){
        return find(self->parent, str, str_hash);
    }
    return NULL;
}

void VL_SymMap_insert(VL_SymMap* self, const VL_Symbol* sym, VL_Object* value){
    VL_SymMap_insert_cstr(self, sym->label, sym->hash, value);
}
VL_Object* VL_SymMap_find_str(const VL_SymMap* self, const VL_Str* str){
    return find(self, str, VL_Str_hash(str));
}
VL_Object* VL_SymMap_find(const VL_SymMap* self, const VL_Symbol* sym){
    return find(self, sym->label, sym->hash);
}

void VL_SymMap_print(const VL_SymMap* self){
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
    printf("|");
    if(self->parent != NULL){
        VL_SymMap_print(self->parent);
    }
    printf("}");
}
