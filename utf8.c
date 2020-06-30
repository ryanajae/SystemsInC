#include "samples/prototypes.h"
#include <error.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * Function takes unicode code point and the initialized array to store byte sequence. to_utf8 then performs conversion from code point to the proper UTF-8 encoding using bitmasks for the given character. Returns the number of bytes, or the length of the array, as type int.
 */
int to_utf8(unsigned short cp, unsigned char seq[])
{
    if (cp <= 0x007f) { // cp will never be less than 0
        seq[0] = cp; // leading zero will be padded
        return 1;
    } else if (cp >= 0x0080 && cp <= 0x07ff) { // Unicode code point range for 11 sigbits
        seq[0] = 0xC0 | cp >> 6; // Pad with six zeros to orient with mask for fixed bits 110xxxxx
        seq[1] = 0x80 | (cp & (UCHAR_MAX >> 2)); // Create mask with two zeros followed by all ones to get last 6 sig bits, or'ing with mask 10xxxxxx
        return 2;
    } else { // Code point must be in range U+0800 to U+FFFF
        seq[2] = ((0x3f & cp) | 0x80); // Use mask with six ones in least significant position to get last 6 sigbits, then or with mask to get fixed bits
        seq[1] = ((0x3f & (cp >> 6)) | 0x80); // Use mask with six ones in least significant position to get next 6 sigbits, then or with mask to get fixed bits
        seq[0] = ((0xf & (cp >> 12)) | 0xE0); // Use mask with four ones in least significant position to get final 4 sigbits, then or with mask to get fixed bits 1110xxxx
        return 3;
    }
}




// ------- DO NOT EDIT ANY CODE BELOW THIS LINE (but do add comments!)  -------

/*
 * print_utf8 takes the user input, the converted bytes stored in the seq[] array and the number of bytes to print the program output in a nicely spaced, easily readable format of Code Point, Hex representation, and UTF Character.
 */
void print_utf8(unsigned short cp, unsigned char seq[], int len)
{
    int i;
    printf("U+%04X   Hex:", cp); // Print code point 
    for (i = 0; i < len; i++) // Print hex representation of each byte
       printf(" %02x", seq[i]);
    for ( ; i < 4; i++) printf("   "); // Blank spacess for formatting
    seq[len] = '\0'; // Add null terminator
    printf("Character: %s  \n", seq); // Print final character
}

/*
 * This function converts the user inputted string to numerical shorts by calling the function strtol. convert_arg performs error handling by checking for illegal user inputs. Returns the user inputted string as an unsigned short.
 */
unsigned short convert_arg(const char *str, unsigned short low, unsigned short high)
{
    char *end;
    long n = strtol(str, &end, 0); // Takes the user inputted string and converts it to a numerical long value
    if (*end != '\0') // strtotal returns a pointer to the null terminator \0 when the string conversion is successful
        error(1, 0, "Invalid code point '%s'", str); // Pointer was not to null terminator and error is thrown that user input was not legal
    if (n < low || n > high) // Checks that n isn't less than 0 or longer than USHRT_MAX
        error(1, 0, "Code point %s not in range [%u, %u]", str, low, high); // Print error to user that value is out of bounds
    return n;
}

/*
 * utf8 is a program that interprets a code point encoding a Unicode character and displays that character to the user. The Unicode encoding is as referenced on the Assignment 1 spec.
 */
int main(int argc, char *argv[])
{
    if (argc < 2) 
        error(1, 0, "Missing argument. Please supply one or more unicode code points in decimal or hex."); // Error handling for not enough arguments
    
    for (int i = 1; i < argc; i++) {
        unsigned short cp = convert_arg(argv[i], 0, USHRT_MAX);
        unsigned char seq[4]; // Initialize array
        int len = to_utf8(cp, seq); // Convert to utf from hex and get length in bytes
        print_utf8(cp, seq, len); // Print program output to user
    }
    return 0;
}
