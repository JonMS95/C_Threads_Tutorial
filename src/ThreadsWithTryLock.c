/*
It may happen sometimes that a thread wants to acquire a mutex that has been previously locked by another thread.
In those situations, there is a wide variety of actions that can be considered, being trylocks one of the most
popular among those.

"Trylocks" are a simple mechanism that make the typical mutex lock procedure non-blocking, leading any thread
not to wait for a potentially-undefined time span before getting target mutex's ownership.

The syntax for this tool is quite simple:

pthread_mutex_trylock(pthread_mutex_t *mutex);

Where:
    ·mutex: is the target mutex meant to be locked by the current thread.
    Returns 0 if the mutex was successfully locked, or an error code otherwise (typically EBUSY).

In the current section, a thread is going to be started. The thread in question will lock a mutex and it will
hold it "for a long time" (let's say, a second). Meanwhile, another thread will try to lock it, acting in
consequence if such lockage is not possible.
*/

/********* Include statements *********/

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsWithTryLock.h"

/**************************************/

/********** Define statements *********/

#define DEFAULT_WORK_TIME   1

/**************************************/

/********* Private variables **********/

/**************************************/

/**** Private function prototypes *****/

static void* threadsWithTryLockRoutine(void* arg);

/**************************************/

/******** Function definitions ********/

static void* threadsWithTryLockRoutine(void* arg)
{
    pthread_mutex_t* p_mutex = (pthread_mutex_t*)arg;

    // Try to lock target mutex first. If unable, exit current routine. Go ahead otherwise.
    int ret = pthread_mutex_trylock(p_mutex);

    if(ret != 0)
    {
        printf("%sThread with ID: %lu is unable to lock mutex in address: %p. Error code: %d (%s).%s\r\n",
                PRINT_COLOR_RED     ,
                pthread_self()      ,
                p_mutex             ,
                ret                 ,
                strerror(ret)       ,
                PRINT_COLOR_RESET   );
        
        return NULL;
    }

    printf("%sMutex in address: %p has been successfully acquired by thread with ID: %lu.%s\r\n",
            PRINT_COLOR_GREEN   ,
            p_mutex             ,
            pthread_self()      ,
            PRINT_COLOR_RESET   );

    // If target mutex gets locked by current thread, simulate some work, then release the mutex.
    sleep(DEFAULT_WORK_TIME);

    // Finally, unlock the mutex and exit the function.
    pthread_mutex_unlock(p_mutex);

    printf("%sThread with ID: %lu exiting its routine.%s\r\n", PRINT_COLOR_GREEN, pthread_self(), PRINT_COLOR_RESET);

    return NULL;
}

void threadsWithTryLock()
{
    // Declare threads and shared mutex variables.
    pthread_t t_0, t_1;
    pthread_mutex_t mutex;

    // Initialize mutex.
    pthread_mutex_init(&mutex, NULL);

    // Initialize threads, passing shared mutex as input parameter for both.
    if(checkThreadCreationStatus( pthread_create(&t_0, NULL, threadsWithTryLockRoutine, &mutex) ))
        return;
    
    if(checkThreadCreationStatus( pthread_create(&t_1, NULL, threadsWithTryLockRoutine, &mutex) ))
    {
        pthread_join(t_0, NULL);
        return;
    }

    pthread_join(t_0, NULL);
    pthread_join(t_1, NULL);

    pthread_mutex_destroy(&mutex);
}

/*
As seen on the example above, a thread effectively locks the mutex whereas the other one tries to acquire it
just once. Had it been more patient, it could have locked the mutex (provided it had waited until the fellow
thread had already unlocked it). Another mechanism related to this question will be explained on further
lessons. 
*/

/**************************************/
