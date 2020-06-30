#include "allocator.h"

#define ALIGNMENT 8
#define MIN_SIZE_BLOCK 32 // sizeof(header) + sizeof(char *) * 2 + 8 byte payload

static void *free_list, *heap_start;

typedef struct {
    unsigned int sz;	// size of memory block
    bool free;		    // in use or free?
    bool end;			// end header?
} header;

typedef struct {
	void *next;
	void *prev;
} node;

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
 * the end header and its counterpart node to represent all 
 * free space at the end of the heap and sets the global variables.
 */
bool myinit(void *segment_start, size_t segment_size)
{
	if (segment_size < MIN_SIZE_BLOCK) return false;
	header *endhead = segment_start;
	heap_start = segment_start;
    endhead->sz = rounddown(segment_size, ALIGNMENT) - (sizeof(header) * 3); // header * and two void *
    endhead->free = true;
    endhead->end = true;
    void *backnodeptr = (char *)segment_start + sizeof(header);
    node *backnode = backnodeptr;
    backnode->next = NULL;
    backnode->prev = NULL;
    free_list = endhead;
    return true;
}

/* Type: helper function mynewendheader
 * ----------------------------------
 * Takes a pointer to the last location on the heap of the last
 * end header and creates a new end header at the appropriate new
 * location given some size.
 */
void *mynewendheader(void *location, unsigned int size) {
	void *newendptr = (char *)location + sizeof(header) + size;
	header *newend = newendptr;
	newend->sz = ((header *)location)->sz - size;
	newend->free = true;
	newend->end = true;
	void *newnodeptr = (char *)newendptr + sizeof(header);
	node *backnode = newnodeptr;
	backnode->next = ((node *)((char *)location + sizeof(header)))->next;
	backnode->prev = ((node *)((char *)location + sizeof(header) * 2))->prev;
	return newend;
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
 * memory and traverses the heap starting from the initial block on
 * the explicit free list until finding the location that can accommodate
 * the requested memory.
 */
void *mymalloc(size_t requestedsz)
{
	unsigned int needed = roundup(requestedsz, ALIGNMENT);
    // Edge cases where size is 0 or larger than INT_MAX
	if (needed == 0 || needed > 0xFFFFFFFF) return NULL;
	if (needed < MIN_SIZE_BLOCK) needed = MIN_SIZE_BLOCK;
	void *traversal = free_list;
	while (true) {
    	if (((header *)traversal)->sz < needed && ((header *)traversal)->end == true) return NULL; // Heap exhausted
    	if (((header *)traversal)->sz >= needed && ((header *)traversal)->free == true) { // Found open block
    		// Case 1: Available block is the end header
    		if (((header *)traversal)->end == true) {
    			free_list = mynewendheader(traversal, needed);
    			void *oldendnodeptr = (char *)traversal + sizeof(header);
    			node *oldendnode = oldendnodeptr;
    			void *newendnodeptr = (char *)free_list + sizeof(header);
    			node *newendnode = newendnodeptr;
    			newendnode->next = oldendnode->next;
    			newendnode->prev = oldendnode->prev;
    			mynewheader(traversal, needed);
    			return oldendnodeptr;
    		}
            // Case 2: Regular block
    		((header *)traversal)->free = false;
    		traversal = (char *)traversal + sizeof(header);
    		node *oldnode = traversal;
    		void *nextheader = oldnode->next;
    		void *nextnodeptr = (char *)nextheader + sizeof(header);
    		node *nextnode = nextnodeptr;
    		nextnode->prev = oldnode->prev;
    		free_list = oldnode->next;
    		return traversal;
    	}
    	traversal = (char *)traversal + sizeof(header);
    	node *nextnode = traversal;
    	void *next = nextnode->next;
    	traversal = next;
    }
}

/* Type: function myfree
 * ----------------------------------
 * Updates the header for the memory block pointed to by *ptr to show
 * the block as FREE and then adds it to the explicit free list.
 *
 * Does not support coalescing because I couldn't get it to work in time
 * :((((((
 */
void myfree(void *ptr)
{
	void *blockhead = (char *)ptr - sizeof(header);
	((header *)blockhead)->free = true;
	node *newnode = ptr;
	newnode->next = free_list; // Point to the previous front of the list
	newnode->prev = NULL;
	void *endnodeptr = (char *)free_list + sizeof(header);
	node *endnode = endnodeptr;
	endnode->prev = blockhead;
	free_list = blockhead; // Add freed block to the front of the list
}

/* Type: function myrealloc
 * ----------------------------------
 * Takes the pointer for the memory block to be reallocated and the newsz
 * for that block. 
 *
 * Ran out of time to implement :(
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
		return oldptr;
	}
	return NULL;
}

/* Type: function validate_heap
 * ----------------------------------
 * Called after every request, validate_heap traverses the free list to make sure
 * no in-use blocks have snuck into the list. Next, the function traverses all 
 * memory blocks and checks that there aren't invalid entries for the header struct 
 * fields which indicate a bad heap.
 */
bool validate_heap()
{
    void *free = free_list;
    while (true) {
    	if (!((header *)free)->free) return false;
    	if (((header *)free)->end) break;
    	free = (char *)free + sizeof(header);
    	node *next = free;
    	free = next->next;
    }
    void *traversal = heap_start;
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
