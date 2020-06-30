#include "samples/prototypes.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

/*
 * Function takes const char * to array of environment variables
 * and a const char string key. Returns the found path in the envp[].
 * Written in lab section with Daniel. 
 */
const char *get_env_value(const char *envp[], const char *key)
{
    int keylen = strlen(key);
    for (int i = 0; envp[i] != NULL; i++) {
        const char *cur = envp[i];
        if (strncmp(key, cur, keylen) == 0 && cur[keylen] == '=') return cur + keylen + 1; // Iterate past PATH= variable name to get just the actual path 
    }
    return NULL; 
}

/*
 * parse_path takes the pointer to the path buffer, the pointer to
 * the directory buffer holding the current directory being searched, 
 * and the name of the executable being searched for. The function
 * creates and uses a dirent struct to construct the path. 
 */
void parse_path(char *pathptr, const char *remaining, char *dir, char *execname, bool wildcard) {
    DIR *dirptr = opendir(dir);
    if (dirptr == NULL) return;
    struct dirent *entry;
        if (wildcard) { // Handles wildcard case by iterating through all directory entry names
            while((entry = readdir(dirptr)) != NULL) {
                if (strstr(entry->d_name, execname) != NULL) { // Find entry that contains wildcard substring
                    sprintf(pathptr, "%s/%s", dir, entry->d_name);
                    if (access(pathptr, X_OK) == 0 && access(pathptr, R_OK) == 0) {
                        printf("%s\n", pathptr);
                    }
                }
            }
        } else { // Standard case checks directory by directory
            while ((entry = readdir(dirptr)) != NULL) {
                sprintf(pathptr, "%s/%s", dir, execname); // Concatenate executable to the path string with a /
                if (access(pathptr, X_OK) == 0 && access(pathptr, R_OK) == 0) { // File exists and has read permissions
                    printf("%s\n", pathptr);
                }
                break; // While stepping through in debugger I noticed this loop seems to continue unnecessarily for a (seemingly) arbitrary amount of time...so I just broke because I'm checking one dirent at a time in this case
            }
        }
        closedir(dirptr);
    }

/*
 * mywhich searches for a command name and displays the path to its
 * matching executable. This search is performed directory by directory
 * and utilizes the scan_token function. mywhich also has additional
 * functionality supporting a wildcard search when an argument has a
 * "+" prefix, returning all executables that contain the wildcard substring.
 */
    int main(int argc, char *argv[], const char *envp[])
    {
        char *execname = "";
        const char *searchpath = get_env_value(envp, "MYPATH");
    if (searchpath == NULL) searchpath = get_env_value(envp, "PATH"); // No MYPATH environment variable, use PATH instead
    char dir[PATH_MAX];
    char path[PATH_MAX];
    char *pathptr = path;
    bool wildcard = false;
    if (argc == 1) { // No arguments case = print all directories in search path
        const char *remaining = searchpath;
        printf("Directories in search path:\n");
        while (scan_token(&remaining, ":", dir, sizeof(dir))) { // Tokenize search path into directories, then print them
            printf("%s\n", dir);
        }
    } else {
        for (int i = 1; argv[i]; i++) { // Iterate through all search names
            const char *remaining = searchpath; // Copy to iteratively progress through searchpath
            execname = argv[i];
            if (execname[0] == '+') { // Check for wildcard case
                execname++;
                wildcard = true;
            }
            while (scan_token(&remaining, ":", dir, sizeof(dir))) { // Tokenize then check each directory path using helper function
                parse_path(pathptr, remaining, dir, execname, wildcard);
            }
        }
    }
    return 0;
}
