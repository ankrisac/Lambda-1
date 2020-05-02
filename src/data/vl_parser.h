#pragma once
#include "vl_parser_core.h"

char VLP_match_escape_seq(char in);

bool VLP_token_symbol_label(char val);
bool VLP_token_symbol_operator(char val);
bool VLP_token_symbol(char val);

bool VLP_Number(VL_Parser* self, VLP_State* state);
bool VLP_String(VL_Parser* self, VLP_State* state);

bool VLP_Label(VL_Parser* self, VLP_State* state);
bool VLP_Operator(VL_Parser* self, VLP_State* state);

bool VLP_FAtom(VL_Parser* self, VLP_State* state);
bool VLP_FExpr(VL_Parser* self, VLP_State* state);
bool VLP_LExpr(VL_Parser* self, VLP_State* state);
bool VLP_BExpr(VL_Parser* self, VLP_State* state);
bool VLP_IExpr(VL_Parser* self, VLP_State* state);
void VLP_NExpr(VL_Parser* self, VLP_State* state);

VLP_State VLP_Lisp(VL_Parser* self, VLP_State in);