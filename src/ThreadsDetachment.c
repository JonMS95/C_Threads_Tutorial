/*
By now, only synchronized threads have been explained. When threads are synchronized, main thread can somehow retrieve information from other
threads. However, there is a way to detach those derived threads from the main one, so main thread has no longer any control of what's going
on in any other thread, making it unable to join them or wait for them to finish whatsoever.

Some key points about detached threads are the following:
·No synchronization: as stated above, the detached thread and the main thread do not know about each other after having been detached, so the
main thread cannot wait for them to join nor know about their status any longer.
·Resources are automatically cleaned up: once the detached thread finishes, resources such as memory used by the detached thread areç
automatically managed (recovered) by the system.
·Detached thread continues its own path: once a thread is detached, it will continue until it accomplishes its task no matter if the main
thread has already finished or not (if some conditions explained below are met).

What happens if the main thread ends?
When the main thread finishes, it typically end the whole programs, regardless of the state of any other thread found within the process.
If main thread's main function returns or calls exit, the whole process is ended, killing all of the threads involved. Nevertheless, there still
exists a workaround to make detached thread keep running: using pthread_exit. This will end the main thread, without terminating every other
thread on its way. Its syntax is the following:

pthread_exit(void* retval)

Even though there's no explicit example about this in the current file, keep in mind that any thread (apart from the main one) can terminate itself. 

To sum up, the main thread will prevent any other thread from getting terminated when using pthread_exit instead of simply returning from its
main function. pthread_exit() terminates the calling thread but allows other threads to continue running.
*/

/********* Include statements *********/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsDetachment.h"

/**************************************/

/********** Define statements *********/

#define TIME_BEFORE_DETACHMENT          1

/**************************************/

/**** Private function prototypes *****/

static int createAndDetach();
static void* detachedThreadRoutine(void* arg);

/**************************************/

/******** Function definitions ********/

static int createAndDetach()
{
    pthread_t t_0;
    
    if( checkThreadCreationStatus( pthread_create(&t_0, NULL, detachedThreadRoutine, NULL) ) )
        return -1;

    pthread_detach(t_0);
    return 0;
}

static void* detachedThreadRoutine(void* arg)
{
    // Wait for 5 seconds before doing anything so that it can be noted how thread does not continue if pthread_exit is not being called
    // in the main thread.
    sleep(TIME_BEFORE_DETACHMENT);

    printf("%sThis is a detached thread! TID: %lu%s\r\n", PRINT_COLOR_CYAN, pthread_self(), PRINT_COLOR_RESET);
    return NULL;
}

void threadsDetachment()
{
    // Main thread is terminated before detached thread ends.
    printf("%sMain thread is terminated before detached thread ends.%s\r\n", PRINT_COLOR_YELLOW, PRINT_COLOR_RESET);
    if(createAndDetach() < 0)
        return;

    printf("%sExiting main thread now (TID: %lu).%s\r\n", PRINT_COLOR_PURPLE, pthread_self(), PRINT_COLOR_RESET);
    pthread_exit(NULL);
}

/*
As explained before, detached thread will wait 5 seconds in any case. If main thread does not exit, then it will terminate the whole process
including every thread involved. Since just one of the two cases above uses pthread_exit, terminal output will display just one of the messages
printed by the thread execution routine.
*/

/**************************************/
