#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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

    printf("thread finished\n");
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {

    if(argc != 4){
        perror("not 4 commandline arguments\n");
        return EXIT_FAILURE;
    }


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
