#include "vl_type.h"

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
