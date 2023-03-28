#include <stdlib.h>
#include <stdio.h>

#include "fliki.h"
#include "global.h"
#include "debug.h"

// current line of diff file
static int cur_diff_line_num = 1;
static int serial;

static int infile_next_line;
static int outfile_cur_line;

struct {
    int error;
    int begin_char;
    int deletions;
    int additions;
    int dashes;
    int deletions_buffer_index;
    int additions_buffer_index;
    short cur_add_line_len;
    short cur_delete_line_len;
} hunk_state;

static int myisdigit(int c) {
    if (c < '0' || c > '9') {
        return 0;
    }
    return 1;
}

static int get_hunk_type(char c) {
    if (c == 'a') {
        return HUNK_APPEND_TYPE;
    }
    if (c == 'd') {
        return HUNK_DELETE_TYPE;
    }
    if (c == 'c') {
        return HUNK_CHANGE_TYPE;
    }
    return HUNK_NO_TYPE;
}

static int read_range(FILE *in, int *start, int *end) {
    int *num = start;
    while (1) {
        int c = fgetc(in);
        if (feof(in)) {
            return EOF;
        }

        if (myisdigit(c)) {
            *num = *num*10+(c-'0');
            continue;
        }
        if (c == ',') {
            *end = 0;
            num = end;
            continue;
        }

        ungetc(c, in);
        break;
    }

    return 0;
}

static int is_valid_hunk(HUNK *hp) {
    // "%da%d\n", <num1>, <num2>
    // "%da%d,%d\n", <num1>, <num2>, <num3>
    if (hp->type == HUNK_APPEND_TYPE) {
        if (hp->old_end >= 0) {
            error("%d", hp->old_end);
            return 0;
        }
        if (hp->new_end >= 0 && hp->new_end < hp->new_start) {
            error("%d,%d", hp->new_start, hp->new_end);
            return 0;
        }
    }

    // "%dd%d\n", <num1>, <num2>
    // "%d,%dd%d\n", <num1>, <num2>, <num3>
    if (hp->type == HUNK_DELETE_TYPE) {
        if (hp->new_end >= 0) {
            error("%d", hp->new_end);
            return 0;
        }
        if (hp->old_end >= 0 && hp->old_end < hp->old_start) {
            error("%d,%d", hp->old_start, hp->old_end);
            return 0;
        }
    }

    if (hp->type == HUNK_CHANGE_TYPE) {
        if (hp->old_end >= 0 && hp->old_end < hp->old_start) {
            error("%d,%d", hp->old_start, hp->old_end);
            return 0;
        }

        if (hp->new_end >= 0 && hp->new_end < hp->new_start) {
            error("%d,%d", hp->new_start, hp->new_end);
            return 0;
        }
    }
    
    return 1;
}

static int calc_line_count(int end, int start) {
    if (end < 0) {
        return 1;
    }

    return end - start + 1;
}

static int skip_dashes(FILE *in) {
    for (int i = 0; i < 3; i++) {
        int c = fgetc(in);
        if (feof(in)) {
            return EOF;
        }
        if (i < 2 && c != '-') {
            return ERR;
        }
        if (i == 2 && c != '\n') {
            return ERR;
        }
    }

    return 0;
}

static void init_hunk(HUNK *hp) {
    hp->old_start = 0;
    hp->old_end = -1; // not exist
    hp->new_start = 0;
    hp->new_end = -1; // not exist
}

static void reset_hunk_state() {
    hunk_state.error = 0;
    hunk_state.additions = 0;
    hunk_state.deletions = 0;
    hunk_state.begin_char = 0;
    hunk_state.dashes = 0;
    hunk_state.deletions_buffer_index = 2;
    hunk_state.additions_buffer_index = 2;
     hunk_state.cur_delete_line_len = 0;
    hunk_state.cur_add_line_len = 0;

    for (int i = 0; i < HUNK_MAX; i++) {
        *(hunk_deletions_buffer+i)=0;
    }
    for (int i = 0; i < HUNK_MAX; i++) {
        *(hunk_additions_buffer+i)=0;
    }
}

static void save_addition_char(char c) {
    if (hunk_state.additions_buffer_index >= HUNK_MAX - 2) {
        return;
    }
    *(hunk_additions_buffer+hunk_state.additions_buffer_index) = c;
    hunk_state.additions_buffer_index++;
    hunk_state.cur_add_line_len++;
    if (c == '\n' || hunk_state.additions_buffer_index == HUNK_MAX - 2) {
        int start = hunk_state.additions_buffer_index - hunk_state.cur_add_line_len - 2;
        char *len_buf = (char *)&hunk_state.cur_add_line_len;
        *(hunk_additions_buffer+start) = *(len_buf + 0);
        *(hunk_additions_buffer+start+1) = *(len_buf + 1);

        if (c == '\n') {
            hunk_state.additions_buffer_index += 2;
            hunk_state.cur_add_line_len = 0;
        }
    }
}


static void save_deletion_char(char c) {
    if (hunk_state.deletions_buffer_index >= HUNK_MAX - 2) {
        return;
    }
    *(hunk_deletions_buffer+hunk_state.deletions_buffer_index) = c;
    hunk_state.deletions_buffer_index++;
    hunk_state.cur_delete_line_len++;
    if (c == '\n' || hunk_state.deletions_buffer_index == HUNK_MAX - 2) {
        int start = hunk_state.deletions_buffer_index - hunk_state.cur_delete_line_len - 2;
        char *len_buf = (char *)&hunk_state.cur_delete_line_len;
        *(hunk_deletions_buffer+start) = *(len_buf + 0);
        *(hunk_deletions_buffer+start+1) = *(len_buf + 1);

        if (c == '\n') {
            hunk_state.deletions_buffer_index += 2;
            hunk_state.cur_delete_line_len = 0;
        }
    }
}


static int hunk_getc_help(FILE *in) {
    while (1) {
        int c = fgetc(in);
        if (feof(in)) {
            return EOF;
        }
        if (hunk_state.begin_char) {
            if (hunk_state.begin_char == '>') {
                save_addition_char(c);
            } else {
                save_deletion_char(c);
            }
            if (c == '\n') {
                hunk_state.begin_char = 0;
                cur_diff_line_num++;
            }
            return c;
        }

        if (c == '>' || c == '<') {
            hunk_state.begin_char = c;
            c = fgetc(in);
            if (feof(in)) {
                return EOF;
            }
            if (c != ' ') {
                return ERR;
            }
            continue;
        }
        
        if (c == '-') {
            if (hunk_state.dashes) {
                return ERR;
            }
            int ret = skip_dashes(in);
            if (ret < 0) {
                return ret;
            }
            hunk_state.dashes = 1;
            cur_diff_line_num++;
            continue;
        }

        ungetc(c, in);
        break;
    }

    return EOS;
}

/**
 * @brief Get the header of the next hunk in a diff file.
 * @details This function advances to the beginning of the next hunk
 * in a diff file, reads and parses the header line of the hunk,
 * and initializes a HUNK structure with the result.
 *
 * @param hp  Pointer to a HUNK structure provided by the caller.
 * Information about the next hunk will be stored in this structure.
 * @param in  Input stream from which hunks are being read.
 * @return 0  if the next hunk was successfully located and parsed,
 * EOF if end-of-file was encountered or there was an error reading
 * from the input stream, or ERR if the data in the input stream
 * could not be properly interpreted as a hunk.
 */

int hunk_next(HUNK *hp, FILE *in) {
    // skip data
    while (1) {
        int ret = hunk_getc_help(in);
        if (ret == EOS) {
            break;
        }
        if (ret < 0) {
            return ret;
        }
    }

    init_hunk(hp);

    int c = fgetc(in);
    if (feof(in)) {
        return EOF;
    }

    // first char must digital
    if (!myisdigit(c)) {
        error("invalid hunk, line %d", cur_diff_line_num);
        return ERR;
    }
    ungetc(c, in);

    // read old range
    int ret = read_range(in, &(hp->old_start), &(hp->old_end));
    if (ret != 0) {
        error("invalid hunk, line %d", cur_diff_line_num);
        return ret;
    }

    // read command
    c = fgetc(in);
    hp->type = get_hunk_type(c);
    if (hp->type == HUNK_NO_TYPE) {
        error("invalid hunk, line %d", cur_diff_line_num);
        return ERR;
    }

    // read new range
    ret = read_range(in, &(hp->new_start), &(hp->new_end));
    if (ret != 0) {
        error("invalid hunk, line %d", cur_diff_line_num);
        return ret;
    }

    // read line break
    c = fgetc(in);
    if (c != '\n') {
        error("invalid hunk, line %d", cur_diff_line_num);
        return ERR;
    }

    if (!is_valid_hunk(hp)) {
        error("invalid hunk, line %d", cur_diff_line_num);
        return ERR;
    }

    hp->serial = ++serial;
    reset_hunk_state();
    cur_diff_line_num++;
    return 0;
}

/**
 * @brief  Get the next character from the data portion of the hunk.
 * @details  This function gets the next character from the data
 * portion of a hunk.  The data portion of a hunk consists of one
 * or both of a deletions section and an additions section,
 * depending on the hunk type (delete, append, or change).
 * Within each section is a series of lines that begin either with
 * the character sequence "< " (for deletions), or "> " (for additions).
 * For a change hunk, which has both a deletions section and an
 * additions section, the two sections are separated by a single
 * line containing the three-character sequence "---".
 * This function returns only characters that are actually part of
 * the lines to be deleted or added; characters from the special
 * sequences "< ", "> ", and "---\n" are not returned.
 * @param hdr  Data structure containing the header of the current
 * hunk.
 *
 * @param in  The stream from which hunks are being read.
 * @return  A character that is the next character in the current
 * line of the deletions section or additions section, unless the
 * end of the section has been reached, in which case the special
 * value EOS is returned.  If the hunk is ill-formed; for example,
 * if it contains a line that is not terminated by a newline character,
 * or if end-of-file is reached in the middle of the hunk, or a hunk
 * of change type is missing an additions section, then the special
 * value ERR (error) is returned.  The value ERR will also be returned
 * if this function is called after the current hunk has been completely
 * read, unless an intervening call to hunk_next() has been made to
 * advance to the next hunk in the input.  Once ERR has been returned,
 * then further calls to this function will continue to return ERR,
 * until a successful call to call to hunk_next() has successfully
 * advanced to the next hunk.
 */

int hunk_getc(HUNK *hp, FILE *in) {
    if (hunk_state.error) {
        return ERR;
    }

    int c = hunk_getc_help(in);
    if (c == EOF) {
        return ERR;
    }

    return c;
}

static int print_hunk_data(char begin_char, char *buf, FILE *out) {
    int index = 0;
    while (1) {
        short len = *((short *)(buf+index));
         index += 2;
        if (len == 0) {
            break;
        }

        fputc(begin_char, out);
        fputc(' ', out);
        for (int i = 0; i < len; i++) {
            fputc(*(buf+index), out);
            index++;
        }
    }
    if (index >= HUNK_MAX-2) {
        fprintf(out, "...\n");
    }
    return index;
}

/**
 * @brief  Print a hunk to an output stream.
 * @details  This function prints a representation of a hunk to a
 * specified output stream.  The printed representation will always
 * have an initial line that specifies the type of the hunk and
 * the line numbers in the "old" and "new" versions of the file,
 * in the same format as it would appear in a traditional diff file.
 * The printed representation may also include portions of the
 * lines to be deleted and/or inserted by this hunk, to the extent
 * that they are available.  This information is defined to be
 * available if the hunk is the current hunk, which has been completely
 * read, and a call to hunk_next() has not yet been made to advance
 * to the next hunk.  In this case, the lines to be printed will
 * be those that have been stored in the hunk_deletions_buffer
 * and hunk_additions_buffer array.  If there is no current hunk,
 * or the current hunk has not yet been completely read, then no
 * deletions or additions information will be printed.
 * If the lines stored in the hunk_deletions_buffer or
 * hunk_additions_buffer array were truncated due to there having
 * been more data than would fit in the buffer, then this function
 * will print an elipsis "..." followed by a single newline character
 * after any such truncated lines, as an indication that truncation
 * has occurred.
 *
 * @param hp  Data structure giving the header information about the
 * hunk to be printed.
 * @param out  Output stream to which the hunk should be printed.
 */

void hunk_show(HUNK *hp, FILE *out) {
    char cmd = 'a';
    if (hp->type == HUNK_DELETE_TYPE) {
        cmd = 'd';
    } else {
        cmd = 'c';
    }
    if (hp->old_end >= 0 && hp->new_end >= 0) {
        fprintf(out, "%d,%d%c%d,%d\n", hp->old_start, hp->old_end, cmd, hp->new_start, hp->new_end);
    } else if (hp->old_end >= 0) {
        fprintf(out, "%d,%d%c%d\n", hp->old_start, hp->old_end, cmd, hp->new_start);
    } else if (hp->new_end >= 0) {
        fprintf(out, "%d%c%d,%d\n", hp->old_start, cmd, hp->new_start, hp->new_end);
    } else {
        fprintf(out, "%d%c%d\n", hp->old_start, cmd, hp->new_start);
    }
    
    if (print_hunk_data('<', hunk_deletions_buffer, out)) {
        short len = *((short *)hunk_additions_buffer);
        if (len > 0) {
            fprintf(out, "---\n");
        }
    }
    print_hunk_data('>', hunk_additions_buffer, out);
}

static void outputc(char c, FILE *out) {
    if (global_options & NO_PATCH_OPTION) {
        return;
    }
    fputc(c, out);
}

static int copy_lines(FILE *in, FILE *out, int num) {
    for (int i = 0; i < num; i++) {
        while (1) {
            int c = getc(in);
            if (feof(in)) {
                return 0;
            }
            outputc(c, out);
            if (c == '\n') {
                infile_next_line++;
                outfile_cur_line++;
                break;
            }
        }
    }

    return 1;
}

static int is_outfile_line_ok(HUNK *hp) {
    int new_end = hp->new_end;
    if (new_end < 0) {
        new_end = hp->new_start;
    }
    if (outfile_cur_line != new_end) {
        error("%d %d", outfile_cur_line, new_end);
        return 0;
    }

    return 1;
}

static int patch_change(HUNK *hp, FILE *in, FILE *out, FILE *diff) {
    if (infile_next_line > hp->old_start) {
        error(" %d > %d", infile_next_line, hp->old_start);
        return -1;
    }
    if (hp->old_start - infile_next_line > 0) {
        if (!copy_lines(in, out, hp->old_start - infile_next_line)) {
            error("copy_lines");
            return -1;
        }
    }
    
    int deletions = calc_line_count(hp->old_end, hp->old_start);
    int additions = calc_line_count(hp->new_end, hp->new_start);
    for (int i = 0; i < deletions; i++) {
        while (1) {
            int c = hunk_getc(hp, diff);
            if (c < 0) {
                error("hunk_getc %d", c);
                return -1;
            }
            if (c != '\n' && hunk_state.begin_char != '<') {
                error("begin_char %c", hunk_state.begin_char);
                return -1;
            }
            int read_char = fgetc(in);
            if (read_char != c) {
                error("%c != %c", read_char, c);
                return -1;
            }
            if (c == '\n') {
                infile_next_line++;
                break;
            }
        }
    }
    for (int i = 0; i < additions; i++) {
        while (1) {
            int c = hunk_getc(hp, diff);
            if (c < 0) {
                error("hunk_getc %d", c);
                return -1;
            }
            if (c != '\n' && hunk_state.begin_char != '>') {
                error("begin_char %c", hunk_state.begin_char);
                return -1;
            }
            if (!hunk_state.dashes) {
                error("no dashes");
                return -1;
            }
            outputc(c, out);
            if (c == '\n') {
                outfile_cur_line++;
                break;
            }
        }
    }

    if (!is_outfile_line_ok(hp)) {
        return -1;
    }
    return 0;
}

static int patch_delete(HUNK *hp, FILE *in, FILE *out, FILE *diff) {
    if (infile_next_line > hp->old_start) {
        return -1;
    }
    if (hp->old_start - infile_next_line > 0) {
        if (!copy_lines(in, out, hp->old_start - infile_next_line)) {
            return -1;
        }
    }
    
    int deletions = calc_line_count(hp->old_end, hp->old_start);
    for (int i = 0; i < deletions; i++) {
        while (1) {
            int c = hunk_getc(hp, diff);
            if (c < 0) {
                return -1;
            }
            if (c != '\n' && hunk_state.begin_char != '<') {
                return -1;
            }
            if (fgetc(in) != c) {
                return -1;
            }
            if (c == '\n') {
                infile_next_line++;
                break;
            }
        }
    }

    if (!is_outfile_line_ok(hp)) {
        return -1;
    }
    return 0;
}

static int patch_append(HUNK *hp, FILE *in, FILE *out, FILE *diff) {
    if (infile_next_line-1 > hp->old_start) {
        error(" %d -1 > %d", infile_next_line-1, hp->old_start);
        return -1;
    }

    if (!copy_lines(in, out, hp->old_start - infile_next_line+1)) {
        error("copy_lines");
        return -1;
    }

    int additions = calc_line_count(hp->new_end, hp->new_start);
    for (int i = 0; i < additions; i++) {
        while (1) {
            int c = hunk_getc(hp, diff);
            if (c < 0) {
                return -1;
            }
            if (c != '\n' && hunk_state.begin_char != '>') {
                return -1;
            }
            outputc(c, out);
            if (c == '\n') {
                outfile_cur_line++;
                break;
            }
        }
    }

    if (!is_outfile_line_ok(hp)) {
        return -1;
    }
    return 0;
}

static int output_error(const char *msg) {
    if (global_options & QUIET_OPTION) {
        return 0;
    }
    fprintf(stderr, "%s", msg);
    fputc('\n', stderr);
    return 1;
}

/**
 * @brief  Patch a file as specified by a diff.
 * @details  This function reads a diff file from an input stream
 * and uses the information in it to transform a source file, read on
 * another input stream into a target file, which is written to an
 * output stream.  The transformation is performed "on-the-fly"
 * as the input is read, without storing either it or the diff file
 * in memory, and errors are reported as soon as they are detected.
 * This mode of operation implies that in general when an error is
 * detected, some amount of output might already have been produced.
 * In case of a fatal error, processing may terminate prematurely,
 * having produced only a truncated version of the result.
 * In case the diff file is empty, then the output should be an
 * unchanged copy of the input.
 *
 * This function checks for the following kinds of errors: ill-formed
 * diff file, failure of lines being deleted from the input to match
 * the corresponding deletion lines in the diff file, failure of the
 * line numbers specified in each "hunk" of the diff to match the line
 * numbers in the old and new versions of the file, and input/output
 * errors while reading the input or writing the output.  When any
 * error is detected, a report of the error is printed to stderr.
 * The error message will consist of a single line of text that describes
 * what went wrong, possibly followed by a representation of the current
 * hunk from the diff file, if the error pertains to that hunk or its
 * application to the input file.  If the "quiet mode" program option
 * has been specified, then the printing of error messages will be
 * suppressed.  This function returns immediately after issuing an
 * error report.
 *
 * The meaning of the old and new line numbers in a diff file is slightly
 * confusing.  The starting line number in the "old" file is the number
 * of the first affected line in case of a deletion or change hunk,
 * but it is the number of the line *preceding* the addition in case of
 * an addition hunk.  The starting line number in the "new" file is
 * the number of the first affected line in case of an addition or change
 * hunk, but it is the number of the line *preceding* the deletion in
 * case of a deletion hunk.
 *
 * @param in  Input stream from which the file to be patched is read.
 * @param out Output stream to which the patched file is to be written.
 * @param diff  Input stream from which the diff file is to be read.
 * @return 0 in case processing completes without any errors, and -1
 * if there were errors.  If no error is reported, then it is guaranteed
 * that the output is complete and correct.  If an error is reported,
 * then the output may be incomplete or incorrect.
 */

int patch(FILE *in, FILE *out, FILE *diff) {
    infile_next_line = 1;
    outfile_cur_line = 0;
    HUNK hunk;
    while (1) {
        int ret = hunk_next(&hunk, diff);
        if (ret == EOF) {
            break;
        }
        if (ret < 0) {
            output_error("hunk_next fail");
            return -1;
        }

        if (hunk.type == HUNK_CHANGE_TYPE) {
            if (patch_change(&hunk, in, out, diff) < 0) {
                if (output_error("patch change fail")) {
                    hunk_show(&hunk, stderr);
                }
                return -1;
            }
        } else if (hunk.type == HUNK_DELETE_TYPE) {
            if (patch_delete(&hunk, in, out, diff) < 0) {
                if (output_error("patch delete fail")) {
                    hunk_show(&hunk, stderr);
                }
                return -1;
            }
        } else {
            if (patch_append(&hunk, in, out, diff) < 0) {
                if (output_error("patch append fail")) {
                    hunk_show(&hunk, stderr);
                }
                return -1;
            }
        }
    }

    return 0;
}
