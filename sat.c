#include "samples/prototypes.h"
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
 * signed_max takes the integer bitwidth which specifies the number of bits, and returns the max value for type long in signed representation.
 */
long signed_max(int bitwidth)
{ 
    return (1L << (bitwidth - 1)) - 1L; // Create a ones long and left shift it, then subtract one
}

/*
 * signed_min takes the integer bitwidth which specifies the number of bits, and returns the min value for type long in signed representation.
 */
long signed_min(int bitwidth)
{
    return -1L << (bitwidth - 1); // Create a negative ones long then shift it by number of bits - 1
}

/*
 * sat_add takes two values longs a and b and an int bitwidth and returns the saturated sum. The addition operation conditionally checks for overflow or underflow in the case of both values being both positive or both negative. If one value is negative and one is positive, they'll never be big enough to overflow/underflow.
 */
long sat_add(long a, long b, int bitwidth)
{
    if (((a & (1 << (bitwidth - 1))) == 0) && ((b & (1 << (bitwidth - 1))) == 0)){ // Both values are positive
         if (((a+b) & (1 << (bitwidth - 1))) != 0) return signed_max(bitwidth);// Sign changes, so just return signed_max
    }
    if (((a & (1 << (bitwidth - 1))) != 0) && ((b & (1 << (bitwidth - 1))) != 0)){ // Both values are negative
         if (((a+b) & (1 << (bitwidth - 1))) == 0) return signed_min(bitwidth); // Sign changes to positive, so return signed_min
    }
    return a + b; // Numbers are not same sign, so return sum
}




// ------- DO NOT EDIT ANY CODE BELOW THIS LINE (but do add comments!)  -------

/*
 * This function converts the user inputted strings to numerical longs by calling the function strtol. convert_arg also performs error handling by checking for illegal user inputs. Returns the user inputted string as an unsigned long.
 */
long convert_arg(const char *str, long low, long high, const char *argname)
{
    char *end;
    long n = strtol(str, &end, 0); // Takes the user inputted string and converts it to a numerical long value
    if (*end != '\0') // strtotal returns a pointer to the null-terminator \0 when the string conversion is successful
        error(1, 0, "Invalid number '%s'", str); // Pointer was not to null terminator and error is thrown that user input was not legal
    if (n < low || n > high || errno == ERANGE) // Checks that bitwidth isn't less than 4 or greater than 32*8
        error(1, 0, "Illegal value %s. %s must be in range [%ld, %ld]", str, argname, low, high); // Print error to user that value is out of bounds
    return n;
}

/*
 * This program sat performs saturating addition by checking for overflow and underflow for a value in signed representation for a given bitwidth. The function returns signed_max for overflow and signed_min for underflow cases, and otherwise returns the correct sum of the signed values.
 */
int main(int argc, char *argv[])
{
    if (argc < 2) error(1, 0, "Missing argument. Please specify the bitwidth."); // Error handling for not enough arguments
    
    int bitwidth = convert_arg(argv[1], 4, sizeof(long)*8, "Bitwidth"); // Convert user input for bitwidth as stored as the second inputted argument
    long min = signed_min(bitwidth); // Compute minimum for long in signed representation
    long max = signed_max(bitwidth); // Compute maximum for long in signec representation

    if (argc < 4)  { // No addition, just find range
        printf("%d-bit signed integer range\n", bitwidth); // Print how many bits for the range
        printf("min: %ld   %#018lx\n", min, min); // Print the minimum
        printf("max:  %ld   %#018lx\n", max, max); // Print the maximum
    } else { // Handle addition
        long a = convert_arg(argv[2], min, max, "Operand"); // Argument 3 becomes long a
        long b = convert_arg(argv[3], min, max, "Operand"); // Argument 4 becomes long b
        printf("%ld + %ld = %ld\n", a, b, sat_add(a, b, bitwidth)); // Display output to user
    }
    return 0;
}
