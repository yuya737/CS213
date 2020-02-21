#define _GNU_SOURCE

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <math.h>
#include <inttypes.h>

// The minimum size returned by malloc
#define MIN_MALLOC_SIZE 16

// Round a value x up to the next multiple of y
#define ROUND_UP(x,y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

// The size of a single page of memory, in bytes
#define PAGE_SIZE 0x1000


void* freelist[9]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

typedef struct header {
    int magicNumber;
    int pageSize;
} header_t;

typedef struct node {
    struct node* next;
} node_t;

int round_size(int x) {

    int firstSize = 16;
    int count = 0;
    while (firstSize < x){
        firstSize *= 2;
        count++;
    }
    return count;
}
/**
 * Allocate space on the heap.  * \param size  The minimium number of bytes that must be allocated
 * \returns     A pointer to the beginning of the allocated space.
 *              This function may return NULL when an error occurs.
 */
void* xxmalloc(size_t size) {
    if (size > 2048) {
        fprintf(stderr, "in xxmalloc if case with size %d\n", (int) size);
        size = ROUND_UP(size, PAGE_SIZE);
        void* p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        return p;
    } else{
        // Round the size up to the next multiple of the page size
        fprintf(stderr, "in xxmalloc else case with size %d\n", (int)size );
        //size = ROUND_UP(size, PAGE_SIZE);
        fprintf(stderr, "Size is: %d\n", (int)size);

        int freeListIndex = round_size(size);

        if (freelist[freeListIndex] == NULL){
            // Request memory from the operating system in page-sized chunks
            size = ROUND_UP(size,PAGE_SIZE);
            void* p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

            // Check for errors
            if(p == MAP_FAILED) {
                fputs("mmap failed! Giving up.\n", stderr);
                exit(2);
            }

            int pageSize = pow(2, freeListIndex+4);
            int pageCount = size / pageSize;
            fprintf(stderr, "pageCount, %d pageSize: %d\n", pageCount, pageSize);
            int counter = 0;
            while (counter < pageCount){
                if (counter == 0){
                    header_t* header = (header_t*) p;
                    header->magicNumber = 1234;
                    header->pageSize = pageSize;
                    counter++;
                    continue;
                } else if (counter == (pageCount - 1)) {
                    //uintptr_t address = (uintptr_t) p; 
                    node_t* address = (node_t*) ((uintptr_t) p + pageSize*counter);  
                    address->next = NULL;
                    counter++;
                } else {
                    node_t* address = (node_t*) ((uintptr_t) p + pageSize*counter);  
                    address->next =(node_t*) ((uintptr_t) p + pageSize*(counter + 1)); 
                    counter++;
                }
            }
            //fprintf(stderr, "address to be added 0x%" PRIxPTR, (uintptr_t)p+pageSize);
            freelist[freeListIndex] = p+pageSize;
        }

        node_t* ret = (node_t*) freelist[freeListIndex];
        node_t* nextHead = ret->next;
        freelist[freeListIndex] = nextHead;
        for (int i = 0; i < 8; i++){
            fprintf(stderr, "freelist address: 0x%" PRIxPTR "\n", (uintptr_t) freelist[i]);
        }
        return ret;
    }
}
/**
 * Get the available size of an allocated object
 * \param ptr   A pointer somewhere inside the allocated object
 * \returns     The number of bytes available for use in this object
 */
size_t xxmalloc_usable_size(void* ptr) {
    fprintf(stderr, "in xxmalloc_usable_size\n");
    uintptr_t headerAddress = (uintptr_t) ptr;
    if ((uintptr_t) ptr % PAGE_SIZE != 0) { 
        headerAddress = ROUND_UP((uintptr_t) ptr, PAGE_SIZE) - PAGE_SIZE;
    }
    fprintf(stderr, "xxmaloc_usable_size header address 0x%" PRIxPTR "\n", headerAddress);
    fprintf(stderr, "xxmaloc_usable_size ptr address 0x%" PRIxPTR "\n", (uintptr_t)ptr);
    if (((header_t*) headerAddress)->magicNumber != 1234){
        fprintf(stderr, "Magic Number error: magic number is %d, headerAddress is: 0x%"  PRIxPTR "\n",((header_t*) headerAddress)->magicNumber,headerAddress );
        fprintf(stderr, "Page size: %d\n", ((header_t*) headerAddress)->pageSize);
        return -1;
    } else {
        fprintf(stderr, "Header check OK!\n");
        return ((header_t*) headerAddress)->pageSize;
    }
}

/**
 * Free space occupied by a heap object.
 * \param ptr   A pointer somewhere inside the object that is being freed
 */
void xxfree(void* ptr) {
    fprintf(stderr, "in xxfree\n");
    // Don't free NULL!
    if(ptr == NULL) return;

    size_t blocksize = xxmalloc_usable_size(ptr);
    if (blocksize == -1) return; // Invalid block

    uintptr_t freeHeadAddress = (uintptr_t) ptr;

    if (freeHeadAddress % blocksize == 0){ 
        freeHeadAddress = ROUND_UP((uintptr_t) ptr, blocksize) - blocksize;
    }

    int freeListIndex = round_size(blocksize);
    node_t* curHead = (node_t*) freelist[freeListIndex];
    ((node_t*) freeHeadAddress)->next = curHead;
    freelist[freeListIndex] = (void *) freeHeadAddress;
    return;
}


