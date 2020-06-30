#include <string.h>
#include <stdbool.h>
#include "allocator.h"

#define ALIGNMENT 8
#define MIN_SIZE_BLOCK 16 // sizeof(header) + 8 byte payload

static void *heap_start, *heap_end;

// 8 bytes
typedef struct {
    unsigned int sz;	// size of memory block (does not include sizeof(header))
    bool free;		    // in use or free
    bool end;			// end header or not
} header;

/* Type: function roundup
 * ----------------------------------
 * Takes an integer value and rounds it to the nearest multiple
 * of the mult parameter by first adding mult - 1 to the value
 * and then turning off all bits that are less than the bit for
 * the multiple.
 * 
 * Citation: bump.c, CS107 Teaching Staff
 */
unsigned int roundup(unsigned int sz, unsigned int mult)
{
    return (sz + mult-1) & ~(mult-1);
}

/* Type: function rounddown
 * ----------------------------------
 * Takes an integer value and rounds it down to the multiple
 * of the mult parameter by turning off all bits that are less
 * significant than the bit for the multiple of 2.
 *
 * Citation: bump.c, CS107 Teaching Staff
 */
unsigned int rounddown(unsigned int sz, unsigned int mult)
{
    return sz & ~(mult-1);
}

/* Type: function myinit
 * ----------------------------------
 * Takes a pointer to the start of the heap segment and the
 * segment_size as determined by test_harness. Initializes
 * the end header which represents all free space at the end
 * of the heap and sets the global variable heap_start to the
 * start of the heap segment.
 */
bool myinit(void *segment_start, size_t segment_size)
{
    if (segment_size < MIN_SIZE_BLOCK) return false;
    heap_start = segment_start;
    heap_end = (char *)segment_start + rounddown(segment_size, ALIGNMENT);
    header *endhead = segment_start;
    endhead->sz = rounddown(segment_size, ALIGNMENT) - sizeof(header);
    endhead->free = true;
    endhead->end = true;
    return true;
}

/* Type: helper function mynewendheader
 * ----------------------------------
 * Takes a pointer to the last location on the heap of the last
 * end header and creates a new end header at the appropriate new
 * location given some size.
 */
void mynewendheader(void *location, unsigned int size) {
    void *newendptr = (char *)location + sizeof(header) + size;
    header *newend = newendptr;
    newend->sz = ((header *)location)->sz - size;
    newend->free = true;
    newend->end = true;
}

/* Type: helper function mynewheader
 * ----------------------------------
 * Takes a pointer to the location in memory on the heap of the
 * start of the new memory block, creating a header for the block
 * at that locaiton.
 */
void mynewheader(void *location, unsigned int size) {
    header *newhead = location;
    newhead->sz = size;
    newhead->free = false;
    newhead->end = false;
}

/* Type: function mymalloc
 * ----------------------------------
 * Takes an 8-byte requestedsz for a block of dynamically allocated
 * memory and traverses the heap starting from the initial block via
 * the implicit list until finding the location that can accommodate
 * the requested memory.
 */
void *mymalloc(size_t requestedsz)
{
    unsigned int needed = roundup(requestedsz, ALIGNMENT);
    // Edge cases where size is 0 or larger than INT_MAX
    if (needed == 0 || needed > 0xFFFFFFFF) return NULL;
    if (needed < MIN_SIZE_BLOCK) needed = MIN_SIZE_BLOCK;
    void *traversal = heap_start;
    while (true) {
        if (((header *)traversal)->sz < needed && ((header *)traversal)->end == true) return NULL; // Heap exhausted
    	if (((header *)traversal)->sz >= needed && ((header *)traversal)->free == true) { // Found open block
    		// Case 1: Available block is the end header
            if (((header *)traversal)->end == true) {
                mynewendheader(traversal, needed);
                mynewheader(traversal, needed);
                void *alloc = (char *)traversal + sizeof(header);
                return alloc;
            }
            // Case 2: Regular block
            ((header *)traversal)->free = false;
    		void *alloc = (char *)traversal + sizeof(header); // Advance past header to get pointer to memory block
    		return alloc;
    	}
    	traversal = (char *)traversal + sizeof(header) + ((header *)traversal)->sz;
    }
}

/* Type: function myfree
 * ----------------------------------
 * Updates the header for the memory block pointed to by *ptr to show
 * the block as FREE.
 */
void myfree(void *ptr)
{
	if (ptr == NULL) return;
	ptr = (char *)ptr - sizeof(header); // Move pointer back to header in front of block
	((header *)ptr)->free = true;
}

/* Type: function myrealloc
 * ----------------------------------
 * Takes the pointer for the memory block to be reallocated and the newsz
 * for that block. Checks first if a resize in-place is possible by seeing
 * if the block is to be shrunk, or if there are free blocks adjacent down
 * the heap that can be coalesced. If a resize in-place is not possible, the
 * memory is re-malloc'd and the old memory block freed.
 */
void *myrealloc(void *oldptr, size_t newsz)
{
    // If ptr is NULL, call to realloc functions as call to malloc
    if (oldptr == NULL) return mymalloc(newsz);
    unsigned int needed = roundup(newsz, ALIGNMENT);
    // If ptr is not NULL but the requested size is zero, realloc
    // functions as a call to free
    if (oldptr != NULL && needed == 0) {
        myfree(oldptr);
        return NULL;
    }
    void *oldptrhead = (char *)oldptr - sizeof(header);
    
    // Case 1: Resize-in place possible because block is being shrunk
    if (needed < ((header *)oldptrhead)->sz) {
        unsigned int extraspace = ((header *)oldptrhead)->sz - needed;
        if (extraspace >= MIN_SIZE_BLOCK) { // Split block
            ((header *)oldptrhead)->sz = needed;
            void *splitblock = (char *)oldptr + needed;
            extraspace -= sizeof(header); // Decrement size of splitblock's header
            mynewheader(splitblock, extraspace);
            ((header *)splitblock)->free = true;
        }
        return oldptr;
    }
    // Case 2: Absorb free blocks to the right
    void *rightblockhead = (char *)oldptr + ((header *)oldptrhead)->sz;
    if (((header *)rightblockhead)->free == true) {
        // Right block is the end header
        if (((header *)rightblockhead)->end == true) {
            unsigned int offset = needed - ((header *)oldptrhead)->sz;
            rightblockhead = (char *)rightblockhead - sizeof(header); // Move back by 8 because mynewendhead adds 8 for new header that isn't created at rightblockhead 
            mynewendheader(rightblockhead, offset);
            ((header *)oldptrhead)->sz += offset;
            return oldptr;
        }
        // Else traverse adjacent free blocks until run out of free blocks or size is met
        unsigned int freesz = ((header *)oldptrhead)->sz + ((header *)rightblockhead)->sz + sizeof(header);
        while (true) {
            if (freesz >= needed) {
                ((header *)oldptrhead)->sz = freesz;
                return oldptr;
            }
            rightblockhead = (char *)rightblockhead + sizeof(header) + ((header *)rightblockhead)->sz;
            if (((header *)rightblockhead)->free == true) {
                if (((header *)rightblockhead)->end == true) {
                    unsigned int offset = needed - freesz;
                    rightblockhead = (char *)rightblockhead - sizeof(header);
                    mynewendheader(rightblockhead, offset);
                    ((header *)oldptrhead)->sz = needed;
                    return oldptr;
                }
                freesz += ((header *)rightblockhead)->sz + sizeof(header);
            } else {
                break; // Resize in-place is not possible, jump to re-malloc
            }
        }
    }
    // Case 3: Traverse heap to find block via call to malloc
    void *newblock = mymalloc(newsz);
    mynewheader(newblock, needed);
    memcpy(newblock, oldptr, ((header *)oldptrhead)->sz); // Preserve data from old block
    myfree(oldptr);
    return newblock;
}

/* Type: function validate_heap
 * ----------------------------------
 * Called after every request, validate_heap traverses all memory blocks and 
 * checks that there aren't invalid entries for the header struct fields which
 * indicate a bad heap.
 */
bool validate_heap()
{
    void *traversal = heap_start;
    // Traverse until reaching the end of the heap
    while (!((header *)traversal)->end) {
        int address = *(int *)traversal;
        // Check alignment
        if (address % ALIGNMENT != 0) return false;
        // Nested if block checks that free field is a bool
        if (((header *)traversal)->free != true) {
            if (((header *)traversal)->free != false) return false;
        }
        // Second check ensures that end field is a bool
        if (((header *)traversal)->end != false) return false;
        if (((header *)traversal)->sz == 0) return false;
        traversal = (char *)traversal + sizeof(header) + ((header *)traversal)->sz;
    }
    return true;
}
