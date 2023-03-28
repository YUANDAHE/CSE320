#include <stdio.h>

#include "fliki_utils.h"

Test(fliki_hunk_show_test, basic_test, .timeout = 5) {
    char *TEST_FILE = "rsrc/file1_file2.diff";

    /**
     * FIRST TEST
     */

    // Creating new hunks to test on.
    HUNK lib_newHunk;
    FILE *lib_diffFile = fopen(TEST_FILE, "r");

    HUNK newHunk;
    FILE *diffFile = fopen(TEST_FILE, "r");

    // Advance program states
    int hunkLen;
    hunkLen = advance_lib_hunk_state(&lib_newHunk, lib_diffFile);
    advance_student_hunk_state(&newHunk, diffFile, hunkLen);

    char *lib_buf = capture_lib_hunk_show_output(&lib_newHunk);
    char *buf = capture_student_hunk_show_output(&newHunk);

    assert_fliki_expected_string(buf, lib_buf, "basic_test_first_hunk");

    /**
     * SECOND TEST
     */

    hunkLen = advance_lib_hunk_state(&lib_newHunk, lib_diffFile);
    advance_student_hunk_state(&newHunk, diffFile, hunkLen);

    lib_buf = capture_lib_hunk_show_output(&lib_newHunk);
    buf = capture_student_hunk_show_output(&newHunk);

    assert_fliki_expected_string(buf, lib_buf, "basic_test_second_hunk");

    /**
     * THIRD TEST
     */

    hunkLen = advance_lib_hunk_state(&lib_newHunk, lib_diffFile);
    advance_student_hunk_state(&newHunk, diffFile, hunkLen);

    lib_buf = capture_lib_hunk_show_output(&lib_newHunk);
    buf = capture_student_hunk_show_output(&newHunk);

    assert_fliki_expected_string(buf, lib_buf, "basic_test_third_hunk");
}

Test(fliki_hunk_show_test, print_middle_test, .timeout = 5) {
    char *TEST_FILE = "rsrc/file1_file2.diff";

    /**
     * FIRST TEST
     */
    HUNK lib_newHunk;
    FILE *lib_diffFile = fopen(TEST_FILE, "r");

    HUNK newHunk;
    FILE *diffFile = fopen(TEST_FILE, "r");

    // Advance program state to end of first hunk
    int hunkLen;
    hunkLen = advance_lib_hunk_state(&lib_newHunk, lib_diffFile);
    advance_student_hunk_state(&newHunk, diffFile, hunkLen);

    // Puts the file descriptor to somewhere random in the second hunk.
    hunk_next(&newHunk, diffFile);
    lib_hunk_next(&lib_newHunk, lib_diffFile);
    for (int i = 0; i < 6 + 1; i++) {
        int newChar = hunk_getc(&newHunk, diffFile);
        int libChar = lib_hunk_getc(&lib_newHunk, lib_diffFile);
    }

    // Capture output
    char *lib_buf = capture_lib_hunk_show_output(&lib_newHunk);
    char *buf = capture_student_hunk_show_output(&newHunk);

    assert_fliki_expected_string(buf, lib_buf, "print_middle_test_in_middle");

    /**
     * READING REST OF HUNK
     */

    // Second HUNK test.
    for (int i = 0; i < 29 + 1; i++) {
        int newChar = hunk_getc(&newHunk, diffFile);
        int libChar = lib_hunk_getc(&lib_newHunk, lib_diffFile);
    }

    lib_buf = capture_lib_hunk_show_output(&lib_newHunk);
    buf = capture_student_hunk_show_output(&newHunk);

    assert_fliki_expected_string(buf, lib_buf, "print_middle_test_normal");
}

Test(fliki_hunk_show_test, elipsis_test, .timeout = 5) {
    char *TEST_FILE = "rsrc/long_text_file.diff";

    // Creating new hunks to test on.
    HUNK lib_newHunk;
    FILE *lib_diffFile = fopen(TEST_FILE, "r");

    HUNK newHunk;
    FILE *diffFile = fopen(TEST_FILE, "r");

    // Advance program states
    int hunkLen = advance_lib_hunk_state(&lib_newHunk, lib_diffFile);
    advance_student_hunk_state(&newHunk, diffFile, hunkLen);

    char *lib_buf = capture_lib_hunk_show_output(&lib_newHunk);
    char *buf = capture_student_hunk_show_output(&newHunk);

    assert_fliki_expected_string(buf, lib_buf, "elipsis_test");
}

//Tests for int patch(FILE *in, FILE *out, FILE *diff)
/**
 * @brief generate output with file1 and file1_file2.diff
 * @details cmp output file2 should be empty
 * @return 0
 *
 *
 */
Test(patch_test_suite, basic_test, .timeout = 5){

    char *IN_FILE = "rsrc/file1";
    char *OUT_FILE = "test_output/patch_basic.out";
    char *DIFF_FILE = "rsrc/file1_file2.diff";
    char *ANSWER_FILE = "rsrc/file2";

    // open in, out, diff files
    FILE *in = fopen(IN_FILE, "r");
    FILE *out = fopen(OUT_FILE, "w");
    FILE *diff = fopen(DIFF_FILE, "r");

    int return_code = patch(in, out, diff);
    fclose(in);
    fclose(out);
    fclose(diff);
    //patch should return 0
    assert_func_expected_status(return_code, 0, "patch returns with error");
    //diff output with answer
    check_patch_output(ANSWER_FILE, OUT_FILE, "basic_test");
}

/**
 * @brief generate output with empty diff file
 * @details return: 0
 *          output file should be the same as input file
 */

Test(patch_test_suite, empty_diff_test, .timeout = 5){

    char *IN_FILE = "rsrc/file1";
    char *OUT_FILE = "test_output/patch.out";
    char *DIFF_FILE = "rsrc/empty";
    char *ANSWER_FILE = "rsrc/file1";

    // open in, out, diff files
    FILE *in = fopen(IN_FILE, "r");
    FILE *out = fopen(OUT_FILE, "w");
    FILE *diff = fopen(DIFF_FILE, "r");

    int return_code = patch(in, out, diff);
    fclose(in);
    fclose(out);
    fclose(diff);

    //patch should return 0 UPDATE- since the doc did not define error status for empty diff file, dont check the return code
    //assert_func_expected_status(return_code, 0, "patch returns with error");


    //diff output with answer
    check_patch_output(ANSWER_FILE, OUT_FILE, "empty_diff_test");

}

/**
 * @brief Invalid Diff File - Missing '<' from beginning (Using file1, file2)
 * @return -1.
 *
 */
Test(patch_test_suite, invalid_diff_test, .timeout = 5){

    char *IN_FILE = "rsrc/file1";
    char *OUT_FILE = "test_output/patch_invalid_diff.out";
    char *DIFF_FILE = "rsrc/file1_file2_baddir.diff";
    char *ANSWER_FILE = "rsrc/empty";

    // open in, out, diff files
    FILE *in = fopen(IN_FILE, "r");
    FILE *out = fopen(OUT_FILE, "w");
    FILE *diff = fopen(DIFF_FILE, "r");

    //execute patch function
    int return_code = patch(in, out, diff);
    fclose(in);
    fclose(out);
    fclose(diff);

    //patch should return -1
    assert_func_expected_status(return_code, -1, "patch should return with error");

    //diff output with answer -  should be empty
    check_patch_output(ANSWER_FILE, OUT_FILE, "invalid_diff_test");

}

/**
 * @brief Invalid Diff File - Invalid Diff File - Missing '---' separator (Using file1, file2)
 * @return -1
 *
 */
Test(patch_test_suite, no_sep_test, .timeout = 5){


    char *IN_FILE = "rsrc/file1";
    char *OUT_FILE = "test_output/patch_no_sep.out";
    char *DIFF_FILE = "rsrc/file1_file2_no_sep.diff";
    char *ANSWER_FILE = "rsrc/empty";

    // open in, out, diff files
    FILE *in = fopen(IN_FILE, "r");
    FILE *out = fopen(OUT_FILE, "w");
    FILE *diff = fopen(DIFF_FILE, "r");



    //execute patch function
    int return_code = patch(in, out, diff);
    fclose(in);
    fclose(out);
    fclose(diff);

    //patch should return -1
    assert_func_expected_status(return_code, -1, "patch should return with error");

    //diff output with answer -  should be empty
    check_patch_output(ANSWER_FILE, OUT_FILE, "no_sep_test");

}

/**
 * @brief invalid diff file - input line number mismatch in change opearation
 * @return -1
 * @param out output should be empty
 *
 *
*/
    
Test(patch_test_suite, wrong_line_number_test, .timeout = 5){


    char *IN_FILE = "rsrc/file1";
    char *OUT_FILE = "test_output/patch_wrong_line_number.out";
    char *DIFF_FILE = "rsrc/file1_file2_wrong_line_number.diff";
    char *ANSWER_FILE = "rsrc/patch_wrong_line_number_sol.out";

    // open in, out, diff files
    FILE *in = fopen(IN_FILE, "r");
    FILE *out = fopen(OUT_FILE, "w");
    FILE *diff = fopen(DIFF_FILE, "r");


    //execute patch function
    int return_code = patch(in, out, diff);
    fclose(in);
    fclose(out);
    fclose(diff);

    //patch should return -1
    assert_func_expected_status(return_code, -1, "patch should return with error");

    //not checking the output here since it is considered a truncated hunk test
    //check_patch_output(ANSWER_FILE, OUT_FILE, "wrong_line_number_test");

}

/**
 * @brief invalid diff file - line range in changing hunk does not match the output lines
 * @return -1
 * @param out output should be empty
 *
 *
*/

Test(patch_test_suite, wrong_line_range_test, .timeout = 5){

    char *IN_FILE = "rsrc/file1";
    char *OUT_FILE = "test_output/patch_wrong_line_range.out";
    char *DIFF_FILE = "rsrc/file1_file2_wrong_line_range.diff";
    char *ANSWER_FILE = "rsrc/empty";

    // open in, out, diff files
    FILE *in = fopen(IN_FILE, "r");
    FILE *out = fopen(OUT_FILE, "w");
    FILE *diff = fopen(DIFF_FILE, "r");

    //execute patch function
    int return_code = patch(in, out, diff);
    fclose(in);
    fclose(out);
    fclose(diff);

    //patch should return -1
    assert_func_expected_status(return_code, -1, "patch should return with error");

    //not checking the output here since it is considered a truncated hunk test
    // check_patch_output(ANSWER_FILE, OUT_FILE, "wrong_line_range_test");

}

/**
 * @brief invalid diff file - deleted line number exceeds input file lines leads to truncated results
 * @return -1
 * @param out output should discard deletion on the third hunk, but apply changes to the first two hunks. Should match rsrc/patch_wrong_delete_line_sol.out
 *
 *
*/

Test(patch_test_suite, wrong_delete_line_test, .timeout = 5){


    char *IN_FILE = "rsrc/file1";
    char *OUT_FILE = "test_output/patch_wrong_delete_line.out";
    char *DIFF_FILE = "rsrc/file1_file2_wrong_delete_line.diff";
    char *ANSWER_FILE = "rsrc/patch_wrong_delete_line_sol.out";

    // open in, out, diff files
    FILE *in = fopen(IN_FILE, "r");
    FILE *out = fopen(OUT_FILE, "w");
    FILE *diff = fopen(DIFF_FILE, "r");

    //execute patch function
    int return_code = patch(in, out, diff);
    fclose(in);
    fclose(out);
    fclose(diff);


    //patch should return -1
    assert_func_expected_status(return_code, -1, "patch should return with error");

    //not checking the output here since it is considered a truncated hunk test
    // check_patch_output(ANSWER_FILE, OUT_FILE, "wrong_delete_line_test - output file should be truncated");

}


/**
 * @brief Valid Diff File - Check if it returns the correct hunks from it.
 *
 */
Test(hunk_next_suite, basic_pass_test, .timeout = 5) {
    HUNK hunk;
    HUNK lib_hunk;
    char *test_file = "rsrc/hunk_test1.diff";
    FILE *diff = fopen(test_file, "r");
    FILE *diff_solution = fopen(test_file, "r");
    for (int i = 0; i < 4; i++){
        int exp_ret = lib_hunk_next(&lib_hunk, diff_solution);
        int ret = hunk_next(&hunk, diff);
        int result = 0;
        if (exp_ret == 0)
            result = compare_hunks(&hunk, &lib_hunk);
        else if (exp_ret == EOF)
            assert_func_expected_status(ret, exp_ret, "not returning EOF. end-of-file or error reading from the input stream not detected");
        else if (exp_ret == ERR)
            assert_func_expected_status(ret, exp_ret, "not returning ERR. The data in the input stream should have not been interpreted as a hunk");
        assert_func_expected_status(result, 0, "hunk was parsed incorrectly");
    }
    fclose(diff_solution);
    fclose(diff);
}

/**
 * @brief Corrupted Diff File - truncated in middle of seconde hunk's header
 *
 */
Test(hunk_next_suite, truncated_diff_test, .timeout = 5) {
    HUNK hunk;
    FILE *diff = fopen("rsrc/hunk_test3.diff", "r");
    hunk_next(&hunk, diff);
    int ret = hunk_next(&hunk, diff);
    assert_func_expected_status(ret, ERR, "not returning ERR. Diff file had malformed hunk header!");

    fclose(diff);
}


/**
 * @brief Invalid Diff File - with invalid command character
 *
 */
Test(hunk_next_suite, invalid_diff_command_test, .timeout = 5) {
    HUNK hunk;
    HUNK lib_hunk;
    char *test_file = "rsrc/hunk_test2.diff";
    FILE *diff = fopen(test_file, "r");
    FILE *diff_solution = fopen(test_file, "r");
    int exp_ret = lib_hunk_next(&lib_hunk, diff_solution);
    int ret = hunk_next(&hunk, diff);
    int result = 0;
    if (exp_ret == 0)
        result = compare_hunks(&hunk, &lib_hunk);
    else if (exp_ret == EOF)
        assert_func_expected_status(ret, exp_ret, "not returning EOF. end-of-file or error reading from the input stream not detected");
    else if (exp_ret == ERR)
        assert_func_expected_status(ret, exp_ret, "not returning ERR. The data in the input stream should have not been interpreted as a hunk");
    assert_func_expected_status(result, 0, "hunk was parsed incorrectly");

    fclose(diff_solution);
    fclose(diff);
}

/**
 * @brief Check if hunk_next() returns the next hunk successfully without finishing parsing of the previous hunk
 *
 */
Test(hunk_next_suite, state_agnostic_hunk_test, .timeout = 5) {
    HUNK hunk;
    HUNK lib_hunk;
    char *test_file = "rsrc/hunk_test1.diff";
    FILE *diff = fopen("rsrc/hunk_test1.diff", "r");
    FILE *diff_solution = fopen(test_file, "r");
    lib_hunk_next(&lib_hunk, diff_solution);
    hunk_next(&hunk, diff);
    lib_hunk_getc(&lib_hunk, diff_solution);
    hunk_getc(&hunk, diff);
    int exp_ret = lib_hunk_next(&lib_hunk, diff_solution);
    int ret = hunk_next(&hunk, diff);
    int result = 0;
    if (exp_ret == 0)
        result = compare_hunks(&hunk, &lib_hunk);
    else if (exp_ret == EOF)
        assert_func_expected_status(ret, exp_ret, "not returning EOF. end-of-file or error reading from the input stream not detected");
    else if (exp_ret == ERR)
        assert_func_expected_status(ret, exp_ret, "not returning ERR. The data in the input stream should have not been interpreted as a hunk");
    assert_func_expected_status(result, 0, "hunk was parsed incorrectly");

    fclose(diff_solution);
    fclose(diff);
}

/**
 * @brief Check if hunk_next() properly interpretes 0a1 hunk
 *
 */
Test(hunk_next_suite, leading_zero_pass_test1, .timeout = 5) {
    HUNK hunk;
    HUNK lib_hunk;
    char *test_file = "rsrc/hunk_test4.diff";
    FILE *diff = fopen(test_file, "r");
    FILE *diff_solution = fopen(test_file, "r");
    int exp_ret = lib_hunk_next(&lib_hunk, diff_solution);
    int ret = hunk_next(&hunk, diff);
    int result = 0;
    if (exp_ret == 0)
        result = compare_hunks(&hunk, &lib_hunk);
    else if (exp_ret == EOF)
        assert_func_expected_status(ret, exp_ret, "not returning EOF. end-of-file or error reading from the input stream not detected");
    else if (exp_ret == ERR)
        assert_func_expected_status(ret, exp_ret, "not returning ERR. The data in the input stream should have not been interpreted as a hunk");
    assert_func_expected_status(result, 0, "hunk was parsed incorrectly");

    fclose(diff_solution);
    fclose(diff);
}

/**
 * @brief Check if hunk_next() properly interpretes 1d0 hunk
 *
 */
Test(hunk_next_suite, leading_zero_pass_test2, .timeout = 5) {
    HUNK hunk;
    HUNK lib_hunk;
    char *test_file = "rsrc/hunk_test5.diff";
    FILE *diff = fopen(test_file, "r");
    FILE *diff_solution = fopen(test_file, "r");
    int exp_ret = lib_hunk_next(&lib_hunk, diff_solution);
    int ret = hunk_next(&hunk, diff);
    int result = 0;
    if (exp_ret == 0)
        result = compare_hunks(&hunk, &lib_hunk);
    else if (exp_ret == EOF)
        assert_func_expected_status(ret, exp_ret, "not returning EOF. end-of-file or error reading from the input stream not detected");
    else if (exp_ret == ERR)
        assert_func_expected_status(ret, exp_ret, "not returning ERR. The data in the input stream should have not been interpreted as a hunk");
    assert_func_expected_status(result, 0, "hunk was parsed incorrectly");

    fclose(diff_solution);
    fclose(diff);
}


/**
 * @brief Check if hunk_next() properly complains about 1c0 hunk
 *        
 * 
 *        This test is commented out, since people can check for this error in
 *        different parts of their program other than hunk_next(), and therefore
 *        this shouldn't be checked in hunk_next_suite
 *
 */
/* Test(hunk_next_suite, leading_zero_fail_test, .timeout = 5) {
    HUNK hunk;
    HUNK lib_hunk;
    char *test_file = "rsrc/hunk_test6.diff";
    FILE *diff = fopen(test_file, "r");
    FILE *diff_solution = fopen(test_file, "r");
    int exp_ret = lib_hunk_next(&lib_hunk, diff_solution);
    int ret = hunk_next(&hunk, diff);
    int result = 0;
    if (exp_ret == 0)
        result = compare_hunks(&hunk, &lib_hunk);
    else if (exp_ret == EOF)
        assert_func_expected_status(ret, exp_ret, "not returning EOF. end-of-file or error reading from the input stream not detected");
    else if (exp_ret == ERR)
        assert_func_expected_status(ret, exp_ret, "not returning ERR. The data in the input stream should have not been interpreted as a hunk");
    assert_func_expected_status(result, 0, "hunk was parsed incorrectly");

    fclose(diff_solution);
    fclose(diff);
}*/


/**
 * @brief Check if hunk_getc() will return a basic line, followed by EOS.
 * @see rsrc/getc.diff, lines 1-2
 */
Test(hunk_getc_suite, basic_line_return_test, .timeout = 5) {
    int hunk_start = 1;  // Line # of hunk header
    char *path = "rsrc/getc.diff";
    HUNK hunk;
    FILE *getc_stream = fopen(path, "r");
    FILE *f_stream = fopen(path, "r");

    skip_lines(getc_stream, hunk_start-1);  // Go to desired hunk
    hunk_next(&hunk, getc_stream);  // Setup hunk struct
    skip_lines(f_stream, hunk_start);  // Go to desired hunk

    // Match entire line.
    int received, expected;
    cr_assert_eq(0, compare_hunk_lines(1, f_stream, getc_stream, &hunk, &expected, &received),
        "Invalid char received for hunk_getc. Got: %i | Expected: %i ", received, expected);

    // Ensure EOS
    received = hunk_getc(&hunk, getc_stream);
    cr_assert_eq(received, EOS,
        "hunk_getc failed to return EOS! Got: %i | Expected: %i (EOS)", received, EOS);

    // Close files.
    fclose(getc_stream);
    fclose(f_stream);
}

/**
 * @brief Check if hunk_getc() will return multiple lines with identifiers inside.
 * @see rsrc/getc.diff, lines 3-6
 */
Test(hunk_getc_suite, multiple_line_return_test, .timeout = 5) {
    int hunk_start = 3;  // Line # of hunk header
    char *path = "rsrc/getc.diff";
    HUNK hunk;
    FILE *getc_stream = fopen(path, "r");
    FILE *f_stream = fopen(path, "r");

    skip_lines(getc_stream, hunk_start-1);  // Go to desired hunk
    hunk_next(&hunk, getc_stream);  // Setup hunk struct
    skip_lines(f_stream, hunk_start);  // Go to desired hunk

    // Match entire hunk.
    int received, expected;
    cr_assert_eq(0, compare_hunk_lines(3, f_stream, getc_stream, &hunk, &expected, &received),
        "Invalid char received for hunk_getc. Got: %i | Expected: %i ", received, expected);

    // Ensure EOS
    received = hunk_getc(&hunk, getc_stream);
    cr_assert_eq(received, EOS,
        "hunk_getc failed to return EOS! Got: %i | Expected: %i (EOS)", received, EOS);

    // Close files.
    fclose(getc_stream);
    fclose(f_stream);
}

/**
 * @brief Check if hunk_getc() will make the appropriate EOS returns in a change-type hunk.
 * @see rsrc/getc.diff, lines 7-10
 */
Test(hunk_getc_suite, basic_change_return_test, .timeout = 5) {
    int hunk_start = 7;
    char *path = "rsrc/getc.diff";
    HUNK hunk;
    FILE *getc_stream = fopen(path, "r");
    FILE *f_stream = fopen(path, "r");

    skip_lines(getc_stream, hunk_start-1);  // Go to desired hunk
    hunk_next(&hunk, getc_stream);  // Setup hunk struct
    skip_lines(f_stream, hunk_start);  // Go to desired hunk

    // Match entire line.
    int received, expected;
    cr_assert_eq(0, compare_hunk_lines(1, f_stream, getc_stream, &hunk, &expected, &received),
        "Invalid char received for hunk_getc. Got: %i | Expected: %i ", received, expected);

    // Ensure EOS
    received = hunk_getc(&hunk, getc_stream);
    cr_assert_eq(received, EOS,
        "hunk_getc failed to return EOS! Got: %i | Expected: %i (EOS)", received, EOS);

    // Move to second section of change hunk.
    skip_lines(f_stream, 1);  // Skip '---' line

    // Match entire line.
    cr_assert_eq(0, compare_hunk_lines(1, f_stream, getc_stream, &hunk, &expected, &received),
        "Invalid char received for hunk_getc. Got: %i | Expected: %i ", received, expected);

    // Ensure EOS
    received = hunk_getc(&hunk, getc_stream);
    cr_assert_eq(received, EOS,
        "hunk_getc failed to return EOS! Got: %i | Expected: %i (EOS)", received, EOS);

    // Close files.
    fclose(getc_stream);
    fclose(f_stream);
}

/**
 * @brief Check if hunk_getc() will correctly return ERR when encountering ">" as opposed to "> "
 * @see rsrc/getc.diff, lines 11-12
 */
Test(hunk_getc_suite, basic_error_test, .timeout = 5) {
    int hunk_start = 11;
    char *path = "rsrc/getc.diff";
    HUNK hunk;
    FILE *getc_stream = fopen(path, "r");

    skip_lines(getc_stream, hunk_start-1);  // Go to desired hunk
    hunk_next(&hunk, getc_stream);  // Setup hunk struct

    // Verify ERR
    int received = hunk_getc(&hunk, getc_stream);
    cr_assert_eq(received, ERR,
        "Invalid return received for hunk_getc. Got: %i | Expected: %i (ERR)", received, ERR);

    // Close files
    fclose(getc_stream);
}

/**
 * @brief Check if hunk_getc() will correctly return ERR when encountering the wrong type of
 * brackets on the second part of a change hunk.
 * @see rsrc/getc.diff, lines 21-24
 */
Test(hunk_getc_suite, mismatch_brackets_test, .timeout = 5) {
    int hunk_start = 21;
    char *path = "rsrc/getc.diff";
    HUNK hunk;
    FILE *f_stream = fopen(path, "r");
    FILE *getc_stream = fopen(path, "r");

    skip_lines(getc_stream, hunk_start-1);  // Go to desired hunk
    hunk_next(&hunk, getc_stream);  // Setup hunk struct
    skip_lines(f_stream, hunk_start);  // Go to desired hunk

    // Match first part of hunk
    int received, expected;
    cr_assert_eq(0, compare_hunk_lines(1, f_stream, getc_stream, &hunk, &expected, &received),
        "Invalid char received for hunk_getc. Got: %i | Expected: %i", received, expected);

    // Ensure EOS
    received = hunk_getc(&hunk, getc_stream);
    cr_assert_eq(received, EOS,
        "hunk_getc failed to return EOS! Got: %i | Expected: %i (EOS)", received, EOS);

    // Verify ERR
    received = hunk_getc(&hunk, getc_stream);
    cr_assert_eq(received, ERR,
        "Invalid return received for hunk_getc. Got: %i | Expected: %i (ERR)", received, ERR);

    // Close files
    fclose(getc_stream);
    fclose(f_stream);
}

/**
 * @brief Check if hunk_getc() will correctly return ERR when encountering EOF in the middle of a hunk.
 * @see rsrc/getc.diff, lines 36-37
 */
Test(hunk_getc_suite, eof_mid_hunk_test, .timeout = 5) {
    int hunk_start = 36;
    char *path = "rsrc/getc.diff";
    HUNK hunk;
    FILE *f_stream = fopen(path, "r");
    FILE *getc_stream = fopen(path, "r");

    skip_lines(getc_stream, hunk_start-1);  // Go to desired hunk
    hunk_next(&hunk, getc_stream);  // Setup hunk struct
    skip_lines(f_stream, hunk_start);  // Go to desired hunk

    // Match first part of hunk
    int received, expected;
    cr_assert_eq(-1, compare_hunk_lines(1, f_stream, getc_stream, &hunk, &expected, &received),
        "Invalid char received for hunk_getc. Got: %i | Expected: %i (ERR)", received, ERR);

    // Close files
    fclose(getc_stream);
    fclose(f_stream);
}
