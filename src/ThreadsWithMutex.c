/*
As seen in ThreadsWithInputParameters.c, two different threads can work with different variables at the same time. However, the case in which
different threads access and modify the same variable has not been explained yet. For this purpose, mutex mechanism is used.

Mutex is short for "MUTual EXclusion", which is kind of a lock that prevents other threads to interact with a variable once it has been locked.
This mechanism is commonly assigned to a variable or more generally, to a memory zone. Once it has been locked, other threads will be unable to
access it (either for reading or writing) until the thread which locked it unlocks it again.

Mutex creation syntax is the following:

pthread_mutex_t lock;
pthread_mutex_init(&lock, NULL);

Note that a pthread_mutex_t type variable must be created first. 

Once the mutex lock has been created, the critical section should be enclosed by mutex lock/unlock statements, so that the section's execution
is restricted to a single thread.

pthread_mutex_lock(&lock);
// Critical section
pthread_mutex_unlock(&lock);

For example, if a thread locks the mutex, and another thread tries to lock it shortly after, it will have to wait until the first thread unlocks it
in order to execute its very own critical section.

If no mutex is used it can lead to race conditions. In this example, it will be shown what happens when different threads try to increment a common
counter both with and without any mutex lock usage.
*/

/********* Include statements *********/

#include <pthread.h>
#include <stdio.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsWithMutex.h"

/**************************************/

/********** Define statements *********/

#define NUMBER_OF_THREADS       7
#define NUMBER_OF_INCREMENTS    1000000

/**************************************/

/********* Private variables **********/

static unsigned long counter;
static pthread_mutex_t lock;
static int use_mutex;

/**************************************/

/**** Private function prototypes *****/

static void* incrementFunction(void* arg);
static int createThreadsAndRun();

/**************************************/

/******** Function definitions ********/

static void* incrementFunction(void* arg)
{
    // First, lock the critical section (if allowed) so that no other thread but the current one can manipulate
    // the variable taken as input parameter.
    if(use_mutex)
        pthread_mutex_lock(&lock);

    unsigned long* p_cnt = (unsigned long*)arg;

    for(int i = 0; i < NUMBER_OF_INCREMENTS; i++)
    {
        ++(*(p_cnt));
    }

    // Unlock the mutex for other threads to be able to use the counter variable.
    if(use_mutex)
        pthread_mutex_unlock(&lock);

    return NULL;
}

static int createThreadsAndRun()
{
    counter = 0;
    pthread_t threads[NUMBER_OF_THREADS];

    if(use_mutex)
        pthread_mutex_init(&lock, NULL);

    // Declare a function to the target routine to be executed.

    for(int i = 0; i < sizeof(threads) / sizeof(threads[0]); i++)
    {
        if(checkThreadCreationStatus( pthread_create(&threads[i], NULL, incrementFunction, &counter) ))
        {
            if(use_mutex)
                pthread_mutex_destroy(&lock);
            
            return -1;
        }
    }

    for(int i = 0; i < sizeof(threads) / sizeof(threads[0]); i++)
        pthread_join(threads[i], NULL);
    
    printf("%sFinal counter value (%sUSING MUTEX):\t%lu%s\r\n"   ,
            (use_mutex ? PRINT_COLOR_GREEN : PRINT_COLOR_RED)   ,
            (use_mutex ? "" : "NOT ")                           ,
            counter, PRINT_COLOR_RESET                          );
    
    if(use_mutex)
        pthread_mutex_destroy(&lock);

    return 0;
}

void functionUsingThreadWithoutMutex()
{
    printf("%sNot using Mutex:%s\r\n", PRINT_COLOR_YELLOW, PRINT_COLOR_RESET);
    use_mutex = 0;
    createThreadsAndRun();

    printf("\r\n%sUsing Mutex:%s\r\n", PRINT_COLOR_YELLOW, PRINT_COLOR_RESET);
    use_mutex = 1;
    createThreadsAndRun();
}

/*
As seen on the resulting counters, final values are way different depending on the mutex usage. Here is what happens when a integer-like variable
is incremented in C (var++):
1-The variable is fetched, and a copy of it is stored within a register.
2-The variable stored in the register gets incremented.
3-The variable is returned, storing the resulting value in the input variable.

Therefore, if no mutex lock is used, it may happen multiple threads copy the resulting value in the same memory address at the same time, leading to
the variable not getting incremented whatsoever (as different threads have stored the same value in the counter).

Mutex using function, instead, copies the variable's value, increment it and then store it into the input variable's memory address one by one, so it's
guaranteed that no thread but the current one modifies the variable.
*/

/**************************************/
