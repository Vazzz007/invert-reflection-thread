#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "create_matrix.h"
#include "func_eval.h"

static int max(int i, int j){
    if (i >= j) {
	return i;
    }
    return j;
}

static double sym_f(int i, int j){
    return (double)fabs(i - j);
}

static double symnul_f(int i, int j){
    return (double)fabs(i - j) + 1.0;
}

static double gilb_f(int i, int j){
    return 1.0 / (i + j + 1.0);
}

static double up_tr(int i, int j){
    if (i == j){
	return (double)1;
    }
    if (i < j){
	return (double)-1;
    }
    return (double)0;
}

static double uniform(int i, int j, int n){
    return (double)(n - max(i, j));
}

int create_matrix(double *A, int n, char *formula){
    
    int i, j;     
        
    if (strcmp(formula, "sym") == 0){
        for (i = 0; i < n; ++i)
            for (j = 0; j < n; ++j)
                A[i * n + j] = sym_f(i, j);
    } else if (strcmp(formula, "symnul") == 0){
        for (i = 0; i < n; ++i)
            for (j = 0; j < n; ++j)
                A[i * n + j] = symnul_f(i, j);
    } else if (strcmp(formula, "gilb") == 0){
        for (i = 0; i < n; ++i)
            for (j = 0; j < n; ++j)
                A[i * n + j] = gilb_f(i, j);
    } else if (strcmp(formula, "1") == 0){
        for (i = 0; i < n; ++i)
            for (j = 0; j < n; ++j)
                A[i * n + j] = up_tr(i, j);
    } else if (strcmp(formula, "9") == 0){
        for (i = 0; i < n; ++i)
            for (j = 0; j < n; ++j)
                A[i * n + j] = uniform(i, j, n);
    } else {
        printf ("Error: Invalid formula!\n");
        help();
        free(A);
        return -1;
    }
    
    return 0;
}

void PrintMatrix(int n, double *matr, int max_out){
    
    int i;
    int j;
    int nPrint;
    
    nPrint = (n > max_out) ? max_out : n;

    for (i = 0; i < nPrint; ++i){
        printf("| ");
        for (j = 0; j < nPrint; ++j)
            printf("%10.3e ", matr[i * n + j]);
        printf("|\n");
    }
}

int InputMatrix(int n, double *A, FILE *fin){
    int i;
    int j;
    
    for (i = 0; i < n; ++i)
        for (j = 0; j < n; ++j)
            if (fscanf(fin, "%lf", &A[i * n + j]) != 1)
                return -1;
    
    return 0;
}

double SolutionError(int n, double* a, double* x){
    int i;
    int j;
    int k;
    double tmp;
    double rezult;

    rezult = 0.0;
    for (i = 0; i < n; ++i){
        for (j = 0; j < n; ++j){
            tmp = 0.0;
            for (k = 0; k < n; ++k)
                tmp += a[i * n + k] * x[k * n + j];

            if (i == j)
                tmp -= 1.0;

            rezult += tmp * tmp;
        }
    }

    return sqrt(rezult);
}

int multi(int n, double* a, double* x, int my_rank, double *residual, int total_threads, double tmp)
{
    //*residual = 0.0;

    for (int i = my_rank * n / total_threads; i < (my_rank + 1) * n / total_threads; ++i){
        for (int j = 0; j < n; ++j){
            tmp = 0.0;
            for (int k = 0; k < n; ++k)
                tmp += a[i * n + k] * x[k * n + j];

            if (i == j)
                tmp -= 1.0;

            *residual += tmp * tmp;

        }
    //printf("\nmy_rank = %d, i = %d, residual = %10.3e\n", my_rank, i, *residual);
    }
    *residual = sqrt(*residual);
    
    return 0;
}
