#include "samples/prototypes.h"
#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Type: function print_uniq_lines
 * ----------------------------------
 * Takes pointer to a FILE struct, calling read_line
 * to process each line. Keeps count of repeating lines,
 * printing each line after it doesn't occur consecutively
 * with the number of occurrences as a prefix followed by
 * the line.
 */
void print_uniq_lines(FILE *fp)
{
    int count = 1;
    char *curr = read_line(fp);
    while (curr != NULL) {
        char *prev = malloc(sizeof(char)*strlen(curr) + 1);
        prev = strcpy(prev, curr);
        free(curr);
        curr = read_line(fp);
        if (curr == NULL) { // Reach EOF, print last prev and free
            printf("%6d %s\n", count, prev);
            free(curr);
            free(prev);
            break;
        }
        if (strcmp(prev, curr) == 0) { // Consecutive occurrence increments count
            count++;
        } else { // Not consecutive occurrence, print prev and reset count
            printf("%6d %s\n", count, prev);
            count = 1;
        }
        free(prev);
    }
}

/* myuniq
 * ----------------------------------
 * Counts number of consecutive occurences of lines in a file. Prints
 * this number as a prefix followed by the line. Does not support any CLI flags.
 */
int main(int argc, char *argv[])
{
    FILE *fp;

    if (argc == 1) {
        fp = stdin;
    } else { // Ignores any arguments other than file as first argument
        fp = fopen(argv[1], "r");
        if (fp == NULL) error(1, 0, "%s: no such file", argv[1]);
    }
    print_uniq_lines(fp);
    fclose(fp);
    return 0;
}
