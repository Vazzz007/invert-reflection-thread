#ifndef __FUNC_EVAL_H_INCLUDED__
#define __FUNC_EVAL_H_INCLUDED__

#include <pthread.h>
#include <stdlib.h>
#include <math.h>

void synchronize(int total_threads);
int InvMatrix(int n, double *a, double *x, int my_rank, int total_threads, int *status);

#endif /* __FUNC_EVAL_H_INCLUDED__ */