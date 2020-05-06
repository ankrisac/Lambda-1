#include "module.h"

VL_ModuleSrc* VL_ModuleSrc_new(const VL_Str* file_path);
void VL_ModuleSrc_clear(VL_ModuleSrc* self);
void VL_ModuleSrc_delete(VL_ModuleSrc* self);

VL_Str* VL_ModuleSrc_get_ln(const VL_ModuleSrc* self, VL_SrcPos s_begin, VL_SrcPos s_end, const char* highlight_color);

void VL_ModuleSrc_push_err(VL_ModuleSrc* self, const VL_ParseState* begin, VL_ParseState* end, VL_Str* str);
void VL_ModuleSrc_push_err_str(VL_ModuleSrc* self,  const VL_ParseState* begin, VL_ParseState* end, char* err);
void VL_ModuleSrc_pop_err(VL_ModuleSrc* self);
void VL_ModuleSrc_pop_errors(VL_ModuleSrc* self, size_t a, size_t b);


char VL_ModuleSrc_peek(VL_ModuleSrc* self, VL_ParseState* state);
void VL_ModuleSrc_next(VL_ParseState* state, char chr);
char VL_ModuleSrc_pop(VL_ModuleSrc* self, VL_ParseState* state);

void VL_ModuleSrc_print_error_stack(VL_ModuleSrc* self);
void VL_ModuleSrc_print_state(const VL_ModuleSrc* self, VL_SrcPos begin, VL_SrcPos end);

bool VL_match_space(char val);
bool VL_match_digit(char val);
bool VL_ModuleSrc_match_chr(VL_ModuleSrc* self, VL_ParseState* state, char val);

void VL_ModuleSrc_parse_Space(VL_ModuleSrc* self, VL_ParseState* state);
bool VL_ModuleSrc_parse_SpaceSep(VL_ModuleSrc* self, VL_ParseState* state);