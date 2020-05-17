#include "arc_object.h"

VL_ARC_Object* VL_ARC_Object_new(VL_Object* self){
    VL_ARC_Object* out = malloc(sizeof(VL_ARC_Object));
    out->ref_count = 1;
    out->weak_ref_count = 0;
    out->data = *self;
    return out;
}
VL_ARC_Object* VL_ARC_Object_strong_copy(VL_ARC_Object* self){
    self->ref_count++;
    return self;
}
VL_ARC_Object* VL_ARC_Object_weak_copy(VL_ARC_Object* self){    
    self->weak_ref_count++;
    return self;
}
void VL_ARC_Object_strong_clear(VL_ARC_Object* self){
    switch(self->ref_count){
        case 1:{
            VL_Object_clear(&self->data);
            if(self->weak_ref_count == 0){
                free(self);
            }
            break;
        }
        case 0:
            perror("ARC: Invalid reference count");
            break;
        default:
            self->ref_count--;
            break;
    }
}
void VL_ARC_Object_weak_clear(VL_ARC_Object* self){
    if(self->weak_ref_count > 0){
        self->weak_ref_count--;
        if(self->weak_ref_count == 0 && self->ref_count == 0){
            free(self);
        }
    }
}

void VL_ARC_Object_print(VL_ARC_Object* self){
    printf("{S%zu:W%zu}[", self->ref_count, self->weak_ref_count);
    VL_Object_print(&self->data);
    printf("]");
}
void VL_ARC_Object_print_type(VL_ARC_Object* self){
    printf("[S%zu:W%zu]", self->ref_count, self->weak_ref_count);
}
