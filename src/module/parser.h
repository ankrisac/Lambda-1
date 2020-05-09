#pragma once
#include "module.h"

#define VLT_ERR(SRC) VLT_BOLD VLT_RED SRC VLT_RESET

char VL_Module_parse_match_escape_seq(char in);

bool VL_Module_parse_token_symbol_label(char val);
bool VL_Module_parse_token_symbol_operator(char val);
bool VL_Module_parse_token_symbol(char val);

bool VL_Module_parse_Number(VL_Module* self, VL_ParseState* state);
bool VL_Module_parse_String(VL_Module* self, VL_ParseState* state);

bool VL_Module_parse_Label(VL_Module* self, VL_ParseState* state);
bool VL_Module_parse_Operator(VL_Module* self, VL_ParseState* state);

bool VL_Module_parse_FAtom(VL_Module* self, VL_ParseState* state);
bool VL_Module_parse_FExpr(VL_Module* self, VL_ParseState* state);

bool VL_Module_parse_AExpr(VL_Module* self,  VL_ParseState* begin, VL_ParseState* state);
bool VL_Module_parse_LExpr(VL_Module* self, VL_ParseState* state);
bool VL_Module_parse_BExpr(VL_Module* self, VL_ParseState* state);

bool VL_Module_parse_IExpr(VL_Module* self, VL_ParseState* state);
void VL_Module_parse_NExpr(VL_Module* self, VL_ParseState* state);

VL_ParseState VL_Module_parse_Lisp(VL_Module* self, VL_ParseState in);