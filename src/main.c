#include <stdlib.h>
#include <stdbool.h>

#include <stdio.h>
#include <string.h>

#include "module/core.h"
#include <time.h>

int main(int argc, char** argv){
    if(argc == 1){
        VL_Core* viper = VL_Core_new();
        VL_Core_repl(viper);
        VL_Core_delete(viper);
    }
    else if(argc == 2){
        VL_Core* viper = VL_Core_new();
    
        VL_Str* path = VL_Str_from_cstr(argv[1]);  

        VL_Core_exec_file(viper, path);

        VL_Str_delete(path);
        VL_Core_delete(viper);
    }
    else{
        printf(VLT_ERR("Error") ": expected only 0-1 files\n");
    }

    return 0;
}