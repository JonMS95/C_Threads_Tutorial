/*
A timed wait is used when a thread wants towait for a condition to become true, but it does not want to wait forever.
Instead, a specific amount of time is set. If such condition does not become true within the expected time span, then
the thread in question quits waiting and moves on.

Note the subtle difference between timed waits and timed locks: while timed locks try to acquire a lock before a deadline
is reached, timed waits block their thread execution routine until either a condition happens or a timeout occurs (its fate
will be decided depending on what happens first).

The prototype for this lesson's key function is shown below:

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

Where:
    ·cond: is the condition variable to wait on.
    ·mutex: is just the mutex protecting the shared condition.
    ·abstime: absolute timeout (same as in timed locks).
    ·Returns 0 on success or an error code on failure.

Overall, it can be stated that timed waits behave as if they were kind of a lending system. Thread A owns the mutex first,
then it releases it waiting for another thread (B) to acquire it for a limited time. If such timeout has been reached or not
will decide how thread A behaves afterwards.
*/

/********* Include statements *********/

#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsWithTimedWait.h"

/**************************************/

/********** Define statements *********/

// "Happy-path" case.
#define SIGNALER_INITIAL_DELAY_ON_TIME  3
#define WORKER_WAIT_OFFSET_ON_TIME      5

// "Timeout reached" case.
#define SIGNALER_INITIAL_DELAY_EXPIRED  5
#define WORKER_WAIT_OFFSET_EXPIRED      3

/**************************************/

/****** Private type definitions ******/

typedef struct
{
    pthread_mutex_t*    p_mutex     ;
    pthread_cond_t*     p_condition ;
} COMMON_DATA;

typedef struct
{
    COMMON_DATA*    p_common_data;
    unsigned long*  p_wait_offset;
} WORKER_DATA;

typedef struct
{
    COMMON_DATA*    p_common_data   ;
    unsigned long*  p_initial_delay ;
} SIGNALER_DATA;

/**************************************/

/**** Private function prototypes *****/

static void* workerThreadRoutine(void* arg);
static void* signalerThreadRoutine(void* arg);
static void timedWaitTestCase(unsigned long worker_time_offset, unsigned long signaler_initial_delay);

/**************************************/

/******** Function definitions ********/

static void* workerThreadRoutine(void* arg)
{
    WORKER_DATA* worker_data = (WORKER_DATA*)arg;

    struct timespec timeout;

    timeout.tv_nsec = 0;
    timeout.tv_sec  = time(NULL) + *(worker_data->p_wait_offset);

    // Lock the mutex first.
    pthread_mutex_lock(worker_data->p_common_data->p_mutex);

    printf("%sWorker thread (TID: %lu) has effectively locked the mutex.%s\r\n",
            PRINT_COLOR_CYAN    ,
            pthread_self()      ,
            PRINT_COLOR_RESET   );
    
    // Release mutex and wait for signaled condition to be met again.
    int ret = pthread_cond_timedwait(   worker_data->p_common_data->p_condition ,
                                        worker_data->p_common_data->p_mutex     ,
                                        &timeout                                );

    // If such condition is not signaled, then release the mutex and exit the function.
    if(ret == ETIMEDOUT)
    {
        printf("%sWorker thread's (TID: %lu) timeout has expired without condition having been signaled again.%s\r\n",
                PRINT_COLOR_RED     ,
                pthread_self()      ,
                PRINT_COLOR_RESET   );
        
        pthread_mutex_unlock(worker_data->p_common_data->p_mutex);
        
        return NULL;
    }
    
    printf("%sWorker thread's (TID: %lu) conditon signaled again! Acquiring previously lent mutex back.%s\r\n",
            PRINT_COLOR_GREEN   ,
            pthread_self()      ,
            PRINT_COLOR_RESET   );
    
    pthread_mutex_unlock(worker_data->p_common_data->p_mutex);

    return NULL;
}

static void* signalerThreadRoutine(void* arg)
{
    SIGNALER_DATA* signaler_data = (SIGNALER_DATA*)arg;

    // Forcefully delay thread's execution routine.
    sleep(*(signaler_data->p_initial_delay));

    // Lock mutex.
    pthread_mutex_lock(signaler_data->p_common_data->p_mutex);

    // Signal condition.
    pthread_cond_signal(signaler_data->p_common_data->p_condition);

    // Unlock mutex.
    pthread_mutex_unlock(signaler_data->p_common_data->p_mutex);

    printf("%sCondition signaled by signaler thread (TID: %lu).%s\r\n",
            PRINT_COLOR_PURPLE  ,
            pthread_self()      ,
            PRINT_COLOR_RESET   );

    return NULL;
}

static void timedWaitTestCase(unsigned long worker_time_offset, unsigned long signaler_initial_delay)
{
    // Declare both threads.
    pthread_t t_0, t_1;

    // Declare mutex variable.
    pthread_mutex_t mutex;

    // Declare shared condition.
    pthread_cond_t cond;

    // Initialize mutex and condition.
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // Create data struct instances for both.
    COMMON_DATA common_data =
    {
        .p_condition    = &cond ,
        .p_mutex        = &mutex,
    };

    WORKER_DATA worker_data =
    {
        .p_common_data = &common_data       ,
        .p_wait_offset = &worker_time_offset,
    };

    SIGNALER_DATA signaler_data =
    {
        .p_common_data      = &common_data              ,
        .p_initial_delay    = &signaler_initial_delay   ,
    };

    // Initialize worker (t_0) and signaler (t_1) threads.
    if(checkThreadCreationStatus( pthread_create(&t_0, NULL, workerThreadRoutine, &worker_data) ))
        return;
    
    if(checkThreadCreationStatus( pthread_create(&t_1, NULL, signalerThreadRoutine, &signaler_data) ))
    {
        pthread_join(t_0, NULL);
        return;
    }

    // Wait for both threads to join main thread again.
    pthread_join(t_0, NULL);
    pthread_join(t_1, NULL);

    // Destroy mutex and condition variables.
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void functionUsingThreadWithTimedWait()
{
    // Case in which the condition is signaled on time.
    printf("%sTesting \"happy-path\" case:%s\r\n", PRINT_COLOR_YELLOW, PRINT_COLOR_RESET);
    timedWaitTestCase(WORKER_WAIT_OFFSET_ON_TIME, SIGNALER_INITIAL_DELAY_ON_TIME);

    // Case in which timeout for condition expires.
    printf("\r\n%sTesting \"timeout reached\" case:%s\r\n", PRINT_COLOR_YELLOW, PRINT_COLOR_RESET);
    timedWaitTestCase(WORKER_WAIT_OFFSET_EXPIRED, SIGNALER_INITIAL_DELAY_EXPIRED);
}

/**************************************/
