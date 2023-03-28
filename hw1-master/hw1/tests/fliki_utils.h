#ifndef _FLIKI_UTILS_H_
#define _FLIKI_UTILS_H_

#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "fliki.h"
#include "global.h"

#define LIMITS "ulimit -f 100; ulimit -t 10; "

static char *progname = "bin/fliki";
/**
 * @brief Assert return status for any function
 *
 */
static void assert_func_expected_status(int status, int expected, const char *caller)
{
        cr_assert_eq(status, expected,
                        "Invalid return for %s. Got: %d | Expected: %d",
                        caller, status, expected);
}

/**
 * @brief Assert return status for any function - Inequality
 *
 */
static void assert_func_unexpected_status(int status, int expected, const char *caller)
{
        cr_assert_neq(status, expected,
                        "Invalid return for %s. Return should not be %d | Got: %d",
                        caller, expected, status);
}

/**
 * @brief Assert expected value for global opts
 *
 */
static void assert_fliki_expected_options(int option, int expected)
{
    cr_assert_eq(option, expected, "Invalid options settings. Got: 0x%x | Expected: 0x%x",
                        option, expected);
}

/**
 * @brief Assert expected pointer is NULL
 *
 */
static void assert_fliki_expected_null_ptr(void *ptr, const char* caller)
{
    cr_assert_null(ptr, "Test Failed: %s returned a non-NULL pointer, Expected NULL", caller);
}

/**
 * @brief Assert expected pointer is not NULL
 *
 */
static void assert_fliki_expected_notnull_ptr(void *ptr, const char* caller)
{
    cr_assert_not_null(ptr, "Test Failed: %s returned NULL pointer, Expected not-NULL", caller);
}

/**
 * @brief Assert expected value for String
 */
static void assert_fliki_expected_string(char *result, char *expected, const char* caller)
{
    cr_assert_null(strcmp(result, expected), "Test Failed: %s", caller);
}


/**
 * @brief Compare two hunks
 */
static int compare_hunk_types(HUNK_TYPE type, HUNK_TYPE lib_type)
{
    int res = 1;
    switch(type) {
        case HUNK_APPEND_TYPE:
            res = (lib_type == HUNK_APPEND_TYPE) ? 1 : 0;
            break;
        case HUNK_DELETE_TYPE:
            res = (lib_type == HUNK_DELETE_TYPE) ? 1 : 0;
            break;
        case HUNK_CHANGE_TYPE:
            res = (lib_type == HUNK_CHANGE_TYPE) ? 1 : 0;
            break;
        default:
            res = 0;
    }
    return res;
}
static int compare_hunks(HUNK *hp1, HUNK *hp2)
{
    int res = 0;
    res =  (hp1->new_start == hp2->new_start) ? 0 : 1;
    res = res | ((hp1->new_end == hp2->new_end) ? 0 : 1);
    res = res | ((hp1->old_start == hp2->old_start) ? 0 : 1);
    res = res | ((hp1->old_end == hp2->old_end) ? 0 : 1);
    res = res | ((hp1->serial == hp2->serial) ? 0 : 1);
    res = res | (compare_hunk_types(hp1->type,hp2->type) ? 0 : 1);
    return res;
}

/**
 * Advances given file pointer past a certain number of '\n's.
 * ex. if line = 2, it will consume data from the fp until
 * it has read 2 '\n' chars, thus you are now on line 3.
 *
 * @return 0 if successful, -1 if EOF was encountered.
 */
static int skip_lines(FILE *fp, int lines) {
    int n = 0;
    int rv;
    while (n < lines) {
        rv = fgetc(fp);
        if (rv == EOF) return -1;
        if (rv == '\n') ++n;
    }
    return 0;
}

/**
 * Advances a given file pointer past a certain number of characters.
 * ex. if chars = 2, it will consume 2 chars from the stream.
 *
 * @return 0 if successful, -1 if EOF was encountered.
 */
static int skip_chars(FILE *fp, int chars) {
    int i = 0;
    while (i++ < chars) {
        if (fgetc(fp) == EOF) return -1;
    }
    return 0;
}

/**
 * Compares the output of each file stream.
 *
 * @param lines_to_compare The amount of lines to compare.
 * @param f_stream This stream will be passed to fgetc() for output.
 *  This should be initially be pointing at the start of the first line *after* the header.
 *  At the start of each line, the first 2 chars from this stream will be ignored (corresponding to "> ").
 * @param getc_stream This stream will be passed to hunk_getc() for output.
 *  This should initially be pointing at the start of the first line *after* the header.
 * @param hunk The hunk structure associated with the lines.
 * @param expected Storage for expected result. Will contain expected char in case of mismatch.
 * @param received Storage for received result. Will contain received char in case of mismatch.
 * @return 0 if the outputs match, -1 if it reached end-of-line but otherwise matched, 1 if they do not match, 2 if there was an error
 * That is, return values <= 0 indicate a success, and > 0 indicate a fail.
 */
static int compare_hunk_lines(int lines_to_compare, FILE *f_stream, FILE *getc_stream, HUNK *hunk, int *expected, int *received) {
    for (int cur_line = 0; cur_line < lines_to_compare; cur_line++) {
        if (skip_chars(f_stream, 2)) return 2;  // Skip "> "
        do {
            *expected = fgetc(f_stream);
            *received = hunk_getc(hunk, getc_stream);
            if (*received == EOF) return 1;  // Prevents infinite loop; hunk_getc() should never return EOF.
            if (*expected != *received)
                return (*expected == EOF && *received == ERR) ? -1 : 1;
        } while (*expected != '\n');
    }
    return 0;
}

/**
 * @brief Advances file pointer and hunk info count number of times forwards
 */
static void advance_student_hunk_state(HUNK *hp, FILE *fp, unsigned int count) {
    // Reading student code until isLibFunction is reached.
    hunk_next(hp, fp);
    for (int i = 0; i < count; i++) {
        int newChar = hunk_getc(hp, fp);
    }
}

/**
 * @brief Advances file pointer and hunk info until EOS is returned.
 *
 * @return number of calls it took until EOS was obtained.
 */
static int advance_lib_hunk_state(HUNK *hp, FILE *fp) {
    // Using solution code until EOS is reached.
    int i = 0, newChar = 0, firstEOSReached = 0;
    lib_hunk_next(hp, fp);
    do {
        if (newChar == EOS) firstEOSReached = 1;

        newChar = lib_hunk_getc(hp, fp);
        i += 1;
    } while(newChar != EOS || (!firstEOSReached && hp->type == HUNK_CHANGE_TYPE));

    return i;
}

/**
 * @brief Captures output of the lib_hunk_show() function.
 */
static char *capture_lib_hunk_show_output(HUNK *hp) {
    // Creating output chunk from open_memstream and checking solution.
    char *lib_buf;
    size_t lib_len;
    FILE *lib_ptr = open_memstream(&lib_buf, &lib_len);
    lib_hunk_show(hp, lib_ptr);
    fclose(lib_ptr);

    return lib_buf;
}

/**
 * @brief Captures output of the student's hunk_show() function.
 */
static char *capture_student_hunk_show_output(HUNK *hp) {
    // Creating output chunk from open_memstream and checking solution.
    char *buf;
    size_t len;
    FILE *ptr = open_memstream(&buf, &len);
    hunk_show(hp, ptr);
    fclose(ptr);

    return buf;
}

/**
 * @brief comnpare answer with student's outputfile
 */

static void check_patch_output(char *answer, char *out, char* testname){
    //diff output
    char* cmp;
    cmp = malloc(strlen(answer)+strlen(out)+1+5); //allocate 5 space for  "cmp " and " "

    strcpy(cmp, "cmp "); /* copy name into the new var */
    strcat(cmp, answer);
    strcat(cmp, " ");
    strcat(cmp, out);
    //char *cmp = "cmp rsrc/file2 test_output/patch_basic.out";
    //fprintf(stdout, "in defined func : %s", cmp);
    int return_code = WEXITSTATUS(system(cmp));
    //fprintf(stdout, " return code: %d\n", return_code);
    if (return_code)
        cr_log_error("patch applied incorrectly");
    assert_func_expected_status(return_code, EXIT_SUCCESS, testname);

    free(cmp);

}
#endif
