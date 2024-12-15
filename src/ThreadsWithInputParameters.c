/*
As seen in BasicThreads.c, input parameters can be provided to thread executing routines using the void* arg input parameter in
pthread_create. If multiple arguments need to be passed, it's common to pack them into a struct.

In this case, the routine will accept a struct containing an array of size 100000 as well as the amount of elements of the array
meant to be modified. The aim will be to modify the specified amount of elements with random numbers between 1 and 100.
*/

/********* Include statements *********/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // Generate random integer values
#include <time.h>   // Create seed for random integers
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsWithInputParameters.h"

/**************************************/

/********** Define statements *********/

#define MAX_ARRAY_SIZE          100000

#define THREAD_0_NUMS_TO_FILL   MAX_ARRAY_SIZE
#define THREAD_1_NUMS_TO_FILL   10

/**************************************/

/****** Private type definitions ******/

typedef struct
{
    unsigned long int elements_to_fill;
    int array[MAX_ARRAY_SIZE];
    clock_t start;
    clock_t end;
    char* color;

} ARRAY_AND_LENGTH;

/**************************************/

/**** Private function prototypes *****/

static int getRandomInteger(int min, int max);
static void* routineUsingInputParameters(void* arg);

/**************************************/

/******** Function definitions ********/

static int getRandomInteger(int min, int max)
{
    return (rand() % (max - min + 1) + min);
}

static void* routineUsingInputParameters(void* arg)
{
    // Cast the received input parameter to a suitable type.
    ARRAY_AND_LENGTH* t_arg = (ARRAY_AND_LENGTH*)arg;

    // Set the routine execution starting date
    t_arg->start = clock();

    // pthread_self will retrieve the current thread's ID.
    printf("%sTID: %lu, start:\t%ld\r\n%s", t_arg->color, pthread_self(), t_arg->start, PRINT_COLOR_RESET);
    
    // Manage the provided parameter as expected.
    for(unsigned long i = 0; i < t_arg->elements_to_fill; i++)
        t_arg->array[i] = getRandomInteger(1, 100);

    // Set the routine execution ending date
    t_arg->end = clock();
    printf("%sTID: %lu, end:\t%ld\r\n%s", t_arg->color, pthread_self(), t_arg->end, PRINT_COLOR_RESET);
    
    return NULL;
}

void functionUsingThreadWithParameters()
{
    // Create the identifiers for each thread first.
    pthread_t t_0;
    pthread_t t_1;

    // Initialize an instance of the struct defined above for each thread.
    ARRAY_AND_LENGTH arr_and_len_0 =
    {
        .elements_to_fill   = THREAD_0_NUMS_TO_FILL ,
        .array              = {}                    ,
        .color              = PRINT_COLOR_PURPLE    ,
    };

    ARRAY_AND_LENGTH arr_and_len_1 =
    {
        .elements_to_fill   = THREAD_1_NUMS_TO_FILL ,
        .array              = {}                    ,
        .color              = PRINT_COLOR_CYAN      ,
    };

    // For random integers to be generated, create the seed for those.
    srand(time(0));

    // Now, create each thread.
    if(checkThreadCreationStatus( pthread_create(&t_0, NULL, routineUsingInputParameters, &arr_and_len_0) ))
        return;
    
    if(checkThreadCreationStatus( pthread_create(&t_1, NULL, routineUsingInputParameters, &arr_and_len_1) ))
    {
        pthread_join(t_0, NULL);
        return;
    }

    pthread_join(t_0, NULL);
    pthread_join(t_1, NULL);
}

/**************************************/
