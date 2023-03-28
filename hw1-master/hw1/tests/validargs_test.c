#include "fliki_utils.h"

/**
 * @brief No Diff Filename Provided
 *
 */
Test(validargs_suite, no_diff_file_given, .timeout = 5) {
    char *argv[] = {progname, "-n", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    int opt = global_options;
    int exp_opt = 0;

    assert_func_expected_status(ret, exp_ret, "no_diff_file_given");
    //assert_fliki_expected_options(opt, exp_opt);
    //assert_fliki_expected_null_ptr(diff_filename, "no_diff_file_given");
}

/**
 * @brief No Flags Nor Diff Provided
 *
 */
Test(validargs_suite, no_diff_file_or_flag_given, .timeout = 5) {
    char *argv[] = {progname, NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    int opt = global_options;
    int exp_opt = 0;

    assert_func_expected_status(ret, exp_ret, "no_diff_file_or_flag_given");
    //assert_fliki_expected_options(opt, exp_opt);
    //assert_fliki_expected_null_ptr(diff_filename, "no_diff_file_or_flag_given");
}


/**
 * @brief Too many flags provided
 *
 */
Test(validargs_suite, too_many_flags, .timeout = 5) {
    char *argv[] = {progname, "-n", "-h", "-q", "-p", "-r", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    int opt = global_options;
    int exp_opt = 0;

    assert_func_expected_status(ret, exp_ret, "too_many_flags");
    //assert_fliki_expected_options(opt, exp_opt);
    //assert_fliki_expected_null_ptr(diff_filename, "too_many_flags");
}

/**
 * @brief Incorrect ordering of flags
 *
 */
Test(validargs_suite, incorrect_input_flag_order, .timeout = 5) {
    char *argv[] = {progname, "-q", "-h", "-n", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    int opt = global_options;
    int exp_opt = 0;

    assert_func_expected_status(ret, exp_ret, "incorrect_input_flag_order");
    //assert_fliki_expected_options(opt, exp_opt);
    //assert_fliki_expected_null_ptr(diff_filename, "incorrect_input_flag_order");
}


/**
 * @brief 1st least significant bit is 1 if '-h' is set
 *
 */
Test(validargs_suite, first_least_significant_bit_if_h_set, .timeout = 5) {
    char *argv[] = {progname, "-h",  NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options & (1 << 0);
    int exp_opt = 1;

    assert_func_expected_status(ret, exp_ret, "first_least_significant_bit_if_h_set");
    assert_fliki_expected_options(opt, exp_opt);
    assert_fliki_expected_null_ptr(diff_filename, "first_least_significant_bit_if_h_set");
}

/**
 * @brief Correct setting of diff_filename when no flags
 *
 */
Test(validargs_suite, diff_filename_no_flags, .timeout = 5) {
    char *argv[] = {progname, "foobar", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int exp_opt = 0;

    assert_func_expected_status(ret, exp_ret, "diff_filename_no_flags");
    assert_fliki_expected_options(opt, exp_opt);
    cr_assert_eq(diff_filename, argv[1], "Variable diff_filename was not properly set");
}

/**
 * @brief Improper modification of global_options or diff_filename on error
 *
 */
Test(validargs_suite, options_modified_on_error, .timeout = 5) {
    char *argv[] = {progname, "-n", "-x", "foobar", NULL};
    int argc = (sizeof(argv) / sizeof(char *)) - 1;
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    int opt = global_options;
    int exp_opt = 0;

    assert_func_expected_status(ret, exp_ret, "options_modified_on_error");
    assert_fliki_expected_options(opt, exp_opt);
    assert_fliki_expected_null_ptr(diff_filename, "options_modified_on_error");
}
