#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

#include <sys/mman.h>	/* for PROT_WRITE/PROT_READ, MAP_SHARED, munmap/mmap, shm_open/shm_unlink */
#include <sys/stat.h>	/* For mode constants */
#include <sys/wait.h>
#include <fcntl.h>

//linked list to track freed, previously allocated blocks.
struct list_entry {
    size_t size;
    struct list_entry* next;
};

typedef struct list_entry list_entry_t;


static list_entry_t free_block_list_head = { 0, NULL };
static const size_t overhead = sizeof(size_t);
static const size_t align_to = 16;

void* my_malloc(size_t size) {

    //ceiling-round size+sizeof(size_t) to the nearest multiple of align_to
    size = (size + sizeof(size_t) + (align_to - 1)) & ~ (align_to - 1);
    list_entry_t* block = free_block_list_head.next;
    list_entry_t** head = &(free_block_list_head.next);
    while (block != NULL) {
        if (block->size >= size) {
            *head = block->next;
            return ((char*)block) + sizeof(size_t);
        }
        head = &(block->next);
        block = block->next;
    }

    block = (list_entry_t*)sbrk(size);
    block->size = size;

    return ((char*)block) + sizeof(size_t);
}

void free(void* ptr) {
    list_entry_t* block = (list_entry_t*)(((char*)ptr) - sizeof(size_t));
    block->next = free_block_list_head.next;
    free_block_list_head.next = block;
}



int main(int argc, char const *argv[]) {


    int* integer = my_malloc(sizeof(int));
    *integer = 10;
    printf("integer: %d\n", *integer);
    free(integer);
    printf("freed. done\n");

    return EXIT_SUCCESS;
}
