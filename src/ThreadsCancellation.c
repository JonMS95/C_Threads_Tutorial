/*
Apart from threads been terminated by themselves, threads can terminate each other by using:

pthread_cancel(pthread_t t)

The function above allows a thread to terminate other based on the passed thread id. However, it's important to note that cancellation
is just a request, so calling pthread_cancel may not necessairily end the pointed thread.

The request will only be attended once a
cancellation point has been reached. These points are places in which it's safe for the thread to be terminated such as sleep,
pthread_join, or reading from an I/O. If a thread never reaches a cancellation point, it will never be terminated despite having
received a cancellation request.

Note that any thread can cancel other (not only the main thread) as long as it knows the ID of thread to be cancelled.

Asynchronous cancellation does also exit though it's way less common since the thread may be terminated at any point in its execution
regardless of it being passing through a critical section or not. This can lead to leaving shared resources in an inconsistent state.

Even though not being necessary, it's strongly recommended to set the cancellation state of a thread if any cancel request is set to
be received. This can be done by using the following built-in functions:

pthread_setcancelstate(int __state, int *__oldstate)
pthread_setcanceltype(int __type, int *__oldtype)

Where:
·state: set to enable or disable, depending on whether cancellation is meant to be allowed for the current thread.
·type: either deferred or asynchronous. As mentioned before, asynchronous cancellation type is not secure.
·oldstate and oldtype: older status of state and type.

Both return an integer: 0 if the operation was successful, an erro code != 0 otherwise. Common error codes can include EINVAL for
invalid parameters or ESRCH for non-existent threads, among others.
*/

/********* Include statements *********/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsCancellation.h"

/**************************************/

/**** Private function prototypes *****/

static void* cancelledThreadRoutine(void* arg);
static void* cancellingThreadRoutine(void* arg);
static void createCancellableThreads(int cancellable);

/**************************************/

/******** Function definitions ********/

static void* cancelledThreadRoutine(void* arg)
{
    int enable_cancellation = *((int*)arg);
    int initial_cancel_state;
    int initial_cancel_type;

    // By default, make the thread non-cancellable:
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &initial_cancel_state);

    // If threads is set to be cancellable, then enable cancellation.
    if(enable_cancellation)
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &initial_cancel_state);

    // Make cancellation type deferred. If cancellation is not enabled, this line will remain useless.
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &initial_cancel_type);

    printf("%sHello from thread ID: %lu\r\nInitial cancel state: %s, current cancel state: %s\r\nInitial cancel type: %s, current cancel type: %s%s\r\n",
            PRINT_COLOR_PURPLE                                                              ,
            pthread_self()                                                                  ,
            (initial_cancel_state == PTHREAD_CANCEL_ENABLE ? "ENABLED" : "DISABLED")        ,
            (initial_cancel_state ? "ENABLED" : "DISABLED")                                 ,
            (initial_cancel_type == PTHREAD_CANCEL_DEFERRED ? "DEFERRED" : "ASYNCHRONOUS")  ,
            "DEFERRED"                                                                      ,
            PRINT_COLOR_RESET                                                               );
    
    sleep(1);
    
    printf("%sThis point was reached due to cancellation not being enabled for thread ID: %lu%s\r\n",
            PRINT_COLOR_YELLOW  ,
            pthread_self()      ,
            PRINT_COLOR_RESET   );

    return NULL;
}

static void* cancellingThreadRoutine(void* arg)
{
    printf("%sThread ID: %lu sends a request for thread %lu to be cancelled%s\r\n",
            PRINT_COLOR_CYAN    ,
            pthread_self()      ,
            *((pthread_t*)arg)  ,
            PRINT_COLOR_RESET   );
    
    pthread_cancel( *((pthread_t*)arg) );

    return NULL;
}

static void createCancellableThreads(int cancellable)
{
    pthread_t t_0, t_1;

    if(checkThreadCreationStatus( pthread_create(&t_0, NULL, cancelledThreadRoutine, &cancellable) ))
        return;
    
    if(checkThreadCreationStatus( pthread_create(&t_1, NULL, cancellingThreadRoutine, &t_0) ))
    {
        pthread_join(t_0, NULL);
        return;
    }

    pthread_join(t_0, NULL);
    pthread_join(t_1, NULL);
}

void threadsCancellation()
{
    printf("%sTest with cancellation enabled%s\r\n", PRINT_COLOR_GREEN, PRINT_COLOR_RESET);
    createCancellableThreads(1);

    printf("%sTest with cancellation disabled%s\r\n", PRINT_COLOR_GREEN, PRINT_COLOR_RESET);
    createCancellableThreads(0);
}

/*
As explaine before, detached thread will wait 5 seconds in any case. If main thread does not exit, then it will terminate the whole process
including every thread involved. Since just one of the two cases above uses pthread_exit, terminal output will display just one of the messages
printed by the thread execution routine.
*/

/**************************************/
