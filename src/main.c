#include <stdlib.h>
#include <stdbool.h>

#include <stdio.h>
#include <string.h>

#include "data/vl_object.h"
#include "parser/vl_parser.h"

#include <time.h>


#define PERF(N, EXPR, MSG)                                      \
{                                                               \
    clock_t t1 = clock();                                       \
    for(size_t i = 0; i < N; i++){                              \
        EXPR                                                    \
    }                                                           \
    clock_t t2 = clock();                                       \
    printf(MSG ": [%g]\n", (double)(t2 - t1)/CLOCKS_PER_SEC);   \
}


void VLP_parse(const char* path_name){
    VL_Parser self;
    
    self.error_stack = VL_Tuple_new(0);
    self.file_path = VL_Str_from_cstr(path_name);
    self.stream = VL_Str_from_file_cstr(path_name);

    VLP_State in = { .ok = true, .pos = 0 };
    VLP_State out = VLP_Lisp(&self, in);

    VLP_print_state(&self, &in, &in);
    printf("-----------------------------\n");

    if(out.ok){
        VL_Object* AST = out.val;

        printf(VLT_BOLD VLT_BLU);
        VL_Type_print(AST->type);    
        printf(VLT_RESET " " VLT_BOLD);

        VL_println(AST);
        printf(VLT_RESET);

        VL_Object_delete(AST);
    
        VLP_print_state(&self, &in, &out);
    }
    else{
        VLP_error_stack(&self);
    }
    
    VL_Str_delete(self.stream);
    VL_Str_delete(self.file_path);
    VL_Tuple_delete(self.error_stack);
}

int main(){
    VLP_parse("data/main.vl");

    return 0;
}