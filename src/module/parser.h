#pragma once
#include "module.h"

#define VLT_ERR(SRC) VLT_BOLD VLT_RED SRC VLT_RESET

char VL_ModuleSrc_parse_match_escape_seq(char in);

bool VL_ModuleSrc_parse_token_symbol_label(char val);
bool VL_ModuleSrc_parse_token_symbol_operator(char val);
bool VL_ModuleSrc_parse_token_symbol(char val);

bool VL_ModuleSrc_parse_Number(VL_ModuleSrc* self, VL_ParseState* state);
bool VL_ModuleSrc_parse_String(VL_ModuleSrc* self, VL_ParseState* state);

bool VL_ModuleSrc_parse_Label(VL_ModuleSrc* self, VL_ParseState* state);
bool VL_ModuleSrc_parse_Operator(VL_ModuleSrc* self, VL_ParseState* state);

bool VL_ModuleSrc_parse_FAtom(VL_ModuleSrc* self, VL_ParseState* state);
bool VL_ModuleSrc_parse_FExpr(VL_ModuleSrc* self, VL_ParseState* state);

bool VL_ModuleSrc_parse_LExpr(VL_ModuleSrc* self, VL_ParseState* state);
bool VL_ModuleSrc_parse_BExpr(VL_ModuleSrc* self, VL_ParseState* state);

bool VL_ModuleSrc_parse_IExpr(VL_ModuleSrc* self, VL_ParseState* state);
void VL_ModuleSrc_parse_NExpr(VL_ModuleSrc* self, VL_ParseState* state);

VL_ParseState VL_ModuleSrc_parse_Lisp(VL_ModuleSrc* self, VL_ParseState in);