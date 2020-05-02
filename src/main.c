#include <stdlib.h>
#include <stdbool.h>

#include <stdio.h>
#include <string.h>

#include "data/vl_object.h"
#include "data/vl_parser.h"
#include "data/vl_compiler.h"

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

void VIPER(const char* path_name){
    VL_Str* path = VL_Str_from_cstr(path_name);
    VL_Compiler* compiler = VL_Compiler_new(path);

    VLC_compile(compiler);

    VL_Compiler_delete(compiler);
    VL_Str_delete(path);
}

int main(){
    VIPER("data/main.vl");

    return 0;
}