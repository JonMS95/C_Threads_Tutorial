/*
Thread attributes are a feature of POSIX threads that allow control over how those threads operate, such as their scheduling,
stack size, and so on. The initialization of each thread attribute follows the syntax shown below:
pthread_attr_t attr;
pthread_attr_init(&attr);

Where pthread_attr_init is the attribute initializing function. It may return 0 for success, EINVAL for an invalid input parameter,
or any error code.

Thread attributes can determine many of the thread's behavioural aspects, such as:
    ·Joinability: tells whether is the thread joinable or detached by using pthread_attr_setdetachstate. Allowed values are:
        ·PTHREAD_CREATE_JOINABLE: makes the thread joinable by the main thread.
        ·PTHREAD_CREATE_DETACHED: creates a detached thread, so it can not be later joined by the main thread.

    ·Stack size: allow changing the stack size to a custom one instead of default (8 MB for Linux OS systems). Use
        pthread_attr_setstacksize to modify it.
    
    ·Guard size: defines extra memory at the end of the thread's stack (in bytes) so as to avoid stack overflows. Use 
        pthread_attr_setguardsize to set it.
    
    ·Scheduling policy and priority: defines how the OS manages thread execution. Some of the most known policies are:
        ·SCHED_OTHER: default policy, time-shared scheduling.
        ·SCHED_RR: Round-robin policy.
        ·SCHED_FIFO: higher priority yasks complete before lower priority tasks.
        The scheduling priotity adjusts within the policy. Use sched_param.sched_priority to set it (included in <sched.h>).
        Please, note also that scheduling priorities other than default may require root privileges.
    
    ·Inherit scheduling: tells whether are threads meant to inherit scheduling policies from parent threads. Use
        pthread_attr_setinheritsched to set this attribute.
    
    ·Processor affinity: binds threads to specific CPU cores. It's Linux-specific. The syntax to set this feature is the following:
        ·cpu_set_t: a variable that defines the set of CPUs tu be used.
        ·CPU_ZERO: a maro used to clean or initialize a cpu_set_t variable to an empty set.
        ·sched_setaffinity: function used to set thread's CPU affinity.
        Same as cancellability, this is not specified by using pthread_attr_t type variables, but pthread_t type variables instead.
        This topic will not be covered in this tutorial since it may not be suppported by some glibc versions.
    
    ·Cancellability: while it's not set by using pthread_attr_t type variables, it tells whether threads can be terminated
        by others. This topic has been thoroughly explained in ThreadsCancellation.c file. 
*/

/********* Include statements *********/

#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsWithAttributes.h"

/**************************************/

/********** Define statements *********/

#define NUMBER_OF_THREADS       10

#define SIZE_OF_1_KB            1024
#define SIZE_OF_4_KB            SIZE_OF_1_KB * 4
#define SIZE_OF_1_MB_IN_BYTES   SIZE_OF_1_KB * SIZE_OF_1_KB

#define MAX_COUNT_VALUE         UINT_MAX

/**************************************/

/****** Private type definitions ******/

typedef struct sched_param priority_param;

typedef struct
{
    int cancelability_enabled;
    int cancellation_type;
} THREAD_CANCELABILITY;

typedef struct
{
    THREAD_CANCELABILITY cancelability;
    unsigned long max_count_value;
} THREAD_INPUT_COMMON_DATA;

typedef struct
{
    int thread_idx;
    THREAD_INPUT_COMMON_DATA* input_common;
} THREAD_INPUT_DATA;

/**************************************/

/********* Private variables **********/

/**************************************/

/**** Private function prototypes *****/

static int     setExampleThreadAttributes(pthread_attr_t* attr, int* scheduling_policy, priority_param* sched_priority_param);
static void    showExampleThreadAttributes(pthread_attr_t* attr, THREAD_CANCELABILITY* cancellability);
static void*   threadWithAttributesRoutine(void* arg);

/**************************************/

/******** Function definitions ********/

static int setExampleThreadAttributes(pthread_attr_t* attr, int* scheduling_policy, priority_param* sched_priority_param)
{
    int err;
    
    // Set detach state to joinable
    if ((err = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE)) != 0)
    {
        printf("Error setting detach state at line %d: %s\n", __LINE__, strerror(err));
        return err;
    }

    // Set custom stack size (1 MB)
    if ((err = pthread_attr_setstacksize(attr, SIZE_OF_1_MB_IN_BYTES)) != 0)
    {
        printf("Error setting stack size at line %d: %s\n", __LINE__, strerror(err));
        return err;
    }

    // Set guard size (4 KB)
    if ((err = pthread_attr_setguardsize(attr, SIZE_OF_4_KB)) != 0)
    {
        printf("Error setting guard size at line %d: %s\n", __LINE__, strerror(err));
        return err;
    }

    // Set scheduling policy
    if ((err = pthread_attr_setschedpolicy(attr, *scheduling_policy)) != 0)
    {
        printf("Error setting scheduling policy at line %d: %s\n", __LINE__, strerror(err));
        return err;
    }

    // Set scheduling priority
    if ((err = pthread_attr_setschedparam(attr, sched_priority_param)) != 0)
    {
        printf("Error setting scheduling parameter at line %d: %s\n", __LINE__, strerror(err));
        return err;
    }

    // Set explicit scheduling to override inherited scheduling
    if ((err = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED)) != 0)
    {
        printf("Error setting inherit scheduling at line %d: %s\n", __LINE__, strerror(err));
        return err;
    }

    return 0;
}

static void showExampleThreadAttributes(pthread_attr_t* attr, THREAD_CANCELABILITY* cancellability)
{
    int detachstate, inheritsched, schedpolicy;
    size_t stacksize, guardsize;
    struct sched_param param;

    // Get detach state
    if (pthread_attr_getdetachstate(attr, &detachstate) == 0)
        printf("%sDetach state:\t\t%s%s\r\n"                                        ,
                PRINT_COLOR_PURPLE                                                  ,
                (detachstate == PTHREAD_CREATE_JOINABLE) ? "Joinable" : "Detached"  ,
                PRINT_COLOR_RESET                                                   );
    else
        printf("Could not retrieve detach state.\r\n");

    // Get stack size
    if (pthread_attr_getstacksize(attr, &stacksize) == 0)
        printf("%sStack size:\t\t%zu bytes%s\r\n"           ,
                PRINT_COLOR_PURPLE                          ,
                stacksize                                   ,
                PRINT_COLOR_RESET                           );
    else
        printf("Could not retrieve stack size.\r\n");

    // Get guard size
    if (pthread_attr_getguardsize(attr, &guardsize) == 0)
        printf("%sGuard size:\t\t%zu bytes%s\r\n"   ,
                PRINT_COLOR_PURPLE                  ,
                guardsize                           ,
                PRINT_COLOR_RESET                   );
    else
        printf("Could not retrieve guard size.\r\n");

    // Get scheduling policy
    if (pthread_attr_getschedpolicy(attr, &schedpolicy) == 0)
        printf("%sScheduling policy:\t%s%s\r\n"                                         ,
                PRINT_COLOR_PURPLE                                                      , 
                (schedpolicy == SCHED_OTHER) ? "SCHED_OTHER" : 
                (schedpolicy == SCHED_RR) ? "SCHED_RR" : "SCHED_FIFO", PRINT_COLOR_RESET);
    else
        printf("Could not retrieve scheduling policy.\r\n");

    // Get scheduling priority
    if (pthread_attr_getschedparam(attr, &param) == 0)
        printf("%sScheduling priority:\t%d%s\r\n"   ,
                PRINT_COLOR_PURPLE                  ,
                param.sched_priority                ,
                PRINT_COLOR_RESET                   );
    else
        printf("Could not retrieve scheduling priority.\r\n");

    // Get inherit-scheduling attribute
    if (pthread_attr_getinheritsched(attr, &inheritsched) == 0)
        printf("%sInherit scheduling:\t%s%s\r\n"                                ,
                PRINT_COLOR_PURPLE                                              , 
                (inheritsched == PTHREAD_INHERIT_SCHED) ? "Inherit" : "Explicit",
                PRINT_COLOR_RESET                                               );
    else
        printf("Could not retrieve inherit-scheduling attribute.\r\n");
    
    if(cancellability == NULL)
        printf("Could not retrieve cancellability details.\r\n");
    else
        printf("%sCancelability:\t\t%s\r\nCancellation type:\t%s%s\r\n"                                         ,
                PRINT_COLOR_PURPLE                                                                              ,
                cancellability->cancelability_enabled == PTHREAD_CANCEL_DISABLE ? "DISABLED" : "ENABLED"        ,
                cancellability->cancellation_type == PTHREAD_CANCEL_ASYNCHRONOUS ? "ASYNCHRONOUS" : "DEFERRED"  ,
                PRINT_COLOR_RESET                                                                               );
}

static void* threadWithAttributesRoutine(void* arg)
{
    THREAD_INPUT_DATA* thread_input_data = (THREAD_INPUT_DATA*)arg;
    THREAD_CANCELABILITY cancelability = thread_input_data->input_common->cancelability;

    // Set cancellability just after having entered thread routine.
    pthread_setcanceltype(cancelability.cancellation_type, NULL);
    pthread_setcancelstate(cancelability.cancelability_enabled, NULL);

    clock_t start_time = clock();

    printf("%sThread with index %d (Thread ID: %lu) start time: %ld%s\r\n"  ,
            PRINT_COLOR_GREEN                                               ,
            thread_input_data->thread_idx                                   ,
            pthread_self()                                                  ,
            clock()                                                         ,
            PRINT_COLOR_RESET                                               );

    unsigned int dummmy_counter = 0;

    for(unsigned int i = 0; i  < MAX_COUNT_VALUE; i++)
        ++dummmy_counter;
    
    printf("%sThread with index %d (Thread ID: %lu) ended its routine. Elapsed time: %ld%s\r\n",
            PRINT_COLOR_BLUE                                                ,
            thread_input_data->thread_idx                                   ,
            pthread_self()                                                  ,
            (clock() - start_time)                                          ,
            PRINT_COLOR_RESET                                               );

    return NULL;
}

void threadsWithAttributes()
{
    // Initialize arrays for threads, attributes, scheduling priority parameters and thread input arguments.
    pthread_t threads[NUMBER_OF_THREADS];
    pthread_attr_t thread_attrs[NUMBER_OF_THREADS];
    priority_param sched_priority_param[NUMBER_OF_THREADS];
    THREAD_INPUT_DATA thread_inputs[NUMBER_OF_THREADS];

    // Initialize a variable holding the common scheduling type.
    int scheduling_policy = SCHED_RR;

    // Initialize and set common cancellabillty details as well as common maximum counter value.
    THREAD_INPUT_COMMON_DATA thread_common_arg =
    {
        .max_count_value                        = MAX_COUNT_VALUE           ,
        .cancelability.cancellation_type        = PTHREAD_CANCEL_ENABLE     ,
        .cancelability.cancelability_enabled    = PTHREAD_CANCEL_DEFERRED   ,
    };

    // Initialize thread attributes variable.
    for(int attr_idx = 0; attr_idx < (sizeof(thread_attrs) / sizeof(thread_attrs[0])); attr_idx++)
    {
        if(threadAttributesCreationStatus(pthread_attr_init(&thread_attrs[attr_idx])))
            return;
    
        // Set priority in ascending order, so that lowest index thread has lowest priority.
        sched_priority_param[attr_idx].sched_priority = (attr_idx + 1);

        // Assign features to the variable holding thread attributes. A custom function is being used this time.
        if(setExampleThreadAttributes(&thread_attrs[attr_idx], &scheduling_policy, &sched_priority_param[attr_idx]))
            return;
    
        // Display a list of the set attributes.
        printf("%sAttribute index:\t%d%s\r\n", PRINT_COLOR_CYAN, attr_idx, PRINT_COLOR_RESET);
        showExampleThreadAttributes(&thread_attrs[attr_idx], &thread_common_arg.cancelability);
        printf("\r\n");
    }

    // Once attributes have been already set, launch all threads.
    for(int thread_idx = 0; thread_idx < (sizeof(threads) / sizeof(threads[0])); thread_idx++)
    {
        thread_inputs[thread_idx].input_common = &thread_common_arg;
        thread_inputs[thread_idx].thread_idx = thread_idx;

        if( checkThreadCreationStatus( pthread_create(&threads[thread_idx], &thread_attrs[thread_idx], threadWithAttributesRoutine, &thread_inputs[thread_idx]) ) )
        {
            for(int cancel_idx = (thread_idx - 1); cancel_idx >= 0; cancel_idx--)
                pthread_cancel(threads[cancel_idx]);
            
            return;
        }
    }

    // Once finished, destroy threads as well as their attribute-holding variable.
    for(int attr_idx = 0; attr_idx < (sizeof(thread_attrs) / sizeof(thread_attrs[0])); attr_idx++)
        pthread_attr_destroy(&thread_attrs[attr_idx]);
    
    for(int thread_idx = 0; thread_idx < (sizeof(threads) / sizeof(threads[0])); thread_idx++)
        pthread_join(threads[thread_idx], NULL);
}

/*
It can be boldly stated that the performance of the function written above strongly depends on the maximum number count as well
as the capacities and number of processor in the machine running it. The higher the maximum number count, the clearer the differences
between threads will be noticed.
*/

/**************************************/
