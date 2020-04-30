#include "data_types.h"

void __VL_Str_malloc(VL_Str* self, size_t len){
    len = (len != 0) ? len : 1;
    self->data = malloc(len * sizeof* self->data);
    self->reserve_len = len;
}
void __VL_Str_realloc(VL_Str* self, size_t len){
    self->data = realloc(self->data, len * sizeof* self->data);
    self->reserve_len = len;
}
void __VL_Str_grow(VL_Str* self){
    __VL_Str_realloc(self, self->reserve_len * 2);
}

void VL_Str_init(VL_Str* self, size_t len){
    self->len = 0;
    __VL_Str_malloc(self, len);
}
void VL_Str_clear(VL_Str* self){
    if(self->reserve_len != 0){
        free(self->data);
        self->reserve_len = 0;
        self->data = NULL;
    }
    self->len = 0;
}
void VL_Str_delete(VL_Str* self){
    VL_Str_clear(self);
    free(self);
}

VL_Str* VL_Str_new(size_t len){
    VL_Str* out = malloc(sizeof(VL_Str));
    VL_Str_init(out, len);
    return out;
}
VL_Str* VL_Str_copy(VL_Str* self){
    VL_Str* out = malloc(sizeof(VL_Str));
    VL_Str_set(out, self);
    return out;
}

void VL_Str_set(VL_Str* self, VL_Str* src){
    self->len = src->len;
    __VL_Str_malloc(self, src->len);
    memcpy(self->data, src->data, src->len);
}
void VL_Str_set_cstr(VL_Str* self, const char* str){
    self->len = strlen(str);
    __VL_Str_malloc(self, self->len);
    memcpy(self->data, str, self->len);
}

void VL_Str_append(VL_Str* self, const VL_Str* out){
    if(self->len + out->len >= self->reserve_len){
        __VL_Str_realloc(self, self->len + out->len);
    }
    memcpy(&self->data[self->len], out->data, out->len);
    self->len = self->len + out->len;
}
void VL_Str_append_char(VL_Str* self, char value){
    if(self->len >= self->reserve_len){
        __VL_Str_grow(self);
    }   
    self->data[self->len] = value;
    self->len++;
}
void VL_Str_append_cstr(VL_Str* self, const char* str){
    size_t str_len = strlen(str);
    if(self->len + str_len >= self->reserve_len){
        __VL_Str_realloc(self, self->len + str_len);
    }
    memcpy(&self->data[self->len], str, str_len);
    self->len = self->len + str_len;
}
void VL_Str_append_int(VL_Str* self, VL_Int val){
    VL_Str* i_str = VL_Str_from_int(val);
    VL_Str_append(self, i_str);
    VL_Str_delete(i_str);
}

void VL_Str_print(VL_Str* self){
    printf("%.*s", (int)self->len, self->data);
}
void VL_Str_print_internal(VL_Str* self){
    printf("[%zu:%zu][%.*s]", self->len, self->reserve_len, (int)self->len, self->data);
}

VL_Str* VL_Str_from_cstr(const char* str){
    VL_Str* out = malloc(sizeof(VL_Str));
    VL_Str_set_cstr(out, str);
    return out;
}
VL_Str* VL_Str_from_int(VL_Int val){
    VL_Str* out = VL_Str_new(1);
    
    bool neg = false;
    if(val < 0){
        val *= -1;
        neg = true;
    }
    
    if(val == 0){
        VL_Str_append_char(out, '0');    
    }
    else{
        while(val > 0){
            char chr = '0' + (char)(val % 10);
            VL_Str_append_char(out, chr);
            val = (val / 10);
        }
        
        if(neg){
            VL_Str_append_char(out, '-');
        }
    }

    VL_Str_reverse(out);
    return out;
}
VL_Str* VL_Str_from_file_cstr(const char* file_path){
    FILE* file = fopen(file_path, "r");
    VL_Str* out = VL_Str_new(0);
    
    if(file != NULL){
        char chr;
        while((chr = fgetc(file)) != EOF){
            VL_Str_append_char(out, chr);
        }
        fclose(file);    
    }
    else{
        printf("Error: %s is an invalid directory\n", file_path);
    }

    return out;
}
VL_Str* VL_Str_from_file(VL_Str* file_path){
    char* path_str = VL_Str_to_cstr(file_path);
    VL_Str* out = VL_Str_from_file_cstr(path_str);
    free(path_str);
    return out;
}

char* VL_Str_to_cstr(VL_Str* self){
    char* out = malloc((self->len + 1) * sizeof* out);
    memcpy(out, self->data, self->len);
    out[self->len] = '\0';
    return out;
}

int VL_Str_cmp_cstr(const VL_Str* self, const char* str){
    const char* j = str;
    for(size_t i = 0; i < self->len; i++, j++){
        if(*j == '\0'){
            return -1;
        }

        if(self->data[i] < *j){
            return 1;
        }
        else if(self->data[i] > *j){
            return -1;
        }     
    }
    if(*j == '\0'){
        return 0;
    }
    return 1;
}
void VL_Str_reverse(VL_Str* self){
    for(size_t i = 0, j = self->len - 1; i < j; i++, j--){
        char temp = self->data[i];
        self->data[i] = self->data[j];
        self->data[j] = temp;
    }
}



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

void VL_Symbol_print(const VL_Symbol symbol){
    #define P(ENUM, NAME) case VL_SYM_ ## ENUM: printf(NAME); break;
    switch(symbol){
        P(ADD, "+") P(SUB, "-") P(MUL, "*") P(DIV, "/")
        P(AND, "and") P(OR, "or") P(NOT, "not")
        P(GTE, ">=") P(LTE, "<=") P(GT, ">") P(LT, "<") P(EQ, "==")
        P(FLOAT, "float") P(INT, "int")
        P(PRINT, "print") P(INFIX, "infix")
    }
    #undef P
}
void VL_Type_print(const VL_Type type){
    #define P(ENUM, NAME) case VL_TYPE_ ## ENUM: printf(NAME); break;
    switch(type){
        P(SYMBOL, "Symbol") P(LITERAL, "Literal") P(NONE, "None")
        P(BOOL, "Bool") P(INT, "Int") P(FLOAT, "Float")
        P(STRING, "String") P(TUPLE, "Tuple")
        P(ARC_STRONG, "Strong ARC") P(ARC_WEAK, "Weak ARC")
    }
    #undef P
}

void VL_ObjectData_clear(VL_ObjectData* self, VL_Type type){
    #define C(ENUM, EXPR) case VL_TYPE_##ENUM: EXPR; break;
    switch(type){
        C(SYMBOL, ) C(LITERAL, )
        C(NONE, ) C(BOOL, ) C(INT, ) C(FLOAT, )
        C(STRING, VL_Str_delete(self->str))
        C(TUPLE, VL_Tuple_delete(self->tuple))
        C(ARC_STRONG, VL_ARC_Object_strong_clear(self->arc))
        C(ARC_WEAK, VL_ARC_Object_weak_clear(self->arc))
    }
    #undef C
}
void VL_ObjectData_set(VL_ObjectData* self, VL_ObjectData* src, VL_Type type){
    #define C(ENUM, EXPR) case VL_TYPE_ ## ENUM: EXPR; break;
    #define D(ENUM, TAG) case VL_TYPE_ ## ENUM: self->TAG = src->TAG; break;
    switch(type){
        D(SYMBOL, symbol) C(LITERAL, ) C(NONE, )
        D(BOOL, v_bool) D(INT, v_int) D(FLOAT, v_float)
        C(STRING, self->str = VL_Str_copy(src->str))
        C(TUPLE, self->tuple = VL_Tuple_copy(src->tuple))
        C(ARC_STRONG, self->arc = VL_ARC_Object_strong_copy(src->arc))
        C(ARC_WEAK, self->arc = VL_ARC_Object_weak_copy(src->arc))
    }
    #undef C
    #undef D
}
void VL_ObjectData_print(const VL_ObjectData* self, VL_Type type){
    #define C(ENUM, EXPR) case VL_TYPE_ ## ENUM: EXPR; break;
    switch(type){
        C(SYMBOL, VL_Symbol_print(self->symbol)) C(LITERAL, printf("Literal (WIP)"))
        C(NONE, printf("None")) C(BOOL, printf((self->v_bool) ? "True" : "False"))
        C(INT, printf("%lli", self->v_int)) C(FLOAT, printf("%f", self->v_float))
        C(STRING, VL_Str_print(self->str)) C(TUPLE, VL_Tuple_print(self->tuple))
        C(ARC_STRONG, printf("ARC Strong"); VL_ARC_Object_print(self->arc))
        C(ARC_WEAK, printf("ARC Weak"); VL_ARC_Object_print_type(self->arc))
    }
    #undef C
}


VL_Object* VL_Object_new(const VL_Type type){
    VL_Object* out = malloc(sizeof* out);
    out->type = type;
    return out;
}
void VL_Object_clear(VL_Object* self){
    VL_ObjectData_clear(&self->data, self->type);
    self->type = VL_TYPE_NONE;
}
void VL_Object_delete(VL_Object* self){
    VL_Object_clear(self);
    free(self);
}

#define VL_DEF(NAME, TYPE, TYPE_ENUM, EXPR)     \
VL_Object* NAME (TYPE val){                     \
    VL_Object* out = VL_Object_new(TYPE_ENUM);  \
    EXPR                                        \
    return out;                                 \
}
VL_DEF(VL_Object_from_bool, const VL_Bool, VL_TYPE_BOOL, out->data.v_bool = val; )
VL_DEF(VL_Object_from_int, const VL_Int, VL_TYPE_INT, out->data.v_int = val; )
VL_DEF(VL_Object_from_float, const VL_Float, VL_TYPE_FLOAT, out->data.v_float = val; )
VL_DEF(VL_Object_from_symbol, const VL_Symbol, VL_TYPE_SYMBOL, out->data.symbol = val; )

VL_DEF(VL_Object_from_cstr, const char*, VL_TYPE_STRING, out->data.str = VL_Str_from_cstr(val); )
VL_DEF(VL_Object_wrap_str, VL_Str*, VL_TYPE_STRING, out->data.str = val; )
VL_DEF(VL_Object_wrap_tuple, VL_Tuple*, VL_TYPE_TUPLE, out->data.tuple = val; )

#undef VL_FROM_DEF

VL_Object* VL_Object_make_ref(VL_Object* self){    
    VL_Object* out = VL_Object_new(VL_TYPE_ARC_STRONG);    
    out->data.arc = VL_ARC_Object_new(self);
    self->type = VL_TYPE_NONE;
    return out;
}
VL_Object* VL_Object_strong_share(VL_ARC_Object* self){    
    VL_Object* out = VL_Object_new(VL_TYPE_ARC_STRONG);    
    out->data.arc = self;
    out->data.arc->ref_count++;
    return out;
}
VL_Object* VL_Object_weak_share(VL_ARC_Object* self){
    VL_Object* out = VL_Object_new(VL_TYPE_ARC_WEAK);    
    out->data.arc = self;
    out->data.arc->weak_ref_count++;
    return out;
}

VL_Object* VL_Object_new_Tuple(const size_t len){
    VL_Object* out = VL_Object_new(VL_TYPE_TUPLE);
    out->data.tuple = VL_Tuple_new(len);
    return out;
}
void VL_Object_print(const VL_Object* self){
    VL_ObjectData_print(&self->data, self->type);
}





void VL_print(VL_Object* self){
    VL_Object_print(self);
}
void VL_println(VL_Object* self){
    VL_print(self);
    printf("\n");
}

void VL_Stack_init(VL_Stack* self){
    self->len = 1024;
    self->data = malloc(self->len * sizeof* self->data);
    self->type = malloc(self->len * sizeof* self->type);
    self->pos = 0;
}
void VL_Stack_clear(VL_Stack* self){
    for(size_t i = 1; i <= self->pos; i++){
        VL_ObjectData_clear(&self->data[i], self->type[i]);
    }
    free(self->data);
    free(self->type);
}

#define DEF(NAME, ARGS, TAG, TAG_ENUM, EXPR)\
void NAME(VL_Stack* self ARGS){             \
    self->pos++;                            \
    self->data[self->pos].TAG = EXPR;       \
    self->type[self->pos] = TAG_ENUM;       \
}

#define COMMA ,
DEF(VL_Stack_push_int,   COMMA VL_Int val, v_int, VL_TYPE_INT, val)
DEF(VL_Stack_push_float, COMMA VL_Float val, v_float, VL_TYPE_FLOAT, val)
DEF(VL_Stack_push_str,   COMMA const char* val, str, VL_TYPE_STRING, VL_Str_from_cstr(val))
DEF(VL_Stack_push_tuple, COMMA const size_t len, tuple, VL_TYPE_TUPLE, VL_Tuple_new(len))
#undef DEF

#define DEF(NAME, EXPR)                                                                 \
void NAME(VL_Stack* self){                                                              \
    if(self->type[self->pos - 1] == self->type[self->pos]){                             \
        switch(self->type[self->pos]){                                                  \
            case VL_TYPE_INT:                                                           \
                EXPR(self->data[self->pos - 1].v_int, self->data[self->pos].v_int)      \
                break;                                                                  \
            case VL_TYPE_FLOAT:                                                         \
                EXPR(self->data[self->pos - 1].v_float, self->data[self->pos].v_float)  \
                break;                                                                  \
            default:                                                                    \
                printf("TypeError!\n");                                                 \
                break;                                                                  \
        }                                                                               \
        self->pos--;                                                                    \
    }                                                                                   \
}

#define ADD(X, Y) X += Y;
#define SUB(X, Y) X -= Y;
#define MUL(X, Y) X *= Y;
#define DIV(X, Y) X /= Y;

DEF(VL_Stack_add, ADD)
DEF(VL_Stack_sub, SUB)
DEF(VL_Stack_mul, MUL)
DEF(VL_Stack_div, DIV)

#undef ADD
#undef SUB
#undef MUL
#undef DIV
#undef DEF

#define DEF(NAME, IN_TYPE, IN_ENUM, OUT_TYPE, OUT_ENUM, EXPR)                       \
void NAME(VL_Stack* self){                                                          \
    switch(self->type[self->pos]){                                                  \
        case OUT_ENUM: break;                                                       \
        case IN_ENUM:                                                               \
            self->type[self->pos] = OUT_ENUM;                                       \
            self->data[self->pos].OUT_TYPE = EXPR(self->data[self->pos].IN_TYPE);   \
            break;                                                                  \
        default:                                                                    \
            printf("TypeError!\n");                                                 \
            break;                                                                  \
    }                                                                               \
}
#define TO_INT(X) (VL_Int)X
#define TO_FLOAT(X) (VL_Float)X
DEF(VL_Stack_to_int, v_float, VL_TYPE_FLOAT, v_int, VL_TYPE_INT, TO_INT)
DEF(VL_Stack_to_float, v_int, VL_TYPE_INT, v_float, VL_TYPE_FLOAT, TO_FLOAT)
#undef TO_INT
#undef TO_FLOAT
#undef DEF

void VL_Stack_print(VL_Stack* self){
    VL_ObjectData_print(&self->data[self->pos], self->type[self->pos]);
}