#include "samples/prototypes.h"
#include <error.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_LINE_LEN 4096
#define MIN_NLINES 100

typedef int (*cmp_fn_t)(const void *p, const void *q);

/* Type: comparison function cmp_pstr
 * ----------------------------------
 * Default comparison function set to the typedef.
 * Sorts according to case-sensitive lexicographic
 * order by calling string.h's strcmp function.
 */
int cmp_pstr(const void *p, const void *q)
{
    return strcmp(*(const char **)p, *(const char **)q);
    // Need to access array of characters, not array of strings
}

/* Type: comparison function cmp_pstr_len
 * ----------------------------------
 * Functionality for sorting by line length. Ties in
 * line length are broken lexicographically. Invoked
 * via the command-line flag 'l'.
 */
int cmp_pstr_len(const void *p, const void *q)
{
    if (strlen(*(const char **)p) == (strlen(*(const char **)q))) {
        return strcmp(*(const char **)p, *(const char **)q);
    } else {
        return strlen(*(const char **)p) - strlen(*(const char **)q);
    }
}

/* Type: comparison function comp_pstr_len
 * ----------------------------------
 * Functionality for sorting by string numerical value
 * by calling stdlib.h's atoi function. Invoked via the
 * command-line flag 'n'.
 */
int cmp_pstr_numeric(const void *p, const void *q)
{
    return atoi(*(const char **)p) - atoi(*(const char **)q);
}

/* Type: function sort_lines
 * ----------------------------------
 * Takes a pointer to the file and the booleans for
 * the command line flags. Reads in all the file's
 * lines into a dynamically allocated array and then
 * uses the appropriate comparison function to sort
 * that array. sort_lines then prints the array in either
 * regular or reverse order.
 */
void sort_lines(FILE *fp, cmp_fn_t cmp, bool uniq, bool reverse)
{
    char line[MAX_LINE_LEN];
    int capacity = MIN_NLINES;
    char **stored = malloc(sizeof(char *) * capacity);
    assert(stored);
    size_t elems = 0;
    while(fgets(line, MAX_LINE_LEN, fp)) { // Read in all lines, each line up to size MAX_LINE_LEN - 1
        if (uniq) { // Use binsert to remove duplicates
            char *lineptr = line;
            char **catch = binsert(&lineptr, stored, &elems, sizeof(char *), cmp);
            if (lineptr == *catch) {
                *catch = strdup(line);
            }
        } else {
            stored[elems] = strdup(line);
            assert(stored[elems]);
            elems++;
        }
        if (capacity == elems) { // Check memory
            capacity = capacity * 2;
            stored = realloc(stored, capacity * sizeof(char *));
            assert(stored);
        }
    }
    if (!uniq) qsort(stored, elems, sizeof(char *), cmp);
    if (reverse) { // Print in reverse order
        for (int i = elems - 1; i >= 0; i--) {
            printf("%s", stored[i]);
            free(stored[i]);
        }
    } else {
        for (int i = 0; i < elems; i++) {
            printf("%s", stored[i]);
            free(stored[i]);
        }
    }
    free(stored);
}

/* mysort
 * ----------------------------------
 * Filter program that reads in a file line-by-line and
 * then prints out the lines in sorted order. Supports 
 * four flags: -l to sort by line length, -n to sort by
 * string numerical value, -r to sort in reverse order,
 * and -u to print only unique lines and discard any
 * duplicates. Utilizes stdlib.h's qsort and the binsert
 * functions to sort using the user-specified comparison.
 */
int main(int argc, char *argv[])
{
    cmp_fn_t cmp = cmp_pstr; // Set to default comparison function
    bool uniq = false, reverse = false;

    int opt;
    while ((opt = getopt(argc, argv, "lnru")) != -1) {
        switch (opt) {
            case 'l': cmp = cmp_pstr_len; break;
            case 'n': cmp = cmp_pstr_numeric; break;
            case 'r': reverse = true; break;
            case 'u': uniq = true; break;
            default: exit(1);
        }
    }

    FILE *fp = stdin; // No file, read in standard input
    if (optind < argc) { // Process non-flag argument, which must be a file
        fp = fopen(argv[optind], "r");
        if (fp == NULL) error(1, 0, "%s: no such file", argv[optind]);
    }
    sort_lines(fp, cmp, uniq, reverse);
    fclose(fp);
    return 0;
}
