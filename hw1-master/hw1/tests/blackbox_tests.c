#include "fliki_utils.h"

/**
 * @brief Program should ignore everything after -h
 *
 */
Test(blackbox_suite, ignore_after_help_test, .timeout = 5) {
    char *cmd = LIMITS"bin/fliki -h -bogus_opt > /dev/null 2>&1";
    int return_code = WEXITSTATUS(system(cmd));
    assert_func_expected_status(return_code, EXIT_SUCCESS, "ignore_after_help_test");
}

/**
 * @brief Check if Errors are printed to stderr and not stdout
 * @details Compare stderr to standard help message, Compare stdout to empty file
 */
Test(blackbox_suite, ensure_errors_to_stderr, .timeout = 5) {
    char *cmd = LIMITS"bin/fliki -n -h > test_output/err_2_stderr.stdout 2>test_output/err_2_stderr.stderr";
    char *cmp_stdout = "cmp rsrc/empty test_output/err_2_stderr.stdout";
    char *cmp_stderr = "cmp rsrc/help_msg test_output/err_2_stderr.stderr";

    int return_code = WEXITSTATUS(system(cmd));
    assert_func_expected_status(return_code, EXIT_FAILURE, "ensure_errors_to_stderr");

    return_code = WEXITSTATUS(system(cmp_stdout));
    if (return_code)
        cr_log_error("STDOUT not empty");
    assert_func_expected_status(return_code, EXIT_SUCCESS, "ensure_errors_to_stderr");

    return_code = WEXITSTATUS(system(cmp_stderr));
    if (return_code)
        cr_log_error("Help message not printed to stderr");
    assert_func_expected_status(return_code, EXIT_SUCCESS, "ensure_errors_to_stderr");
}

/**
 * @brief Diff large JSON
 * @details Tests if diff hunks extracted by randomly modifying a large JSON are correctly applied
 *          test1.json - Original Large JSON
 *          test2.json - Randomly modified (only deletion) test1.json
 *
 */
Test(blackbox_suite, large_json_test, .timeout = 5) {
    char *cmd = LIMITS"bin/fliki rsrc/test2_test1.diff < rsrc/test2.json > test_output/large_test.json";
    char *cmp = "cmp test_output/large_test.json rsrc/test1.json";

    int return_code = WEXITSTATUS(system(cmd));
    assert_func_expected_status(return_code, EXIT_SUCCESS, "large_json_test");

    return_code = WEXITSTATUS(system(cmp));
    if (return_code)
        cr_log_error("Diff applied incorrectly");
    assert_func_expected_status(return_code, EXIT_SUCCESS, "large_json_test");
}

/**
 * @brief Invalid Diff File - Missing '---' separator (Using file1, file2)
 *
 */
Test(blackbox_suite, invalid_diff_separator, .timeout = 5) {
    char *cmd = LIMITS"bin/fliki rsrc/file1_file2_no_sep.diff < rsrc/file1 > test_output/no_sep_test.out";
    char *cmp = "cmp test_output/no_sep_test.out rsrc/empty";

    int return_code = WEXITSTATUS(system(cmd));
    assert_func_expected_status(return_code, EXIT_FAILURE, "invalid_diff_separator");

    return_code = WEXITSTATUS(system(cmp));
    if (return_code)
        cr_log_error("Unexpected output to stdout");
    assert_func_expected_status(return_code, EXIT_SUCCESS, "invalid_diff_separator");
}

/**
 * @brief Invalid Diff File - Missing '<' from beginning (Using file1, file2)
 *
 */
Test(blackbox_suite, invalid_dir_diff, .timeout = 5) {
    char *cmd = LIMITS"bin/fliki rsrc/file1_file2_baddir.diff < rsrc/file1 > test_output/baddir_test.out";
    char *cmp = "cmp test_output/baddir_test.out rsrc/empty";

    int return_code = WEXITSTATUS(system(cmd));
    assert_func_expected_status(return_code, EXIT_FAILURE, "invalid_dir_diff");

    return_code = WEXITSTATUS(system(cmp));
    if (return_code)
        cr_log_error("Unexpected output to stdout");
    assert_func_expected_status(return_code, EXIT_SUCCESS, "invalid_dir_diff");
}

/**
 * @brief Test quiet mode
 * @details Stderr should be empty and exit status should be failure
 */
Test(blackbox_suite, test_quiet_mode, .timeout = 5) {
    char *cmd = LIMITS"bin/fliki -q rsrc/file1_file2_baddir.diff < rsrc/file1 > test_output/quiet_mode.stdout 2>test_output/quiet_mode.stderr";
    char *cmp_stdout = "cmp rsrc/empty test_output/quiet_mode.stdout";
    char *cmp_stderr = "cmp rsrc/empty test_output/quiet_mode.stderr";

    int return_code = WEXITSTATUS(system(cmd));
    assert_func_expected_status(return_code, EXIT_FAILURE, "test_quiet_mode");

    return_code = WEXITSTATUS(system(cmp_stdout));
    if (return_code)
        cr_log_error("STDOUT not empty");
    assert_func_expected_status(return_code, EXIT_SUCCESS, "test_quite_mode");

    return_code = WEXITSTATUS(system(cmp_stderr));
    if (return_code)
        cr_log_error("Help message not printed to stderr");
    assert_func_expected_status(return_code, EXIT_SUCCESS, "test_quiet_mode");
}

/**
 * @brief Test no-output mode
 * @details Stdout should be empty and exit status should be success
 */
Test(blackbox_suite, test_no_out_mode, .timeout = 5) {
    char *cmd = LIMITS"bin/fliki -n rsrc/file1_file2.diff < rsrc/file1 > test_output/no_out_mode.stdout";
    char *cmp_stdout = "cmp rsrc/empty test_output/no_out_mode.stdout";

    int return_code = WEXITSTATUS(system(cmd));
    assert_func_expected_status(return_code, EXIT_SUCCESS, "test_no_out_mode");

    return_code = WEXITSTATUS(system(cmp_stdout));
    if (return_code)
        cr_log_error("STDOUT not empty");
    assert_func_expected_status(return_code, EXIT_SUCCESS, "test_no_out_mode");
}
