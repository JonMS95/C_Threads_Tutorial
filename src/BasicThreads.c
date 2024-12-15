/********* Include statements *********/

#include <pthread.h>
#include <stdio.h>
#include "ThreadCreationStatus.h"
#include "ThreadColors.h"
#include "BasicThreads.h"

/**************************************/

/*
--------------------------------------------------------------------------------------------------------------------------------
What is a thread?
································································································································
A thread is the smallest sequence of programmed instructions that can be managed independently by a scheduler. Threads allow
a program to perform multiple tasks simultaneously. In multithreaded applications, multiple threads run within a single process
and share resources such as heap memory, although each thread has its own stack memory space and registers. In contrast, each
process has its very own memory resources, making context switching slower compared to threads.
--------------------------------------------------------------------------------------------------------------------------------

--------------------------------------------------------------------------------------------------------------------------------
Advantages of threads
································································································································
·Concurrency: same as processes, they make it possible for many tasks to be performed in parallel, improving efficiency.
·Shared memory: as threads do exist within the same process, they can share data directly without any need for inter-process
communication (also known as IPC).
·Lower overhead: threads are way lighter than processes since they share many common resources.
--------------------------------------------------------------------------------------------------------------------------------

--------------------------------------------------------------------------------------------------------------------------------
Including the pthread library
································································································································
To use threads in C, pthread header file must be included.
#include <pthread.h>
When compiling, -pthread flag must be used too.
gcc -pthread example_program.c -o example program
--------------------------------------------------------------------------------------------------------------------------------

--------------------------------------------------------------------------------------------------------------------------------
Basic thread creation
································································································································
To create a thread, pthread_create function has to be called. This function has the following signature:

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void* (*start_routine)(void *), void *arg);

Where:
·thread: a pointer to pthread_t which will store the ID of the created thread.
·attr: a pointer to thread attributes (set to NULL for default attributes).
·start_routine: a pointer to the function that the thread will execute. Note that for the sake of genericness, both input
parameters as well as return values are of type void*.
·arg: a pointer that will be passed to the start routine. Again, void* is used so data of any type can be passed to it.

pthread_create may return either 0 (for successful thread creation) or a positive integer error code, being the following some
of the common ones (all of the following are defined within <errno.h> file):
·EAGAIN: the system lacks the necessary resources for a new thread to be created,
·EINVAL: the thread attributes provided (invalid stack size or scheduling policy) are not suitable for the thread to be created.
·EPERM: the caller has no permission to set the specified scheduling policies.
--------------------------------------------------------------------------------------------------------------------------------
*/

/**** Private function prototypes *****/

static void* basicThreadRoutine(void* arg);

/**************************************/

/******** Function definitions ********/

static void* basicThreadRoutine(void* arg)
{
    // pthread_self will retrieve the current thread's ID.
    printf("%sThis is a thread! TID: %lu%s\r\n", PRINT_COLOR_BLUE, pthread_self(), PRINT_COLOR_RESET);
    return NULL;
}

// This function will create a thread within itself.
void basicThreadUsingFunction()
{
    // First, create the variable that will later store the thread ID.
    pthread_t t_0;

    // Create a new thread. By now, no attributes nor input parameters will be used.
    if( checkThreadCreationStatus( pthread_create(&t_0, NULL, basicThreadRoutine, NULL) ) )
        return;

    // pthread_join will wait for the thread with the given thread ID to finish. If not, the thread creating function will
    // go on regardless of the created thread's state.
    pthread_join(t_0, NULL);
}

/**************************************/
