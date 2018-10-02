#ifndef __CREATE_MATRIX_H_INCLUDED__
#define __CREATE_MATRIX_H_INCLUDED__

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fenv.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "args.h"

int create_matrix(double *A, int n, char *formula);
void PrintMatrix(int n, double *matr, int max_out);
int InputMatrix(int n, double *A, FILE *fin);
double SolutionError(int n, double* a, double* x);

#endif /* __CREATE_MATRIX_H_INCLUDED__ */
