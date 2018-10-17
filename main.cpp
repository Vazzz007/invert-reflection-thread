#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fenv.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "args.h"
#include "create_matrix.h"
#include "func_eval.h"

#define EPS 1e-16

/* factorial supports the following command-line arguments:
 * 
 * -i input_file_name.txt - name of the input file (default = NULL)
 * -n number - number of elements (default = 0)
 * -v - option for debugging
 * -f formula - define formula
 */

struct timespec diff(struct timespec start, struct timespec end);

struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

typedef struct
{
    int n;
    double *A;
    double *X;
    int my_rank;
    double residual;
    int total_threads;
} ARGS_mul;

typedef struct
{
    int n;
    double *A;
    double *X;
    int my_rank;
    int total_threads;
    int status;
} ARGS;

struct timespec time_thread_inv, time_thread_resid;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *Multiplication(void *p_arg);

void *Multiplication(void *p_arg)
{
    ARGS_mul *arg = (ARGS_mul*)p_arg;
    struct timespec time_thread_start, time_thread_end;

    multi(arg->n, arg->A, arg->X, arg->my_rank, &arg->residual, arg->total_threads);

    if( clock_gettime( CLOCK_THREAD_CPUTIME_ID, &time_thread_end) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    time_thread_end = diff(time_thread_start, time_thread_end);

    pthread_mutex_lock(&mutex);
    time_thread_resid.tv_sec += time_thread_end.tv_sec;
    time_thread_resid.tv_nsec += time_thread_end.tv_nsec;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *Inversion(void *p_arg);

void *Inversion(void *p_arg)
{
    ARGS *arg = (ARGS*)p_arg;
    struct timespec time_thread_start, time_thread_end;

    if( clock_gettime( CLOCK_THREAD_CPUTIME_ID, &time_thread_start) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    
    InvMatrix(arg->n, arg->A, arg->X, arg->my_rank, arg->total_threads, &arg->status);
    
    if( clock_gettime( CLOCK_THREAD_CPUTIME_ID, &time_thread_end) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    time_thread_end = diff(time_thread_start, time_thread_end);

    pthread_mutex_lock(&mutex);
    time_thread_inv.tv_sec += time_thread_end.tv_sec;
    time_thread_inv.tv_nsec += time_thread_end.tv_nsec;
    printf("\nThread_number = %d. Work done!\nThread_time\t= %f sec.\n\n",
               arg->my_rank,
               (double)time_thread_end.tv_sec + (double)time_thread_end.tv_nsec/(double)1000000000);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv){
    
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
    
    struct timespec time_start, time_end, time_total, time_thread_total;
    
    int n = 0;
    int max_out = 0;
    char *inFileName = NULL;
    int verbose = 0;
    char *formula = NULL;
    double *X = NULL;
    double *A = NULL;
    FILE *fin;
    int result = 1;
    double nev = 0.0;
    int total_threads;
    pthread_t *threads;
    ARGS *args;
    ARGS_mul *args_mul;
    
    
    if ((get_args(&n, &inFileName, &verbose, &formula, &max_out, &total_threads, argc, argv)) != 0){
        fprintf (stderr, "\nError: Can't get arguments!\n");
        return -1;
    }
    
    if (inFileName == NULL){
        A = (double *)malloc (n * n * sizeof(double));
        X = (double *)malloc (n * n * sizeof(double));
        threads = (pthread_t*)malloc(total_threads * sizeof(pthread_t));
        args = (ARGS*)malloc(total_threads * sizeof(ARGS));
        
        if ((A && X && threads && args) != 1) {
            printf ("\nError: Not enough memory!\n");
            if (A == NULL)
                free(A);
            if (X == NULL)
                free(X);
            if (threads == NULL)
                free(threads);
            if (args == NULL)
                free(args);
            return -1;
        }
        
        if ((create_matrix(A, n, formula)) != 0){
            printf ("\nError: Can't create matrix!\n");
            free(A);
            free(X);
            free(threads);
            free(args);
            return -1;
        }
    } else {
        fin = fopen(inFileName, "r");
        
        if (fin == NULL){
            printf("\nError: Can't open file!\n");
            return -1;
        }
        
        if (fscanf(fin, "%d", &n) != 1){
            printf("\nError: Can't read dimension from file!\n");
            fclose(fin);
            return -1;
        }
        
        if (n < 1){
            printf ("\nError: Invalid matrix dimension!\n");
            fclose(fin);
            return -1;
        }
        
        A = (double *)malloc (n * n * sizeof(double));
        X = (double *)malloc (n * n * sizeof(double));
        threads = (pthread_t*)malloc(total_threads * sizeof(pthread_t));
        args = (ARGS*)malloc(total_threads * sizeof(ARGS));
        
        if ((A && X && threads && args) != 1) {
            printf ("\nError: Not enough memory!\n");
            if (A == NULL)
                free(A);
            if (X == NULL)
                free(X);
            if (threads == NULL)
                free(threads);
            if (args == NULL)
                free(args);
            return -1;
        }
        
        if (InputMatrix(n, A, fin) != 0){
            printf("\nError: Can't read matrix from file!\n");
            fclose(fin);
            free(A);
            free(X);
            free(threads);
            free(args);
            return -1;
        }
    }
    
    printf("\nMatrix A:\n");
    
    PrintMatrix(n, A, max_out);
    
    printf("\n");
    
    printf("\nCalculating...\n");
    
    for (int i = 0; i < total_threads; i++){
        args[i].n = n;
        args[i].A = A;
        args[i].X = X;
        args[i].my_rank = i;
        args[i].total_threads = total_threads;
    }
    
    if( clock_gettime( CLOCK_MONOTONIC, &time_start) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    
    for (int i = 0; i < total_threads; i++){
        if (pthread_create(threads + i, NULL, Inversion, args + i)){
            printf("Cannot create thread %d!\n", i);

            if (A) free(A);
            if (X) free(X);
            if (threads) free(threads);
            if (args) free(args);

            return -1;
        }
        //printf("status = %d, i = %d", args->status, i);
        if (args->status == -1){
            printf("Error: Matrix is uninvertible!\n");
            if (A) free(A);
            if (X) free(X);
            if (threads) free(threads);
            if (args) free(args);
            return -1;
        }
    }
        
    for (int i = 0; i < total_threads; i++){
        if (pthread_join(threads[i], NULL)){
            printf("Cannot wait thread %d!\n", i);

            if (A) free(A);
            if (X) free(X);
            if (threads) free(threads);
            if (args) free(args);

            return -1;
        }
        if (args->status == -1){
            printf("Error: Matrix is uninvertible!\n");
            if (A) free(A);
            if (X) free(X);
            if (threads) free(threads);
            if (args) free(args);
            return -1;
        }
    }
    
    if( clock_gettime( CLOCK_MONOTONIC, &time_end) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    time_end = diff(time_start, time_end);
    time_total = time_end;
    
    if (result == -1){
        printf("\nCan't invert.\n");
    } else {
        printf("\nMatrix A^{-1}:\n");
        PrintMatrix(n, X, max_out);
        printf("\n");
        
        printf("\nInversion time \t\t= %f sec.\nInversion_thread_time\t= %f sec.\n\n",
               (double)time_total.tv_sec + (double)time_total.tv_nsec/(double)1000000000,
               (double)time_thread_inv.tv_sec + (double)time_thread_inv.tv_nsec/(double)1000000000);
        
        
        if (inFileName == NULL){
            if ((create_matrix(A, n, formula)) != 0){
                printf ("\nError: Can't create matrix!\n");
                free(A);
                free(X);
                free(threads);
                free(args);
                return -1;
            }
        } else {
            fin = fopen(inFileName, "r");

            if (fin == NULL){
                printf("\nError: Can't open file!\n");
                free(A);
                free(X);
                free(threads);
                free(args);
                return -1;
            }

            if (fscanf(fin, "%d", &n) != 1){
                printf("\nError: Can't read dimension from file!\n");
                free(A);
                free(X);
                free(threads);
                free(args);
                fclose(fin);
                return -1;
            }

            if (n < 1){
                printf ("\nError: Invalid matrix dimension!\n");
                free(A);
                free(X);
                free(threads);
                free(args);
                fclose(fin);
                return -1;
            }
        
            if (InputMatrix(n, A, fin) != 0){
                printf("\nError: Can't read matrix from file!\n");
                fclose(fin);
                free(A);
                free(X);
                free(threads);
                free(args);
                return -1;
            }
            
            fclose(fin);
        }

        args_mul = (ARGS_mul*)malloc(total_threads * sizeof(ARGS_mul));

        if( clock_gettime( CLOCK_MONOTONIC, &time_start) == -1 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }
        
        for (int i = 0; i < total_threads; i++){
            args_mul[i].n = n;
            args_mul[i].A = A;
            args_mul[i].X = X;
            args_mul[i].my_rank = i;
            args_mul[i].residual = 0.0;
            args_mul[i].total_threads = total_threads;
        }

        for (int i = 0; i < total_threads; i++){
            if (pthread_create(threads + i, NULL, Multiplication, args_mul + i)){
                printf("Cannot create thread %d!\n", i);

                if (A) free(A);
                if (X) free(X);
                if (threads) free(threads);
                if (args) free(args);
                if (args_mul) free(args_mul);

                return -1;
            }
        }

        for (int i = 0; i < total_threads; i++){
            if (pthread_join(threads[i], NULL)){
                printf("Cannot wait thread %d!\n", i);

                if (A) free(A);
                if (X) free(X);
                if (threads) free(threads);
                if (args) free(args);
                if (args_mul) free(args_mul);

                return -1;
            }
        }

        if( clock_gettime( CLOCK_MONOTONIC, &time_end) == -1 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }
        time_end = diff(time_start, time_end);
        printf("\nResidual time \t\t= %f sec.\nResidual_thread_time\t= %f sec.\n\n",
               (double)time_end.tv_sec + (double)time_end.tv_nsec/(double)1000000000,
               (double)time_thread_resid.tv_sec + (double)time_thread_resid.tv_nsec/(double)1000000000);
        time_total.tv_sec += time_end.tv_sec;
        time_total.tv_nsec += time_end.tv_nsec;

        time_thread_total.tv_sec = time_thread_inv.tv_sec + time_thread_resid.tv_sec;
        time_thread_total.tv_nsec = time_thread_inv.tv_nsec + time_thread_resid.tv_nsec;


        for (int i = 0; i < total_threads; i++){
            nev += (1e9) * args_mul[i].residual;
        }

        printf("\nTotal time \t\t= %f sec.\nTotal_thread_time\t= %f sec.\n\n",
               (double)time_total.tv_sec + (double)time_total.tv_nsec/(double)1000000000,
               (double)time_thread_total.tv_sec + (double)time_thread_total.tv_nsec/(double)1000000000);

        printf("\nSolution threaded ||A * A^{-1} - I||\t= %e\n", nev);

        free(args_mul);

        //printf("\nSolution ||A * A^{-1} - I||\t= %e\n", SolutionError(n, A, X));
    }

    free(threads);
    free(args);
    free(A);
    free(X);
    
    return 0;
}
