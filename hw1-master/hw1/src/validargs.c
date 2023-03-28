#include <stdlib.h>

#include "fliki.h"
#include "global.h"
#include "debug.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 * @modifies global variable "diff_filename" to point to the name of the file
 * containing the diffs to be used.
 */

static int mystrcmp(const char* str1, const char* str2) {  
    while ((*str1) && (*str1 == *str2)) {  
        str1++;  
        str2++;  
    }

    if (*str1 > *str2) { 
        return 1;
    } else if (*str1 < *str2) {
        return -1;  
    }

    return 0;
}  

int validargs(int argc, char **argv) {
    if (argc < 2) {
        return -1;
    }
    
    if (mystrcmp(*(argv+1), "-h") == 0) {
        global_options = HELP_OPTION;
        return 0;
    }
    
    for (int i = 1; i < argc; i++) {
        if (mystrcmp(*(argv+i), "-n") == 0) {
            global_options |= NO_PATCH_OPTION;
        } else if (mystrcmp(*(argv+i), "-q") == 0) {
            global_options |= QUIET_OPTION;
        } else {
            if (i < argc - 1) {
                return -1;
            }
            if (i == argc - 1) {
                diff_filename = *(argv+i);
                return 0;
            }
        }
    }

   return -1;
}
