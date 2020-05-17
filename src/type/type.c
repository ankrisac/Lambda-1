#include "type.h"

void VL_Type_print(const VL_Type type){
    #define P(ENUM, NAME) case VL_TYPE_ ## ENUM: printf(NAME); break;
    switch(type){
        P(SYMBOL, "Symbol")         P(KEYWORD, "Keyword") 
        P(NONE, "None")             P(BOOL, "Bool")             
        P(INT, "Int")               P(FLOAT, "Float")
        P(STRING, "String")         P(TUPLE, "Tuple") 
        P(EXPR, "Expr")             P(FUNCTION, "Function")
        P(ARC_STRONG, "Strong ARC") P(ARC_WEAK, "Weak ARC")
    }
    #undef P
}
void VL_Type_perror(const VL_Type type){
    printf("[" VLT_RED VLT_BOLD);
    VL_Type_print(type);
    printf(VLT_RESET "]");
}

void VL_Keyword_print(const VL_Keyword keyword){
    #define P(ENUM, NAME) case VL_SYM_ ## ENUM: printf(NAME); break;
    switch(keyword){
        P(FN, "fn")         P(SET, "def!")
        
        P(ADD, "+")         P(SUB, "-")         
        P(MUL, "*")         P(DIV, "/")
        P(AND, "&&")        P(OR, "||")         
        P(GTE, ">=")        P(LTE, "<=")        
        P(GT, ">")          P(LT, "<") 
        P(EQ, "==")         P(NEQ, "!=") 
        
        P(NOT, "not")
        
        P(DO, "do")         P(IF, "if")         P(WHILE, "while")
        P(FLOAT, "float")   P(INT, "int")       P(TUPLE, "tuple")
        P(PRINT, "print")   P(INPUT, "input")   P(TIME, "time")   
        
        P(INFIX, "infix")
        P(QUOTE, "quote")
        P(QUASIQUOTE, "quasiquote")
        P(UNQUOTE, "unquote")
        P(UNQUOTESPLICE, "unquote-splice")
    }
    #undef P
}
void VL_Keyword_perror(const VL_Keyword type){
    printf("[" VLT_RED VLT_BOLD);
    VL_Keyword_print(type);
    printf(VLT_RESET "]");
}

void VL_Symbol_init(VL_Symbol* self, VL_Str* label){
    self->label = label;
    self->hash = VL_Str_hash(self->label);
}
VL_Symbol* VL_Symbol_new(VL_Str* label){
    VL_Symbol* self = malloc(sizeof* self);
    VL_Symbol_init(self, label);
    return self;
}
void VL_Symbol_copy(VL_Symbol* self, VL_Symbol* src){
    self->label = VL_Str_clone(src->label);
    self->hash = src->hash;
}
VL_Symbol* VL_Symbol_clone(VL_Symbol* self){
    VL_Symbol* out = malloc(sizeof* out);
    VL_Symbol_copy(out, self);
    return out;
}

void VL_Symbol_clear(VL_Symbol* self){
    VL_Str_delete(self->label);
}
void VL_Symbol_delete(VL_Symbol* self){
    VL_Symbol_clear(self);
    free(self);
}


void VL_Symbol_print(const VL_Symbol* self){
    VL_Str_print(self->label);
}

void VL_Object_perror(const VL_Object* self){
    printf("[" VLT_RED VLT_BOLD);
    VL_Type_print(self->type);
    printf(VLT_RESET ":" VLT_RED VLT_BOLD);
    VL_Object_print(self);
    printf(VLT_RESET "]");
}