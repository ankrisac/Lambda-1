#pragma once
#include "../util.h"

#define MAPPING(X)                              \
X(ADD)      X(SUB)      X(MUL)      X(DIV)      \
X(AND)      X(OR)       X(NOT)                  \
X(LTE)      X(GTE)      X(LT)       X(GT)       \
X(EQ)       X(NEQ)                              \
X(FLOAT)    X(INT)      X(TUPLE)                \
X(SET)                                          \
X(PRINT)    X(INPUT)    X(TIME)                 \
X(QUOTE)    X(QUASIQUOTE)                       \
X(UNQUOTE)  X(UNQUOTESPLICE)                    \
X(DO)       X(IF)       X(WHILE)    X(INFIX)    \
X(FN)

#define NAME(X) VL_SYM_ ## X ,

typedef enum{ MAPPING(NAME) } VL_Keyword;

#undef MAPPING
#undef NAME

#define MAPPING(X)          \
X(KEYWORD)      X(SYMBOL)   \
X(BOOL)         X(NONE)     \
X(INT)          X(FLOAT)    \
X(STRING)       X(TUPLE)    \
X(EXPR)         X(FUNCTION) \
X(ARC_STRONG)   X(ARC_WEAK) 

#define NAME(X) VL_TYPE_ ## X ,

typedef enum{ MAPPING(NAME) } VL_Type;

typedef bool VL_Bool;
typedef long long int VL_Int;
typedef double VL_Float;

typedef struct VL_Str VL_Str;
typedef struct VL_Tuple VL_Tuple;

typedef struct VL_ARC_Object VL_ARC_Object;
typedef union VL_ObjectData VL_ObjectData;
typedef struct VL_Object VL_Object;

typedef struct VL_Symbol VL_Symbol;
typedef struct VL_ExprAtom VL_ExprAtom;
typedef struct VL_Expr VL_Expr;

typedef struct VL_SrcPos VL_SrcPos;
typedef struct VL_SymMap VL_SymMap;
typedef struct VL_Function VL_Function;

struct VL_Str{
    char* data;
    size_t len;
    size_t reserve_len;
};

struct VL_Symbol{
    VL_Str* label;
    size_t hash;
};

struct VL_Tuple{
    VL_Object* data;
    size_t len;
    size_t reserve_len;    
};

struct VL_Object{
    union{
        VL_Keyword keyword;
        VL_Symbol* symbol;

        VL_Bool v_bool;
        VL_Int v_int;
        VL_Float v_float;

        VL_Str* str;
        VL_Tuple* tuple;
        VL_Expr* expr;

        VL_ARC_Object* arc;
        VL_Function* fn;
    }data;
    VL_Type type;
};
struct VL_ARC_Object{
    VL_Object data;

    size_t ref_count;
    size_t weak_ref_count;
};

struct VL_SrcPos{
    size_t pos;
    size_t row;
    size_t col;
};
struct VL_ExprAtom{
    VL_Object* val;
    VL_SrcPos begin;
    VL_SrcPos end;
    size_t module_id;
};
struct VL_Expr{
    VL_ExprAtom* data;
    size_t len;
    size_t reserve_len;    
};

struct VL_SymMap{
    size_t len;
    size_t elems;

    VL_Str* keys;
    size_t* hash;
    VL_Object* data;
    VL_SymMap* parent;
};

struct VL_Function{
    VL_SymMap* env;
    VL_Expr* args;
    VL_Object* body;
};

#undef NAME
#undef MAPPING

void VL_Type_perror(const VL_Type type);
void VL_Type_print(const VL_Type type);
void VL_Keyword_print(const VL_Keyword keyword);
void VL_Keyword_perror(const VL_Keyword keyword);

void VL_Symbol_init(VL_Symbol* self, VL_Str* label);
VL_Symbol* VL_Symbol_new(VL_Str* label);
void VL_Symbol_clear(VL_Symbol* self);
void VL_Symbol_delete(VL_Symbol* self);

void VL_Symbol_copy(VL_Symbol* self, VL_Symbol* src);
VL_Symbol* VL_Symbol_clone(VL_Symbol* self);

void VL_Symbol_print(const VL_Symbol* self);

void VL_Object_perror(const VL_Object* self);

#include "str.h"
#include "tuple.h"
#include "expr.h"
#include "function.h"
#include "symmap.h"
#include "arc_object.h"
#include "object.h"