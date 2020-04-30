#pragma once
#include "../data/vl_object.h"

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
void VLP_pop_errors(VL_Parser* self, size_t a, size_t b);

void VLP_error_stack(VL_Parser* self);
void VLP_print_state(const VL_Parser* self, const VLP_State* begin, VLP_State* end);

bool VLP_match_space(char val);
bool VLP_match_digit(char val);
bool VLP_match_chr(VL_Parser* self, VLP_State* state, char val);

void VLP_Space(VL_Parser* self, VLP_State* state);
bool VLP_SpaceSep(VL_Parser* self, VLP_State* state);