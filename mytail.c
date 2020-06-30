#include "samples/prototypes.h"
#include <error.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_NLINES_ON_STACK 10000

/* Type: function print_last_n
 * ----------------------------------
 * Creates an array holding n lines from an inputted
 * file. Stores it on the stack if n < MAX_NLINES_ON_STACK
 * and if greater, dynamically on the heap carefully
 * tracking memory allocation to avoid leaks. Acts as
 * a circular array with wraparound advancement so that
 * a window of size 10 lines is incremented across the
 * entire file.
 */
void print_last_n(FILE *fp, int n)
{
    char *lines[MAX_NLINES_ON_STACK];
    char **arrayptr = lines;
    if (n >= MAX_NLINES_ON_STACK) arrayptr = malloc(sizeof(char *) * n);
    char *line = NULL;
    for (int i = 0; i < n; i++) { // Fill first n spots in the array
        line = read_line(fp);
        if (!line) break; // No more lines
        arrayptr[i] = line;
    }
    int index = n - 1; // Set to highest value for index
    while (line) { // n > number of lines
        index = (index + 1) % n; // Use integer division to wraparound advance
        line = read_line(fp);
        if (!line) break; // No more lines
        free(arrayptr[index]);
        arrayptr[index] = line;
    }
    for (int i = index; i < n; i++) { // Print from index to n
        printf("%s\n", arrayptr[i]);
    }
    for (int i = 0; i < index; i++) { // Print from 0 up to index
        printf("%s\n", arrayptr[i]);
    }
    for (int i = 0; i < n; i++) {
        free(arrayptr[i]);
    }
    if (n >= MAX_NLINES_ON_STACK) free(arrayptr);
}

/* Type: function convert_arg
 * ----------------------------------
 * Takes a number passed as a string on the command
 * line and uses strtol to convert it to an int.
 */
int convert_arg(const char *str)
{
    char *end;
    long n = strtol(str, &end, 10);
    if (*end != '\0') 
        error(1, 0, "Invalid number '%s'", str);
    if (n < 1 || n > INT_MAX) 
        error(1, 0, "%s is not within the acceptable range [%d, %d]", str, 1, INT_MAX);
    return n;
}

/* mytail
 * ----------------------------------
 * Implementation of filter that prints final N lines of
 * an inputted file. Allows user to input a value for N
 * lines, the only supported CLI flag.
 */
int main(int argc, char *argv[])
{
    int num = 10; // Default value for n

    if (argc > 1 && argv[1][0] == '-') { // Handle user inputted value for n
        num = convert_arg(argv[1] + 1);
        argv++;
        argc--;
    }

    FILE *fp;
    if (argc == 1) {
        fp = stdin;
    } else { // Ignore other arguments
        fp = fopen(argv[1], "r");
        if (fp == NULL) error(1, 0, "%s: no such file", argv[1]);
    }
    print_last_n(fp, num);
    fclose(fp);
    return 0;
}
