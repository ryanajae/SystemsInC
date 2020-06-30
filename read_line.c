#include "samples/prototypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

// initial allocation will be for min size, if not big enough, doubles
// to 64, then 128, then 256, etc. as needed to accommodate the entire line
#define MINIMUM_SIZE 32

/* Type: function read_line
 * ----------------------------------
 * Takes pointer to a FILE struct, reading/writing it line-by-line
 * and returning a pointer to a dynamically allocated string of the
 * written line. 
 */
char *read_line(FILE *fp)
{
    int ch = getc(fp);
    if (ch == EOF) return NULL;
    char *line = malloc(MINIMUM_SIZE);
    assert(line);
    if (ch == '\n') return line;
    ungetc(ch, fp);
    char *end = fgets(line, MINIMUM_SIZE, fp);
    if (line[strlen(line) - 1] == '\n') return line;
    if (feof(fp) != 0) return line; // Check for EOF
    int resizes = 0;
    char *catch;
    while (true) {
        catch = realloc(line, MINIMUM_SIZE * (resizes + 2));
        assert(catch);
        line = catch;
        resizes++;
        end = fgets(line + (MINIMUM_SIZE * resizes) - (1 * resizes), MINIMUM_SIZE * resizes, fp);
        if (end[strlen(end) - 1] == '\n') { // Line ends after first newline
            end[strlen(end) - 1] = '\0'; // Replace newline with null char
            break;
        }
    }
    return line;
}
