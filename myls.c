#include <dirent.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

/* Type: function is_dir
 * ----------------------------------
 * Takes the unsigned char d_type for type of file in
 * the dirent structure and returns if it's equal to
 * the macro constant for directory, DT_DIR.
 */
bool is_dir(int filetype)
{
    return filetype == DT_DIR;
}

/* Type: comparison function compar_type
 * ----------------------------------
 * Passed into scandir function as its comparison function
 * to sort directories first, followed by non-directories,
 * alphabetically ordering within type.
 */
int compar_type(const struct dirent **entry1, const struct dirent **entry2) {
    if (is_dir((*entry1)->d_type) == is_dir((*entry2)->d_type)) { // Both are same type, order alphabetically
        return strcasecmp((*entry1)->d_name, (*entry2)->d_name);
    } else if (is_dir((*entry2)->d_type)) {
        return 1;
    } else {
        return -1;
    }
}

/* Type: comparison function compar_name
 * ----------------------------------
 * Passed into scandir function as its comparison function
 * to sort entries alphabetically case-insensitively by using
 * strcasecmp. Returns the relationship (less than, equal, 
 * greater than between entry1's name and entry2's name as an int.
 */
int compar_name(const struct dirent **entry1, const struct dirent **entry2) {
    return strcasecmp((*entry1)->d_name, (*entry2)->d_name);
}

/* Type: filter starts_dot
 * ----------------------------------
 * Passed into scandir function as its filter function,
 * for functionality to exclude entries that start with
 * the "." character. scandir includes entries for which this
 * function returns nonzero and excludes entries for which
 * it returns zero.
 */
int starts_dot(const struct dirent *entry) {
    return strncmp(".", entry->d_name, 1);
}

/* Type: function ls
 * ----------------------------------
 * Takes a pointer to the directory and the booleans for
 * the command line flags. Scans the directory according
 * to the presence or absence of flags and then prints
 * the directory's entries.
 */
void ls(const char *dir, bool show_all, bool sort_by_type)
{
    struct dirent **entrylist;
    int n;
    if (show_all) { // Don't filter out entries that start with "."
        n = scandir(dir, &entrylist, NULL, compar_name);
    } else if (sort_by_type) { // Directories > non-directories
        n = scandir(dir, &entrylist, starts_dot, compar_type);
    } else { // No flags
        n = scandir(dir, &entrylist, starts_dot, compar_name);
    }

    if (n < 0) { // Handle failure
        error(EXIT_FAILURE, 0, "cannot access %s: No such directory", dir);
    } else { // scandir successful
       for (int i = 0; i < n; i++) { // Print each entry in array
        if (is_dir(entrylist[i]->d_type)) {
            printf("%s/\n", entrylist[i]->d_name);
        } else {
            printf("%s\n", entrylist[i]->d_name);
        }
        free(entrylist[i]);
    }
    free(entrylist);
}
}

/* myls
 * ----------------------------------
 * Lists a directory's contents given zero or more
 * directory paths from the command line. Supports 
 * two flags: -a to show hidden directories that 
 * start with ".", and -z to sort directories first
 * and non-directories second. Utilizes the scandir
 * function with custom filter and comparison functions.
 */
int main(int argc, char *argv[])
{
    bool show_all = false;
    bool dirs_first = false;

    int opt;
    while ((opt = getopt(argc, argv, "az")) != -1) { // Process command line flags by 
        //calling getopt, which returns only values prefixed with '-' and -1 with none remaining
        switch (opt) { // Turns on boolean for appropriate flag
            case 'a': show_all = true; break; // Don't ignore entries that start with .
            case 'z': dirs_first = true; break; // Sort directories first followed by non-directories
            default: exit(1);
        }
    }
    if (optind < argc -1) { // External variable updated by getopt of argv index
        // of first argv element that doesn't start with '-'; therefore, process
        // all remaining arguments
        for (int i = optind; i < argc; i++) {
            printf("%s:\n", argv[i]);
            ls(argv[i], show_all, dirs_first);
            printf("\n");
        }
    } else { // No paths, print from current directory
        ls(optind == argc -1? argv[optind] : ".", show_all, dirs_first); 
    }
    return 0;
}
