/*
As seen in ThreadsWithBarrier.c, barriers are a mechanism that allow different threads to wait for all of the other threads to reach
a specified point on the code, so the barrier does only get opened once that condition is met. After that, all of the other threads are
allowed to go on with their tasks.

However, more complex requirements may need to be met sometimes (apart from waiting for other threads to reach a certain code section),
for which conditions may be used. Similar to barriers, conditions allow threads to:
·Wait for a speciic conditions to become true.
·Get notified when those conditions change so that they can proceed with execution.

Condition variables are paired with mutexes:
·A mutex locks a shared resource, or more generally, a critical code section.
·The condition variable helps threads wait for a certain conditions involving the shared resource or critical code section to be met
before unlocking the mutex.

The syntax for thread conditions is the following:
·pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex): releases the mutex and waits for the condition variable to be signaled.
Once it's signaled, it automatically reacquires the mutex.
·pthread_cond_signal(pthread_cond_t *cond): wakes a thread waiting on the condition variable.
·pthread_cond_broadcast(pthread_cond_t *cond): wakes all threads waiting on the condition variable.

How to use them:
1-Initialize the condition variable and mutex
Use pthread_cond_init for conditions, pthread_mutex_init for mutexes.
2-Wait and signal
    ·A thread can wait on a condition using pthread_cond_wait(). This function releases the mutex and blocks the calling thread until
    thread signals the condition.
    ·When the target condition is met, another thread can call pthread_cond_signal() to wake up a single waiting thread or
    pthread_cond_broadcast() if many threads are meant to be awakened.
3-Destroy the condition variable and mutex
Once it's over, destroy the preiously allocated resources using pthread_cond_destroy() and pthread_mutex_destroy().

A beginner-friendly example:
The following example describes a typical consumer-producer architecture, in which a producer generates items until the buffer is full,
and a consumer waits if the buffer is empty. A shared counter will be used, describing the number of elements in the buffer.
*/

/********* Include statements *********/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsWithConditionVariables.h"

/**************************************/

/********** Define statements *********/

#define BUFFER_SIZE 3

/**************************************/

/****** Private type definitions ******/

typedef struct
{
    pthread_mutex_t*    cond_p_mutex_lock   ;
    pthread_cond_t*     cond_p_full_cond    ;

} COND_THREAD_DATA;

/**************************************/

/********* Private variables **********/

/**************************************/

/**** Private function prototypes *****/

static void* consumerRoutine(void* arg);
static void* producerRoutine(void* arg);

/**************************************/

/******** Function definitions ********/

static void* consumerRoutine(void* arg)
{
    // Make thread cancellable so as to terminate it if required.
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    COND_THREAD_DATA* p_cond_thread_data = (COND_THREAD_DATA*)arg;

    pthread_mutex_lock(p_cond_thread_data->cond_p_mutex_lock);

    printf("%sConsumer with TID: %ld says: \"Waiting for items to be replenished ...\"%s\r\n",
            PRINT_COLOR_RED                         ,
            pthread_self()                          ,
            PRINT_COLOR_RESET                       );

    // Wait until the full buffer condition is signaled by the producer. This action frees the mutex lock implicitly,
    // and forces the thread to wait there.
    pthread_cond_wait(p_cond_thread_data->cond_p_full_cond, p_cond_thread_data->cond_p_mutex_lock);

    // Once the condition above has been signaled, consume all items in the buffer.
    for(int i = BUFFER_SIZE; i >= 1; i--)
    {
        printf("%sConsumer with TID: %ld says: \"Consumed an item. Current item number: %d\"%s\r\n",
            PRINT_COLOR_BLUE                        ,
            pthread_self()                          ,
            i                                       ,
            PRINT_COLOR_RESET                       );

        // Simulate time taken to consume a single item.
        sleep(1);
    }
    
    printf( "%sConsumer with TID: %ld says: \"All elements in buffer have been consumed!\"%s\r\n",
            PRINT_COLOR_PURPLE  ,
            pthread_self()      ,
            PRINT_COLOR_RESET   );

    pthread_mutex_unlock(p_cond_thread_data->cond_p_mutex_lock);

    return NULL;
}

static void* producerRoutine(void* arg)
{
    // Make thread cancellable so as to terminate it if required.
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    COND_THREAD_DATA* p_cond_thread_data = (COND_THREAD_DATA*)arg;

    // Lock the mutex first.
    pthread_mutex_lock(p_cond_thread_data->cond_p_mutex_lock);

    // Then, produce until the buffer is full.
    for(int i = 1; i <= BUFFER_SIZE; i++)
    {
        printf( "%sProducer with TID %ld says: \"Added an item. Current item number: %d\"%s\r\n",
            PRINT_COLOR_YELLOW                      ,
            pthread_self()                          ,
            i                                       ,
            PRINT_COLOR_RESET                       );

        // Simulate some time taken to produce a single unit.
        sleep(1);
    }

    // Once it's done, signal consumer letting it know that the buffer has been already replenished.
    printf( "%sProducer with TID %ld says: \"Buffer is full, signaling consumer ...\"%s\r\n",
            PRINT_COLOR_GREEN   ,
            pthread_self()      ,
            PRINT_COLOR_RESET   );
    
    pthread_cond_signal(p_cond_thread_data->cond_p_full_cond);

    // pthread_cond_signal does not free the mutex lock by itself, since meeting more than just a single condition may be required.
    pthread_mutex_unlock(p_cond_thread_data->cond_p_mutex_lock);
    
    return NULL;
}

void threadsWithConditionVariables()
{
    // Declare all variables set to be used in this example.
    pthread_mutex_t mutex_lock      ;
    pthread_cond_t  full_cond       ;
    pthread_t       consumer_thread ;
    pthread_t       producer_thread ;

    // Initialize both conditions as well as the mutex variables.
    pthread_mutex_init( &mutex_lock ,   NULL);
    pthread_cond_init(  &full_cond  ,   NULL);

    COND_THREAD_DATA cond_thread_data =
    {
        .cond_p_mutex_lock  = &mutex_lock   ,
        .cond_p_full_cond   = &full_cond    ,
    };

    // Start consumer and producer threads.
    if( checkThreadCreationStatus( pthread_create(&consumer_thread, NULL, consumerRoutine, &cond_thread_data) ) )
        return;
    
    // Wait for the consumer to start its own routine so that it's ensured that there's at least a thread waiting for the signal.
    sleep(1);

    if( checkThreadCreationStatus( pthread_create(&producer_thread, NULL, producerRoutine, &cond_thread_data) ) )
    {
        // Since both threads are cancellable, they should be terminated if any error occurs first.
        pthread_cancel(consumer_thread);
        return;
    }

    // Wait for both threads to finish.
    pthread_join(consumer_thread, NULL);
    pthread_join(producer_thread, NULL);

    // Destroy both conditions as well as the mutex variable.
    pthread_mutex_destroy(  &mutex_lock );
    pthread_cond_destroy(   &full_cond  );
}

/**************************************/
