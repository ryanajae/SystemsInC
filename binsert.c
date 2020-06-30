#include "samples/prototypes.h"
#include <string.h>

/* binsert
 * ----------------------------------
 * Uses Apple's binary search code to find
 * void *key in the array void *base, and
 * if key is not in the array, inserts it
 * at the correct index. This includes the
 * special case of creating an array with
 * the first element as key. Takes a comparison
 * function as an argument.
 */
void *binsert(const void *key, void *base, size_t *p_nelem, size_t width, int (*compar)(const void *, const void *))
{
	char *end = (char *)base + (width * (*p_nelem)); // Get the end before manipulating *base
	for (size_t nremain = *p_nelem; nremain != 0; nremain >>= 1) {
		void *p = (char *)base + (nremain >> 1) * width; // Divide by two by right shifting a power of 2
		int sign = compar(key, p);
		if (sign == 0) return p; // Match found with key
		if (sign > 0) { // Move right
			base = (char *)p + width;
			nremain--;
		} // Else move left
	}
	if (*p_nelem == 0) { // Case 1: Empty array, copy in key as first element
		memcpy((char *)base, (char *)key, width);
	} else {
		int to_write = end - (char *)base; // Calculate bytes to move
		memmove((char *)base + width, (char *)base, to_write); // Right shift over one
		memcpy((char *)base, (char *)key, width);
	}
	(*p_nelem)++;
	return base;
}
