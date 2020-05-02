#include <stdlib.h>
#include <stdbool.h>

#include <stdio.h>
#include <string.h>

#include "data/vl_object.h"
#include "data/vl_parser.h"

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
void VL_validate_data(VL_Parser* parser, VL_ObjectData* obj, VL_Type type){
    if(type == VL_TYPE_EXPR){
        VL_Expr* expr = obj->expr;
        for(size_t i = 0; i < expr->len; i++){
            //if(!expr->p_begin[i].ok || !expr->p_end[i].ok){
            //    printf(VLT_RED "Compile Error: ");
            //}
            //else{
            //    printf(VLT_BLU);
            //}
   
            VLP_print_state(parser, expr->p_begin[i], expr->p_end[i]);
            VL_validate_data(parser, &expr->data[i], expr->type[i]);
            printf("\n");
        }
        printf("[");
        printf(VLT_BLU);
        VL_Type_print(type);
        printf(VLT_RESET "] = " VLT_BOLD);
        VL_ObjectData_print(obj, type);            

    }
    else{
        printf("[");
        printf(VLT_GRN);
        VL_Type_print(type);
        printf(VLT_RESET "] = " VLT_BOLD);
        VL_ObjectData_print(obj, type);        
        printf("\n");    
    }
}
void VL_validate(VL_Parser* parser, VL_Object* obj){
    VL_validate_data(parser, &obj->data, obj->type);
    printf("\n");
}

void VLP_parse(const char* path_name){
    VL_Parser parser;
    
    parser.error_stack = VL_Tuple_new(0);
    parser.file_path = VL_Str_from_cstr(path_name);
    parser.stream = VL_Str_from_file_cstr(path_name);

    VLP_State in = { .p.ok = true, .p.pos = 0, .p.row = 0, .p.col = 0 };
    VLP_State out = VLP_Lisp(&parser, in);

    VLP_print_state(&parser, in.p, in.p);
    printf("-----------------------------\n");

    if(out.p.ok){
        VL_Object* AST = out.val;

        printf(VLT_BOLD VLT_BLU);
        VL_Type_print(AST->type);    
        printf(VLT_RESET " " VLT_BOLD);

        VL_println(AST);
        printf(VLT_RESET);

        printf("--- Compiling! ---\n");
        VL_validate(&parser, AST);

        VL_Object_delete(AST);

        printf("--- Done! ---\n");    
        VLP_print_state(&parser, in.p, out.p);
    }
    else{
        VLP_error_stack(&parser);
    }
    
    VL_Str_delete(parser.stream);
    VL_Str_delete(parser.file_path);
    VL_Tuple_delete(parser.error_stack);
}

int main(){
    VLP_parse("data/main.vl");

    return 0;
}