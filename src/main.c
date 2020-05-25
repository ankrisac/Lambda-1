#include <stdlib.h>
#include <stdbool.h>

#include <stdio.h>
#include <string.h>

#include "module/module.h"
#include <time.h>


void VIPER(const char* path_name){
    VL_Core* viper = VL_Core_new();
    
    VL_Str* path = VL_Str_from_cstr(path_name);
    VL_Core_repl(viper);
    VL_Str_delete(path);

    VL_Core_delete(viper);
}

#define PERF(N, EXPR, MSG)                                      \
{                                                               \
    clock_t t1 = clock();                                       \
    for(size_t i = 0; i < N; i++){                              \
        EXPR                                                    \
    }                                                           \
    clock_t t2 = clock();                                       \
    printf(MSG ": [%g]\n", (double)(t2 - t1)/CLOCKS_PER_SEC);   \
}

int main(){
    VIPER("data/main.vl");


    return 0;
}