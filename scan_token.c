#include "samples/prototypes.h"
#include <string.h>
#include <stdbool.h>

/*
 * This tokenizing function takes a char ** to an input string separated by
 * delimiters defined in const char * delimiters. The tokens are written to
 * char * buf of length size_t buflen. The function then returns true for a
 * successfully written token and false when no token is written. scan-token
 * has functionality to handle overflow cases when a valid token is larger than
 * buflen, splitting the token into validly sized portions.
 */
bool scan_token(const char **p_input, const char *delimiters, char *buf, size_t buflen)
{
    int len_delim = strspn(*p_input, delimiters); // Handle leading delimiters, returns 0 if none
    *p_input += len_delim;
    int len_token = strcspn(*p_input, delimiters); // Get length of token
    if (len_token >= buflen - 1) { // Handling for overflow case
    	strncpy(buf, *p_input, buflen - 1); // Copy by length of buffer with space for null terminator
    	buf[buflen] = '\0';
    	*p_input += buflen - 1;
    	return true;
    } else if (len_token != 0) {
    	strncpy(buf, *p_input, len_token);
    	buf[len_token] = '\0';
    	*p_input += len_token;
    	return true;
    }
    return false; // p_input is empty meaning len_token == 0
}