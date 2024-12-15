/*
Sames as mutexes, semaphores are a mechanism that allow or prevent threads from accessing critical code sections. In terms of code,
semaphores are variables that are used to control access in a concurrent system. They have non-negative integer value and support
two primary operations:
·wait (P operation): decrease the semaphore's value if it's greater than zero. Otherwise (equal to zero), the calling threads are
blocked until the semaphore's value is greater than zero again.
·signal (V operation): increase the semaphore's value, arguably unblocking a waiting thread.

The syntax for semaphores is the following:
·sem_t: semaphore type. It is used when a variable is declared.
·sem_init(sem_t *sem, int pshared, unsigned int value): initializes the semaphore, where:
    ·sem: a pointer to the sem_t type variable.
    ·pshared: tells whether the semaphore is meant to be shared with other processes (1) or not (0). "pshared" stands for "process-shared".
    ·value: the initial value for the semaphore. This will establish the amount of slots for the semaphore, so use 1 for binary semaphores
    (as it can be just 0 or 1), or a greater number for counting semaphores.
    ·Returns 0 on success, < 0 on failure (specifying an error code retrievable by using errno.h library).
·sem_wait(sem_t* sem): waits until the semaphore which address has been provided allows the current thread to proceed.
·sem_post(sem_t* sem): signals the semaphore. It's kind of a way to tell the semaphore that the current thread is no longer going
through a critical code section.
·sem_getvalue(sem_t *sem, int *sval): retrieves semaphore's current counter without modifying it at all. Returns 0 on success, < 0 on failure.
·sem_destroy(sem_t* sem): destroys the provided semaphore instance.
·sem_open(const char *name, int oflag, mode_t mode, unsigned int value): creates or opens a named semaphore.
·sem_close(sem_t *sem): closes a named semaphore in the current process. It does not remove the semaphore whatsoever, but just removes the reference
from the process instead.
·sem_unlink(const char *name): removes a named semaphore from the system. Any processes using the semaphore will be able to still use it, but no new
processes will be able to do so.

Note that semaphore.h library (which is part of the POSIX standard) should be included for those functions and types to be known/used. 

Semaphores are useful for:
·Mutual exclusion: a limited amouont of threads can access the same code section in parallel. This is the case of binary semaphores.
·Limiting resource access: counting semaphores are useful for limiting the number of threads accessing a certain resource.

Types of semaphores:
·Binary semaphores: these only hold values of 0 or 1 and are used for mutual exclusion, same as mutexes. In fact, mutexes can be
regarded as a particular case of semaphores with just two possible values.
·Counting semaphores: can hold values greater than 1, allowing more than a single thread to access the same resource.

Two examples will be shown in this case, one for each semaphore type.
*/

/********* Include statements *********/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsWithSemaphores.h"

/**************************************/

/********** Define statements *********/

#define MAX_ITERATIONS_NUMBER   100000
#define MAX_COUNTING_SEM_SLOTS  3
#define MAX_NUM_OF_THREADS      5 

/**************************************/

/****** Private type definitions ******/

typedef struct
{
    unsigned long*  p_counter   ;
    sem_t*          p_semaphore ;
} BINARY_SEMAPHORE_DATA;

typedef struct
{
    char*           color       ;
    sem_t*          p_semaphore ;
} COUNTING_SEMAPHORE_DATA;

/**************************************/

/**** Private function prototypes *****/

static void testBinarySemaphores();
static void* binarySemaphoreRoutine(void* arg);
static void testCountingSemaphores();
static void* countingSemaphoreRoutine(void* arg);

/**************************************/

/******** Function definitions ********/

static void* binarySemaphoreRoutine(void* arg)
{
    BINARY_SEMAPHORE_DATA* bsd = (BINARY_SEMAPHORE_DATA*)arg;

    // Enable thread cancellation so that in case of thread creation failure, it can be terminated from the outside.
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    // Count until the maximum number of iterations is reached.
    for(int i = 0; i < MAX_ITERATIONS_NUMBER; i++)
    {
        // Wait for the semaphore. Equivalent to locking a mutex.
        sem_wait(bsd->p_semaphore);

        // Increment the counter.
        ++( *(bsd->p_counter) );

        // once, the critical code section is over, post (unlock) the semaphore.
        sem_post(bsd->p_semaphore);
    }

    return NULL;
}

static void testBinarySemaphores()
{
    // Create thread variables.
    pthread_t t_0;
    pthread_t t_1;

    // Create binary semaphore variable.
    sem_t bin_sem;

    // Create a counter meant to be incremented by both threads at the same time.
    unsigned long counter = 0; 

    // Initialize the semaphore. As it's meant to be binary, the greatest number it can hold must be 1. It's not going to be
    // shared with any other process.
    sem_init(&bin_sem, 0, 1);

    BINARY_SEMAPHORE_DATA bin_sem_data = 
    {
        .p_counter      = &counter      ,
        .p_semaphore    = &bin_sem      ,
    };

    // Once it's done, create threads by passing them the task to accomplish as well as the binary semaphore's address.
    if(checkThreadCreationStatus( pthread_create(&t_0, NULL, binarySemaphoreRoutine, &bin_sem_data) ))
        return;
    
    if(checkThreadCreationStatus( pthread_create(&t_1, NULL, binarySemaphoreRoutine, &bin_sem_data) ))
    {
        pthread_cancel(t_0);
        return;
    }

    pthread_join(t_0, NULL);
    pthread_join(t_1, NULL);

    sem_destroy(&bin_sem);

    printf( "%sCounter value after having ended both threads controlled by a binary semaphore: %lu%s\r\n",
            PRINT_COLOR_CYAN            ,
            *(bin_sem_data.p_counter)   ,
            PRINT_COLOR_RESET           );
}

static void* countingSemaphoreRoutine(void* arg)
{
    COUNTING_SEMAPHORE_DATA* csd = (COUNTING_SEMAPHORE_DATA*)arg;

    // Enable thread cancellation so that in case of thread creation failure, it can be terminated from the outside.
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    // Wait for the counting semaphore to allow the current thread to proceed.
    sem_wait(csd->p_semaphore);

    // Simulate a connection request to server socket.
    sleep(1);

    // Print the number of threads currently working in the critical section.
    int semaphore_free_slots;
    sem_getvalue(csd->p_semaphore, &semaphore_free_slots);
    printf( "%sThread with TID: %ld says: \"Number of semaphore's free slots: %d%s.\"%s\r\n",
            csd->color                                                          ,
            pthread_self()                                                      ,
            semaphore_free_slots                                                ,
            (!semaphore_free_slots ? " (no slots available at the moment)" : ""),
            PRINT_COLOR_RESET                                                   );

    // Once it's done, signal the semaphore.
    sem_post(csd->p_semaphore);

    return NULL;
}

static void testCountingSemaphores()
{
    // Create thread variables.
    pthread_t threads[MAX_NUM_OF_THREADS];
    // Assign a color to each thread.
    char* thread_color[MAX_NUM_OF_THREADS] =
    {
        PRINT_COLOR_BLACK   ,
        PRINT_COLOR_BLUE    ,
        PRINT_COLOR_CYAN    ,
        PRINT_COLOR_GREEN   ,
        PRINT_COLOR_PURPLE  ,
    };
    // Create a struct for each thread.
    COUNTING_SEMAPHORE_DATA counting_semaphore_data[MAX_NUM_OF_THREADS];

    // Create binary semaphore variable.
    sem_t counting_sem; 

    // Initialize the semaphore. As it's meant to be counting-type, the greatest number it can hold must be greater than 1.
    // Again, it's not going to be shared with any other process.
    sem_init(&counting_sem, 0, MAX_COUNTING_SEM_SLOTS);

    for(int i = 0; i < sizeof(threads) / sizeof(threads[0]); i++)
    {
        counting_semaphore_data[i].color        = thread_color[i]   ;
        counting_semaphore_data[i].p_semaphore  = &counting_sem     ;

        if( checkThreadCreationStatus( pthread_create( &threads[i], NULL, countingSemaphoreRoutine, &counting_semaphore_data[i] ) ) )
        {
            for(int j = (i - 1); j >= 0; j--)
                pthread_cancel(threads[j]);
            
            return;
        }
    }
    
    for(int i = 0; i < sizeof(threads) / sizeof(threads[0]); i++)
        pthread_join(threads[i], NULL);

    sem_destroy(&counting_sem);
}

void threadsWithSemaphores()
{
    testBinarySemaphores();
    testCountingSemaphores();
}

/**************************************/
