/*
Compile as superuser and use flag -D_XOPEN_SOURCE=700 so that scheduling
priorities can be applied and timed locks can be used. Use the following
commands as a reference (provided repo directory has been entered first):
*/

// mkdir exe
// gcc -g -Wall -lpthread -D_XOPEN_SOURCE=700 src/*.c -o exe/main 
// sudo ./exe/main

/********* Include statements *********/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "BasicThreads.h"
#include "ThreadsWithInputParameters.h"
#include "ThreadsWithMutex.h"
#include "ThreadsWithTryLock.h"
#include "ThreadsWithTimedMutex.h"
#include "ThreadsWithTimedWait.h"
#include "ThreadsCancellation.h"
#include "ThreadsWithBarrier.h"
#include "ThreadsWithConditionVariables.h"
#include "ThreadsWithSemaphores.h"
#include "ThreadsWithAttributes.h"
#include "ThreadsWithLocalStorage.h"
#include "ThreadsDetachment.h"
#include "MatrixMultiplication.h"

/**************************************/

/********** Define statements *********/

#define MSG_TEST_HEADER_CHAR                        '*'
#define MSG_TEST_BASIC_THREADS                      "Testing basic threads."
#define MSG_TEST_THREADS_WITH_INPUT_PARAMETERS      "Testing threads with input parameters."
#define MSG_TEST_THREADS_WITH_MUTEX                 "Testing threads with MUTual EXclusion."
#define MSG_TEST_THREADS_WITH_TRYLOCK               "Testing threads with trylocks."
#define MSG_TEST_THREADS_WITH_TIMED_MUTEX           "Testing threads with timed mutexes."
#define MSG_TEST_THREADS_CANCELLATION               "Testing thread cancellation."
#define MSG_TEST_THREADS_WITH_BARRIER               "Testing threads with barrier."
#define MSG_TEST_THREADS_WITH_CONDITION_VARIABLES   "Testing threads with condition variables."
#define MSG_TEST_THREADS_WITH_TIMED_WAIT            "Testing threads with timed waits."
#define MSG_TEST_THREADS_WITH_SEMAPHORES            "Testing threads with semaphores."
#define MSG_TEST_THREADS_WITH_ATTRIBUTES            "Testing threads with attributes."
#define MSG_TEST_THREADS_WITH_LOCAL_STORAGE         "Testing threads with local storage."
#define MSG_TEST_THREADS_DETACH                     "Testing detached threads."
#define MSG_TEST_EXAMPLE_MATRIX_MULTIPLICATION      "Example: matrix multiplication using multiple threads."
#define MSG_TEST_FOOTER_CHAR                        '-'

#define TIME_BETWEEN_FUNCTION_CALLS                 1

/**************************************/

/******** Function definitions ********/

static void printTestHeader(const char* test_text)
{
    if(!test_text)
        return;
    
    char test_header[strlen(test_text) + 1];
    memset(&test_header, MSG_TEST_HEADER_CHAR, strlen(test_text));
    test_header[strlen(test_text)] = 0;

    char test_footer[strlen(test_text) + 1];
    memset(&test_footer, MSG_TEST_FOOTER_CHAR, strlen(test_text));
    test_footer[strlen(test_text)] = 0;

    printf("%s\r\n%s\r\n%s\r\n", test_header, test_text, test_footer);
}

static void executeTestFunction(const char* test_text, void(*test_function)(void))
{
    printTestHeader(test_text);
    test_function();
    printf("\r\n");
    sleep(TIME_BETWEEN_FUNCTION_CALLS);
}

int main()
{
    // executeTestFunction(MSG_TEST_BASIC_THREADS                      , basicThreadUsingFunction          );
    // executeTestFunction(MSG_TEST_THREADS_WITH_INPUT_PARAMETERS      , functionUsingThreadWithParameters );
    // executeTestFunction(MSG_TEST_THREADS_WITH_MUTEX                 , functionUsingThreadWithoutMutex   );
    // executeTestFunction(MSG_TEST_THREADS_WITH_TRYLOCK               , threadsWithTryLock                );
    // executeTestFunction(MSG_TEST_THREADS_WITH_TIMED_MUTEX           , functionUsingThreadWithTimedMutex );
    // executeTestFunction(MSG_TEST_THREADS_CANCELLATION               , threadsCancellation               );
    // executeTestFunction(MSG_TEST_THREADS_WITH_BARRIER               , threadsWithBarrier                );
    // executeTestFunction(MSG_TEST_THREADS_WITH_CONDITION_VARIABLES   , threadsWithConditionVariables     );
    // executeTestFunction(MSG_TEST_THREADS_WITH_TIMED_WAIT            , functionUsingThreadWithTimedWait  );
    // executeTestFunction(MSG_TEST_THREADS_WITH_SEMAPHORES            , threadsWithSemaphores             );
    // executeTestFunction(MSG_TEST_THREADS_WITH_ATTRIBUTES            , threadsWithAttributes             );
    // executeTestFunction(MSG_TEST_THREADS_WITH_LOCAL_STORAGE         , threadsWithLocalStorage           );
    // executeTestFunction(MSG_TEST_THREADS_DETACH                     , threadsDetachment                 );
    executeTestFunction(MSG_TEST_EXAMPLE_MATRIX_MULTIPLICATION      , exampleMatrixMultiplication       );

    return 0;
}

/**************************************/
