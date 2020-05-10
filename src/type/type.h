#pragma once
#include "../util.h"

#define MAPPING(X)                                      \
X(ADD)      X(SUB)      X(MUL)      X(DIV)              \
X(AND)      X(OR)       X(NOT)                          \
X(LTE)      X(GTE)      X(LT)       X(GT)               \
X(EQ)       X(NEQ)                                      \
X(FLOAT)    X(INT)      X(TUPLE)                        \
X(SET)                                                  \
X(PRINT)    X(INPUT)    X(TIME)                         \
X(DO)       X(IF)       X(WHILE)    X(INFIX)

#define NAME(X) VL_SYM_ ## X ,

typedef enum{ MAPPING(NAME) } VL_Symbol;
typedef size_t VL_Variable;

#undef MAPPING
#undef NAME

#define MAPPING(X)                      \
X(SYMBOL)       X(VARIABLE)      X(NONE)\
X(BOOL)         X(INT)          X(FLOAT)\
X(STRING)       X(TUPLE)        X(EXPR) \
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


typedef struct VL_ExprAtom VL_ExprAtom;
typedef struct VL_Expr VL_Expr;


typedef struct VL_SrcPos VL_SrcPos;

struct VL_Str{
    char* data;
    size_t len;
    size_t reserve_len;
};

struct VL_Tuple{
    VL_Object* data;
    size_t len;
    size_t reserve_len;    
};

struct VL_Object{
    union{
        VL_Symbol symbol;
        VL_Variable v_varid;

        VL_Bool v_bool;
        VL_Int v_int;
        VL_Float v_float;

        VL_Str* str;
        VL_Tuple* tuple;
        VL_Expr* expr;

        VL_ARC_Object* arc;
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

#undef NAME
#undef MAPPING

void VL_Type_perror(VL_Type type);
void VL_Type_print(const VL_Type type);
void VL_Symbol_print(const VL_Symbol symbol);

#include "str.h"
#include "tuple.h"
#include "expr.h"
#include "object.h"