#ifndef __PARSER_H__
#define __PARSER_H__
#include "data_types.h"

#define VLT_BOLD  "\x1B[1m"

#define VLT_RED   "\x1B[31m"
#define VLT_GRN   "\x1B[32m"
#define VLT_YEL   "\x1B[33m"
#define VLT_BLU   "\x1B[34m"
#define VLT_MAG   "\x1B[35m"
#define VLT_CYN   "\x1B[36m"
#define VLT_WHT   "\x1B[37m"

#define VLT_RESET "\x1B[0m"

#define VLT_ERR(MSG) VLT_BOLD VLT_RED MSG VLT_RESET


typedef struct{
    VL_Str* file_path;
    VL_Str* stream;
    VL_Tuple* error_stack;
}VL_Parser;

typedef struct{
    size_t pos;
    size_t row;
    size_t col;

    bool ok;
    VL_Object* val;
}VLP_State;

char VLP_peek(VL_Parser* self, VLP_State* state);
void VLP_next(VLP_State* state, char chr);
char VLP_pop(VL_Parser* self, VLP_State* state);

VL_Str* VLP_get_ln(const VL_Parser* self, VLP_State s_begin, VLP_State s_end, 
    const char* cursor, const char* highlight_color);

void VLP_push_err(VL_Parser* self, const VLP_State* begin, VLP_State* end, VL_Str* str);
void VLP_push_err_str(VL_Parser* self,  const VLP_State* begin, VLP_State* end, char* err_msg);
void VLP_pop_err(VL_Parser* self);

void VLP_error_stack(VL_Parser* self);
void VLP_print_state(const VL_Parser* self, const VLP_State* begin, VLP_State* end);

char VLP_match_escape_seq(char in);

bool VLP_match(char val, char* set);
bool VLP_match_chr(VL_Parser* self, VLP_State* state, char val);

bool VLP_match_space(char val);
bool VLP_match_digit(char val);

void VLP_Space(VL_Parser* self, VLP_State* state);
bool VLP_SpaceSep(VL_Parser* self, VLP_State* state);
bool VLP_Number(VL_Parser* self, VLP_State* state);
bool VLP_String(VL_Parser* self, VLP_State* state);

#define TOKEN_SPACE(X) X(' ') X('\n') X('\t') X('\r') X('\f') X('\b')
#define TOKEN_SEP(X) X(',') X(';') X(':')
#define TOKEN_BRACKET(X) X('(') X(')') X('[') X(']') X('{') X('}')
#define TOKEN_OPERATOR(X)           \
X('=') X('<') X('>')                \
X('@') X('$') X('%') X('#') X('?')  \
X('!') X('|') X('&') X('^') X('~')  \
X('+') X('-') X('*') X('/')         \

bool VLP_token_symbol_label(char val);
bool VLP_token_symbol_operator(char val);
bool VLP_token_symbol(char val);

bool VLP_Label(VL_Parser* self, VLP_State* state);
bool VLP_Operator(VL_Parser* self, VLP_State* state);

bool VLP_FAtom(VL_Parser* self, VLP_State* state);
bool VLP_IAtom(VL_Parser* self, VLP_State* state);

void VLP_NExpr(VL_Parser* self, VLP_State* state);
bool VLP_FExpr(VL_Parser* self, VLP_State* state);
bool VLP_LExpr(VL_Parser* self, VLP_State* state);
bool VLP_BExpr(VL_Parser* self, VLP_State* state);
bool VLP_IExpr(VL_Parser* self, VLP_State* state);

VLP_State VLP_Lisp(VL_Parser* self, VLP_State in);
#endif