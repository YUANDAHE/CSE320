#include <stdio.h>
#include <stdlib.h>

#include "fliki.h"
#include "global.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if(global_options == HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    
    FILE *diff = fopen(diff_filename, "r");
    if (diff == NULL) {
        fprintf(stderr, "open diff file %s fail", diff_filename);
        return EXIT_FAILURE;
    }
    if (patch(stdin, stdout, diff) < 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS; 
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
