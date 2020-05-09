#include "module.h"

VL_Module* VL_Module_from_file(const VL_Str* file_path);
void VL_Module_clear(VL_Module* self);
void VL_Module_delete(VL_Module* self);

VL_Str* VL_Module_get_ln(const VL_Module* self, VL_SrcPos s_begin, VL_SrcPos s_end, const char* highlight_color);

void VL_Module_push_err(VL_Module* self, const VL_ParseState* begin, VL_ParseState* end, VL_Str* str);
void VL_Module_push_err_str(VL_Module* self,  const VL_ParseState* begin, VL_ParseState* end, char* err);
void VL_Module_pop_err(VL_Module* self);
void VL_Module_pop_errors(VL_Module* self, size_t a, size_t b);


char VL_Module_peek(VL_Module* self, VL_ParseState* state);
void VL_Module_next(VL_ParseState* state, char chr);
char VL_Module_pop(VL_Module* self, VL_ParseState* state);

void VL_Module_print_error_stack(VL_Module* self);
void VL_Module_print_state(const VL_Module* self, VL_SrcPos begin, VL_SrcPos end);

bool VL_match_space(char val);
bool VL_match_digit(char val);
bool VL_Module_match_chr(VL_Module* self, VL_ParseState* state, char val);

void VL_Module_parse_Space(VL_Module* self, VL_ParseState* state);
bool VL_Module_parse_SpaceSep(VL_Module* self, VL_ParseState* state);