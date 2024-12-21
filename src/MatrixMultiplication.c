/*

*/

/********* Include statements *********/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "ThreadColors.h"
#include "ThreadCreationStatus.h"
#include "MatrixMultiplication.h"

/**************************************/

/********** Define statements *********/

#define MIN_MAT_DIM     1
#define MAX_MAT_DIM     5
#define MIN_MAT_VAL     0
#define MAX_MAT_VAL     10
#define MAT_NAME_HEADER "Matrix "
#define MAT_HEADER_SEP  '.'

/**************************************/

/****** Private type definitions ******/

typedef struct
{
    int** mat_A;
    int** mat_B;
    int** mat_C;

    unsigned int mat_A_cols;

    pthread_mutex_t* p_mutex_C;

} MATRIX_MULT_COMMON_DATA;

typedef struct
{
    unsigned int target_row_A;
    unsigned int target_col_B;

    MATRIX_MULT_COMMON_DATA* p_matrix_mult_common_data; 

} MATRIX_MULT_DATA;

/**************************************/

/**** Private function prototypes *****/

static int**    allocateMatrix(unsigned int rows, unsigned int cols);
static int      getDelimitedRandomInteger(int min_val, int max_val);
static int**    populateRandomValuesMatrix(int** mat, unsigned int mat_rows, unsigned int mat_cols, int min_val, int max_val);
static int**    createRandomValuesMatrix(unsigned int rows, unsigned int cols, int min_val, int max_val);
static void     deallocateMatrix(int** mat, unsigned int rows);
static void     printMatrix(int** mat, unsigned int rows, unsigned int cols, char* matrix_name, char* color);

/**************************************/

/******** Function definitions ********/

static int** allocateMatrix(unsigned int rows, unsigned int cols)
{
    int** mat = (int**)malloc(rows * sizeof(int*));

    if(mat == NULL)
        return NULL;

    for(unsigned int row_idx = 0; row_idx < rows; row_idx++)
    {
        mat[row_idx] = (int*)malloc(cols * sizeof(int));

        if(mat[row_idx] == NULL)
            return NULL;
    }
    
    return mat;
}

// Make sure srand(time(NULL)) has been called before using the funcion below.
static int getDelimitedRandomInteger(int min_val, int max_val)
{
    return (rand() % (max_val - min_val + 1) + min_val);
}

static int** populateRandomValuesMatrix(int** mat, unsigned int mat_rows, unsigned int mat_cols, int min_val, int max_val)
{
    if(mat == NULL)
        return NULL;

    for(unsigned int row_idx = 0; row_idx < mat_rows; row_idx++)
    {
        if(mat[row_idx] == NULL)
            return NULL;

        for(unsigned int col_idx = 0; col_idx < mat_cols; col_idx++)
            mat[row_idx][col_idx] = getDelimitedRandomInteger(min_val, max_val);
    }

    return mat;
}

static int** createRandomValuesMatrix(unsigned int rows, unsigned int cols, int min_val, int max_val)
{
    int** mat;

    mat = allocateMatrix(rows, cols);
    
    if(mat == NULL)
    {
        printf("%sCould not allocate matrix memory!%s\r\n", PRINT_COLOR_RED, PRINT_COLOR_RESET);
        return NULL;
    }
    
    mat = populateRandomValuesMatrix(mat, rows, cols, min_val, max_val);

    if(mat == NULL)
    {
        printf("%sCould not populate matrix (address: %p)!%s\r\n", PRINT_COLOR_RED, mat, PRINT_COLOR_RESET);
        return NULL;
    }

    return mat;
}

static void deallocateMatrix(int** mat, unsigned int rows)
{
    for(unsigned int row_idx = 0; row_idx < rows; row_idx++)
        free(mat[row_idx]);
    
    free(mat);
}

static int multiplyRowByColumn(int** A, int** B, unsigned int A_cols, unsigned int row_A, unsigned int col_B)
{
    int ret = 0;

    for(unsigned int i = 0; i < A_cols; i++)
        ret += (A[row_A][i] * B[i][col_B]);

    return ret;
}

static void* matrixMultThreadRoutine(void* arg)
{
    // Make the thread cancellable (deferred).
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    MATRIX_MULT_DATA* p_matrix_mult_data = (MATRIX_MULT_DATA*)arg;

    int calculated_value = multiplyRowByColumn( p_matrix_mult_data->p_matrix_mult_common_data->mat_A        ,
                                                p_matrix_mult_data->p_matrix_mult_common_data->mat_B        ,
                                                p_matrix_mult_data->p_matrix_mult_common_data->mat_A_cols   ,
                                                p_matrix_mult_data->target_row_A                            ,
                                                p_matrix_mult_data->target_col_B                            );
    
    // Write the calculated value onto the resulting matrix.
    pthread_mutex_lock(p_matrix_mult_data->p_matrix_mult_common_data->p_mutex_C);

    p_matrix_mult_data->p_matrix_mult_common_data->mat_C[p_matrix_mult_data->target_row_A][p_matrix_mult_data->target_col_B] = calculated_value;

    pthread_mutex_unlock(p_matrix_mult_data->p_matrix_mult_common_data->p_mutex_C);

    return NULL;
}

static void printMatrix(int** mat, unsigned int rows, unsigned int cols, char* matrix_name, char* color)
{
    printf("%s%s%s%s\r\n", color, MAT_NAME_HEADER, matrix_name, PRINT_COLOR_RESET);
    
    unsigned int full_header_len = strlen(MAT_NAME_HEADER) + strlen(matrix_name);
    char mat_header[full_header_len + 1];
    memset(&mat_header, MAT_HEADER_SEP, full_header_len);
    mat_header[full_header_len] = 0;

    printf("%s%s%s\r\n", color, mat_header, PRINT_COLOR_RESET);

    for(unsigned int row = 0; row < rows; row++)
    {
        printf("%s[\t%s", color, PRINT_COLOR_RESET);

        for(unsigned int col = 0; col < cols; col++)
            printf("%s%d%s\t", color, mat[row][col], PRINT_COLOR_RESET);

        printf("%s]%s\r\n", color, PRINT_COLOR_RESET);
    }

    printf("\r\n");
}

void exampleMatrixMultiplication()
{
    // Initilize time seed for random values to be properly generated.
    srand(time(NULL));

    // Allocate memory for matrices.
    unsigned int mat_A_rows = getDelimitedRandomInteger(MIN_MAT_DIM, MAX_MAT_DIM);
    unsigned int mat_A_cols = getDelimitedRandomInteger(MIN_MAT_DIM, MAX_MAT_DIM);
    
    unsigned int mat_B_rows = mat_A_cols;
    unsigned int mat_B_cols = getDelimitedRandomInteger(MIN_MAT_DIM, MAX_MAT_DIM);
    
    unsigned int mat_C_rows = mat_A_rows;
    unsigned int mat_C_cols = mat_B_cols;

    int** mat_A = createRandomValuesMatrix(mat_A_rows, mat_A_cols, MIN_MAT_VAL, MAX_MAT_VAL);
    int** mat_B = createRandomValuesMatrix(mat_B_rows, mat_B_cols, MIN_MAT_VAL, MAX_MAT_VAL);
    int** mat_C = allocateMatrix(mat_C_rows, mat_C_cols);

    // Create threads, one for each element in the resulting matrix.
    unsigned int threads_num = mat_C_rows * mat_C_cols;
    pthread_t* threads = (pthread_t*)malloc(threads_num * sizeof(pthread_t*));

    // Create a mutex, so that only a single thread writes on the resulting matrix each time.
    pthread_mutex_t mat_C_lock;

    // For each thread, allocate its data structure.
    MATRIX_MULT_DATA* matrix_mult_data_arr = (MATRIX_MULT_DATA*)malloc(threads_num * sizeof(MATRIX_MULT_DATA));

    if(matrix_mult_data_arr == NULL)
    {
        printf("%sCould not allocate matrix data array!%s\r\n", PRINT_COLOR_RED, PRINT_COLOR_RESET);
        return;
    }

    // Allocate common multiplication data.
    MATRIX_MULT_COMMON_DATA* matrix_mult_common_data = (MATRIX_MULT_COMMON_DATA*)malloc(sizeof(MATRIX_MULT_COMMON_DATA)); 
    
    if(matrix_mult_common_data == NULL)
    {
        printf("%sCould not allocate matrix common data!%s\r\n", PRINT_COLOR_RED, PRINT_COLOR_RESET);
        return;
    }

    matrix_mult_common_data->mat_A      = mat_A         ;
    matrix_mult_common_data->mat_B      = mat_B         ;
    matrix_mult_common_data->mat_C      = mat_C         ;
    matrix_mult_common_data->mat_A_cols = mat_A_cols    ;
    matrix_mult_common_data->p_mutex_C  = &mat_C_lock   ;

    // Initialize C matrix mutex lock.
    pthread_mutex_init(&mat_C_lock, NULL);

    // Prepare data given as each thread's input parameter and launch them.
    for(unsigned int thread_idx = 0; thread_idx < threads_num; thread_idx++)
    {
        matrix_mult_data_arr[thread_idx].p_matrix_mult_common_data = matrix_mult_common_data;
        matrix_mult_data_arr[thread_idx].target_row_A = (thread_idx / mat_C_cols);
        matrix_mult_data_arr[thread_idx].target_col_B = (thread_idx % mat_C_cols);

        if(checkThreadCreationStatus( pthread_create(&threads[thread_idx], NULL, matrixMultThreadRoutine, &matrix_mult_data_arr[thread_idx]) ))
        {
            printf("%sCould not create thread %d (element at C[%d][%d]). Aborting matrix multiplication.%s\r\n",
                    PRINT_COLOR_RED,
                    thread_idx,
                    (thread_idx / mat_C_cols),
                    (thread_idx % mat_C_cols),
                    PRINT_COLOR_RESET);
            
            for(unsigned int cancel_idx = (thread_idx - 1); cancel_idx >= 0; cancel_idx--)
                pthread_cancel(threads[cancel_idx]);

            return;
        }
    }

    // Wait for every thread to join the main one.
    for(unsigned int thread_idx = 0; thread_idx < threads_num; thread_idx++)
        pthread_join(threads[thread_idx], NULL);
    
    // Print matrices.
    printMatrix(mat_A, mat_A_rows, mat_A_cols, "A", PRINT_COLOR_CYAN     );
    printMatrix(mat_B, mat_B_rows, mat_B_cols, "B", PRINT_COLOR_PURPLE   );
    printMatrix(mat_C, mat_C_rows, mat_C_cols, "C (A x B = C)", PRINT_COLOR_GREEN    );

    // Free memory used to store pthread_t and MATRIX_MULT_DATA type variables.
    free(threads);
    free(matrix_mult_common_data);
    free(matrix_mult_data_arr);

    // Destroy mutex lock.
    pthread_mutex_destroy(&mat_C_lock);

    // Free memory previously allocated for each matrix.
    deallocateMatrix(mat_A, mat_A_rows);
    deallocateMatrix(mat_B, mat_B_rows);
    deallocateMatrix(mat_C, mat_C_rows);
}

/**************************************/
