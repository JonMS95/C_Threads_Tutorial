/*
At times, it may happen that within a thread's routine, a mutex is waiting for another to be unlocked beforehand, while another
thread requires the opposite process to happen. For instance, if:
    ·Thread 1 requires to lock mutexes 1 and 2 (in the specified order),
    ·Thread 2 requires to lock mutexes 2 and 1 (in the specified order),
a situation known as "Deadlock" may occur. When using common mutexes, the program will get stuck since each thread is waiting for
the other to release locked resources to go on with their tasks.

There are many strategies to prevent these deadlocks, being timed mutexes some of the most common ones. This kind of mutexes are
pretty similar to a common mutex, but waiting for a specified time span to be elapsed before a thread quits trying to lock the target
mutex.

Note that timed mutexes are not kind of a deadline which sets the time at which the mutex will be released by default, but a
point in time from which a thread will not try anymore to lock it if it has not been already released. Thus, it can be regarded as
kind of a "timed trylock".

This way, if the situation previously described happens, and "1" mutex is timed and locked by thread B, it will be unlocked sooner
or later (as it has a deadline for that to happen) and it will eventually let thread A continue its own path.

Timed mutexes are not a whole different type of variable, but common mutexes that have been locked in a special way instead. To do so,
syntax below must be followed:

pthread_mutex_timedlock(pthread_mutex_t* mutex, struct timespec* time)

Where:
    ·mutex: is the mutex variable type for which a timeout is to be set.
    ·time: timeout for the target mutex to be released if it has not been already done explicitly.
    Returns 0 for success, or an error code otherwise.

Note that such function may not be available in every environment. If so, try to compile with -D_XOPEN_SOURCE=700 flag.
*/

/********* Include statements *********/

#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsWithTimedMutex.h"

/**************************************/

/********** Define statements *********/

#define MUTEX_LOCK_TIMEOUT_OFFSET_A 3
#define MUTEX_LOCK_TIMEOUT_OFFSET_B 1
#define SIMULATED_WORK_TIME         1

/**************************************/

/****** Private type definitions ******/

typedef struct
{
    pthread_mutex_t* m_1;
    pthread_mutex_t* m_2;
    char* color;
    unsigned int time_offset;
    unsigned int work_time;
} SHARED_MUTEXES;

/**************************************/

/********* Private variables **********/

/**************************************/

/**** Private function prototypes *****/

static int lockTimedMutex(pthread_mutex_t* mutex, struct timespec* timeout);
static void* generalThreadRoutine(void* arg);
static void* threadARoutine(void* arg);
static void* threadBRoutine(void* arg);

/**************************************/

/******** Function definitions ********/

static int lockTimedMutex(pthread_mutex_t* mutex, struct timespec* timeout)
{
    int ret = pthread_mutex_timedlock(mutex, timeout);

    if(ret == 0)
        printf("%sTimed mutex (addr: %p) succesfully locked.%s\r\n",
                PRINT_COLOR_GREEN   ,
                mutex               ,
                PRINT_COLOR_RESET   );
    else
        printf("%sAn error happened while trying to lock timed mutex (addr: %p). Error: %s.%s\r\n",
                PRINT_COLOR_RED     ,
                mutex               ,
                strerror(ret)       ,
                PRINT_COLOR_RESET   );
    
    return ret;
}

static void* generalThreadRoutine(void* arg)
{
    SHARED_MUTEXES* shared_mutexes = (SHARED_MUTEXES*)arg;

    struct timespec m_1_timeout, m_2_timeout;

    // Lock mutex 1 (mutex 2 for B thread's routine) first. Set a timeout for timed mutex locking.
    printf("%sThread with ID: %lu locking mutex in address %p.%s\r\n",
            shared_mutexes->color   ,
            pthread_self()          ,
            shared_mutexes->m_1     ,
            PRINT_COLOR_RESET       );

    m_1_timeout.tv_nsec = 0;
    m_1_timeout.tv_sec  = time(NULL) + shared_mutexes->time_offset;

    if(lockTimedMutex(shared_mutexes->m_1, &m_1_timeout))
        return NULL;

    // Simulate some work to be done so that deadlock happens ...
    sleep(shared_mutexes->work_time);

    // Try to lock mutex 2 (mutex 1 for B thread's routine).
    printf("%sThread with ID: %lu trying to lock mutex in address %p.%s\r\n",
            shared_mutexes->color   ,
            pthread_self()          ,
            shared_mutexes->m_2     ,
            PRINT_COLOR_RESET       );
    
    m_2_timeout.tv_nsec = 0;
    m_2_timeout.tv_sec  = time(NULL) + shared_mutexes->time_offset;

    if(lockTimedMutex(shared_mutexes->m_2, &m_2_timeout))
    {
        pthread_mutex_unlock(shared_mutexes->m_1);
        return NULL;
    }

    printf("%sThread with ID: %lu finishing routine now.%s\r\n", shared_mutexes->color, pthread_self(), PRINT_COLOR_RESET);

    pthread_mutex_unlock(shared_mutexes->m_1);
    pthread_mutex_unlock(shared_mutexes->m_2);

    return NULL;
}

// Each thread's routine should create its very own struct for shared mutexes.
// This way, mutex addresses are exchanged leading the code to include just a single general routine.
static void* threadARoutine(void* arg)
{
    SHARED_MUTEXES shared_mutexes = *((SHARED_MUTEXES*)arg);
    shared_mutexes.color = PRINT_COLOR_CYAN;
    shared_mutexes.time_offset = MUTEX_LOCK_TIMEOUT_OFFSET_A;

    return generalThreadRoutine(&shared_mutexes);
}

// Thread B's routine is the same as A's, but having exchanged mutex addresses beforehand.
static void* threadBRoutine(void* arg)
{
    SHARED_MUTEXES shared_mutexes = *((SHARED_MUTEXES*)arg);

    // Exchange addresses for both mutex pointer variables.
    pthread_mutex_t* temp = shared_mutexes.m_1;
    shared_mutexes.m_1 = shared_mutexes.m_2;
    shared_mutexes.m_2 = temp;

    shared_mutexes.color = PRINT_COLOR_PURPLE;
    shared_mutexes.time_offset = MUTEX_LOCK_TIMEOUT_OFFSET_B;

    return generalThreadRoutine(&shared_mutexes);
}

void functionUsingThreadWithTimedMutex()
{
    // Declare threads first.
    pthread_t t_A, t_B;

    // Create both mutexes and initialize them.
    SHARED_MUTEXES shared_mutexes;

    pthread_mutex_t mutex_1, mutex_2;

    pthread_mutex_init(&mutex_1, NULL);
    pthread_mutex_init(&mutex_2, NULL);

    shared_mutexes.m_1 = &mutex_1;
    shared_mutexes.m_2 = &mutex_2;

    // Time offset is not set since it should be different for each thread.
    shared_mutexes.work_time = SIMULATED_WORK_TIME;

    // Run each thread now.
    if(checkThreadCreationStatus( pthread_create(&t_A, NULL, threadARoutine, &shared_mutexes) ))
        return;

    if(checkThreadCreationStatus( pthread_create(&t_B, NULL, threadBRoutine, &shared_mutexes) ))
    {
        pthread_join(t_A, NULL);
        return;
    }

    pthread_join(t_A, NULL);
    pthread_join(t_B, NULL);

    pthread_mutex_destroy(shared_mutexes.m_1);
    pthread_mutex_destroy(shared_mutexes.m_2);
}

/**************************************/
