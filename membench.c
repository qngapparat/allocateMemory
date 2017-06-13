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
#include <pthread.h>


pthread_mutex_t mutex;
///////////////////////////////////////////////////////////////////////////////

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



//////////////////////////////////////////////////////////////////////////




struct thread_args{
    unsigned allocation_count;
    unsigned allocation_size;
};

typedef struct thread_args thread_args_t;

void* thread_function(void* arg){

    thread_args_t* args;
    args = (thread_args_t*) arg;

    //to store the addresses of dynamic memory
    void* malloc_return_array[args->allocation_count];

    //lock mutex
    //pthread_mutex_lock (&mutex);

    for(int i = 0; i < args->allocation_count; i++){
        //call mallocs and store every returned adress
        if((malloc_return_array[i] = malloc(args->allocation_size)) == NULL){
            perror("malloc");
            exit(1);
        }
    }

    //free allocated memory
    for(int i = 0; i < args->allocation_count; i++){
        free(malloc_return_array[i]);
    }

    //unlock
    //pthread_mutex_unlock (&mutex);


    printf("thread finished\n");
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {

    if(argc != 4){
        perror("not 4 commandline arguments\n");
        return EXIT_FAILURE;
    }

    //init mutex
    //pthread_mutex_init (&mutex, NULL);


    //parse args to int base 10
    int max_threads = (int)strtol(argv[1], NULL, 10);
    int allocation_count = (int)strtol(argv[2], NULL, 10);
    int allocation_size = (int) strtol(argv[3], NULL, 10);

    //create argument struct to pass to threads
    thread_args_t* my_args = malloc(sizeof(my_args));
    my_args->allocation_size = allocation_size;
    my_args->allocation_count = allocation_count;

    //create threads with given function to execute
    pthread_t thread_id_array[max_threads];
    for(int i = 0; i < max_threads; i++){
        pthread_create(&thread_id_array[i], NULL, &thread_function, my_args);
    }

    //join, discard thread returns
    for(int i = 0; i < max_threads; i++){
        pthread_join(thread_id_array[i], NULL);
    }

    printf("main program finished\n");

    return EXIT_SUCCESS;
}
