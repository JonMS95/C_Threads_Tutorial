/*
Thread local storage (TLS) is a mechanism that allows threads to maintain their own private instance of data, independent from
another threads. Unlike global or static threads, these will be accessible just from the thread itself. While threads have a stack
memory space created for each of them, heap memory is shared, so if heap memory is to be allocated from a thread, it will be
reachable by any other thread unless a TLS key is used. This way, a heap memory variable can be associated solely to a single thread,
preventing others from reading / writing those variables.

To do so, there is a special data type called thread key. These keys act as identifier which associate data with a thread. Keys are
created as follows:

pthread_key_create(pthread_key_t *key, void (*destructor)(void *))

Where:
·key: pointer to store the created key.
·destructor: a function that will be called once the thread exits so that associated data is cleaned afterwards.
Returns 0 on success, or an error code otherwise.

So as to associate a TLS key with its very own thread, use:

pthread_setspecific(pthread_key_t key, const void* value)

Same can be done to retrieve the associated value by providing a TLS key:

pthread_getspecific(pthread_key_t key)

All of the concepts above will be explained thoroughly with a couple of example threads below. One of them will calculate
fibonacci numbers while the other will retrieve prime numbers. The number quantity will be the same for both cases, but
arrays in which numbers will be stored will be different. Thus, two arrays are going to be created, each being reachable just
from a single thread. Although they could be created by simply using stack memory, heap memory will be used this time for
educational purposes.
*/

/********* Include statements *********/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "ThreadsWithLocalStorage.h"

/**************************************/

/********** Define statements *********/

#define NUMBERS_QUANTITY    10

/**************************************/

/****** Private type definitions ******/

typedef struct
{
    pthread_key_t   tls_key;
    unsigned int    num_q;
} NUMBERS_ROUTINE_ARGUMENT;

/**************************************/

/**** Private function prototypes *****/

static void TLSDatacleanup(void* ptr);
static void printArray(unsigned int* target_array, unsigned int target_array_size);
static void* fibonacciNumbersRoutine(void* arg);
static bool isPrime(unsigned int num);
static void* primeNumbersRoutine(void* arg);

/**************************************/

/******** Function definitions ********/

static void TLSDatacleanup(void* ptr)
{
    printf("%sCleaning data in address: %p.%s\r\n", PRINT_COLOR_PURPLE, ptr, PRINT_COLOR_RESET);
    free(ptr);
}

static void printArray(unsigned int* target_array, unsigned int target_array_size)
{
    for(unsigned int i = 0; i < target_array_size; i++)
        printf("%s%d ", PRINT_COLOR_CYAN, target_array[i]);
    printf("%s\r\n", PRINT_COLOR_RESET);
}

static void* fibonacciNumbersRoutine(void* arg)
{
    NUMBERS_ROUTINE_ARGUMENT* fib_args = (NUMBERS_ROUTINE_ARGUMENT*)arg;
    unsigned int target_size = fib_args->num_q;

    if(target_size <= 0)
        return NULL;

    unsigned int* fib_numbers = (unsigned int*)calloc(target_size, sizeof(unsigned int));

    // Just after heap memory has been allocated within the thread execution routine, bind it to its key.
    pthread_setspecific(fib_args->tls_key, fib_numbers);

    switch (target_size)
    {
        case 1:
        {
            fib_numbers[0] = 0;
        }
        break;

        case 2:
        default:
        {
            fib_numbers[0] = 0;
            fib_numbers[1] = 1;
        }
        break;
    }

    for(unsigned int i = 2; i < target_size; i++)
        fib_numbers[i] = fib_numbers[i - 1] + fib_numbers[i - 2];

    printArray(fib_numbers, target_size);

    // the function call below ensures that the current routine is delayed. This way, the other thread will
    // deallocate its key-associated memory, showing different heap memory addresses when freed.
    sleep(1);

    return NULL;
}

static bool isPrime(unsigned int num)
{
    for(unsigned int i = 2; i < num; i++)
        if(num % i == 0)
            return false;
    
    return true;
}

static void* primeNumbersRoutine(void* arg)
{
    NUMBERS_ROUTINE_ARGUMENT* pri_args = (NUMBERS_ROUTINE_ARGUMENT*)arg;
    unsigned int target_size = pri_args->num_q;

    if(target_size <= 0)
        return NULL;
    
    unsigned int* prime_numbers = (unsigned int*)calloc(target_size, sizeof(unsigned int));

    // Just after heap memory has been allocated within the thread execution routine, bind it to its key.
    pthread_setspecific(pri_args->tls_key, prime_numbers);

    unsigned int calculated_numbers = 0;
    unsigned int current_prime_test = 2;

    while(calculated_numbers < target_size)
    {
        if(isPrime(current_prime_test))
            prime_numbers[calculated_numbers++] = current_prime_test;
        ++current_prime_test;
    }

    printArray(prime_numbers, target_size);

    return NULL;
}

void threadsWithLocalStorage()
{
    // Create a variable for each thread.
    pthread_t t_fibonacci, t_prime;

    // Instantiate a NUMBERS_ROUTINE_ARGUMENT variable.
    NUMBERS_ROUTINE_ARGUMENT numbers_arg = { .num_q = NUMBERS_QUANTITY };
    
    // Create a thread local storage key (found within numbers_arg variable). Keep in mind that a cleanup function should
    // be provided as well so that thread-local data is cleansed once the thread exits.
    pthread_key_create(&numbers_arg.tls_key, TLSDatacleanup);

    // Run each thread. There's no need to provide any data but the quantity of numbers to calculate for each thread and
    // the common TLS key.
    if(checkThreadCreationStatus(pthread_create(&t_fibonacci, NULL, fibonacciNumbersRoutine, &numbers_arg)))
        return;
    
    if(checkThreadCreationStatus(pthread_create(&t_prime, NULL, primeNumbersRoutine, &numbers_arg)))
    {
        pthread_join(t_fibonacci, NULL);
        return;
    }

    // Wait for eevery thread to join main thread back.
    pthread_join(t_fibonacci, NULL);
    pthread_join(t_prime, NULL);

    // After both threads have ended their respective routines, TLS key can be erased.
    pthread_key_delete(numbers_arg.tls_key);
}

/**************************************/
