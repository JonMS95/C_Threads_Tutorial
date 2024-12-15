/*
Barriers are a sycnhronization element that allow setting a checkpoint for different threads, without using pthread_join in the main thread.
This way, several threads may execute a task and wait for the barrier to be opened. Once the barrier is opened, they can continue their execution.

The syntax for barriers is the following:

For initialization:
pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count)

Where:
·barrier: a pointer to a barrier. Similar to a mutex, it's a point in the code that locks the execution of the thread, in this case, until the
gets freed.
·barrierattr_t: barrier attributes.
·count: number of threads meant to reach the barrier. Once all of them have met the barrier point, the barrier gets opened and every thread
may continue its execution. This should be used with care, as the number of threads to be using the barrier condition must match the counter's
value. Otherwise, it may lead to race conditions (if count is lower) or deadlocks (if counter is higher).

For destruction:
pthread_barrier_destroy(pthread_barrier_t *barrier)

Where:
·barrier: a pointer to the barrier object that's no longer meant to be used (quite similar to mutex destruction).

*/

/********* Include statements *********/

#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsWithBarrier.h"

/**************************************/

/********** Define statements *********/

#define NUMBER_OF_THREADS       3
#define NUMBER_OF_INCREMENTS_0  10000
#define NUMBER_OF_INCREMENTS_1  1000000
#define NUMBER_OF_INCREMENTS_2  100000000

/**************************************/

/****** Private type definitions ******/

typedef struct
{
    unsigned long       cnt     ;
    pthread_barrier_t*  bar     ;
    char*               clr   ;
} COUNTER_AND_BARRIER;

/**************************************/

/**** Private function prototypes *****/

static void* countUntilLimit(void* arg);

/**************************************/

/******** Function definitions ********/

static void* countUntilLimit(void* arg)
{
    COUNTER_AND_BARRIER* cab = (COUNTER_AND_BARRIER*)arg;

    unsigned long dummy_cnt = 0;

    // Count until the limit is reached.
    for(unsigned long i = 1; i <= cab->cnt; i++)
        ++dummy_cnt;

    // Display thread ID, count result and date to compare with others.
    printf("%sThread ID: %lu has reached its count limit (%lu) at %ld\r\n%s",
            cab->clr,
            pthread_self(),
            dummy_cnt,
            clock(),
            PRINT_COLOR_RESET);

    // once the count limit has been reached, wait for other threads to reach the bar as well.
    pthread_barrier_wait(cab->bar);

    printf("%sBarrier reached, thread ID: %lu goes on at %ld\r\n%s", cab->clr, pthread_self(), clock(), PRINT_COLOR_RESET);

    return NULL;
}

void threadsWithBarrier()
{
    pthread_t threads[NUMBER_OF_THREADS];
    COUNTER_AND_BARRIER cab[NUMBER_OF_THREADS];
    unsigned long counters[NUMBER_OF_THREADS] =
    {
        NUMBER_OF_INCREMENTS_0,
        NUMBER_OF_INCREMENTS_1,
        NUMBER_OF_INCREMENTS_2
    };
    char* colors[] =
    {
        PRINT_COLOR_BLUE    ,
        PRINT_COLOR_BLACK   ,
        PRINT_COLOR_GREEN   ,
    };

    // Create the barrier
    pthread_barrier_t barrier;

    // Initialize the barrier, specifying the number of threads to be controlled.
    pthread_barrier_init(&barrier, NULL, NUMBER_OF_THREADS);

    for(int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        cab[i].cnt = counters[i];
        cab[i].bar = &barrier   ;
        cab[i].clr = colors[i]  ;

        if(checkThreadCreationStatus( pthread_create(&threads[i], NULL, countUntilLimit, &cab[i]) ))
            return;
    }

    for(int i = 0; i < NUMBER_OF_THREADS; i++)
        pthread_join(threads[i], NULL);
}

/*
After executing, it can be noted that the time at which every thread reaches its limit is totally different in each case, whereas the date
for continuing after barrier condition is met is almost the same for all. 
*/

/**************************************/
