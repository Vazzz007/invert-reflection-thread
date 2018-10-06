#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "func_eval.h"

void synchronize(int total_threads)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t condvar_in = PTHREAD_COND_INITIALIZER;
    static pthread_cond_t condvar_out = PTHREAD_COND_INITIALIZER;
    static int threads_in = 0;
    static int threads_out = 0;

    pthread_mutex_lock(&mutex);

    threads_in++;
    if (threads_in >= total_threads){
		threads_out = 0;
		pthread_cond_broadcast(&condvar_in);
    } else
		while (threads_in < total_threads)
			pthread_cond_wait(&condvar_in,&mutex);

    threads_out++;
	if (threads_out >= total_threads){
		threads_in = 0;
		pthread_cond_broadcast(&condvar_out);
	} else
		while (threads_out < total_threads)
			pthread_cond_wait(&condvar_out,&mutex);

	pthread_mutex_unlock(&mutex);
}

int InvMatrix(int n, double *a, double *x, int my_rank, int total_threads, int *status)
{
	int i, j, k;
	int first_row;
	int last_row;
	double tmp1, tmp2;

	if (my_rank == 0)
		for (i = 0; i < n; i++)
			for (j = 0; j < n; j++)
				x[i * n + j] = (double)(i == j);

	for (i = 0; i < n - 1; i++){
		if (my_rank == 0){
			tmp1 = 0.0;
			for (j = i + 1; j < n; j++)
				tmp1 += a[j * n + i] * a[j * n + i];

			tmp2 = sqrt(tmp1 + a[i * n + i] * a[i * n + i]);
			
			/*if (tmp2 < 1e-100){
				*status = -1;
                printf ("status in func = %d", *status);
				pthread_exit(NULL);
                return -1;
			}*/

			a[i * n + i] -= tmp2;

			tmp1 = sqrt(tmp1 + a[i * n + i] * a[i * n + i]);

			tmp1 = 1.0/tmp1;
			for (j = i; j < n; j++)
				a[j * n + i] *= tmp1;

		}
		synchronize(total_threads);

		first_row = (n - i - 1) * my_rank;
		first_row = first_row/total_threads + i + 1;
		last_row = (n - i - 1) * (my_rank + 1);
		last_row = last_row/total_threads + i + 1;

		for (k = first_row; k < last_row; k++)
		{
			tmp1 = 0.0;
			for (j = i; j < n; j++)
				tmp1 += a[j * n + i] * a[j * n + k];

			tmp1 *= 2.0;
			for (j = i; j < n; j++)
				a[j * n + k] -= tmp1 * a[j * n + i];
		}
		synchronize(total_threads);

		first_row = n * my_rank;
		first_row = first_row/total_threads;
		last_row = n * (my_rank + 1);
		last_row = last_row/total_threads;

		for (k = first_row; k < last_row; k++)
		{
			tmp1 = 0.0;
			for (j = i; j < n; j++)
				tmp1 += a[j * n + i] * x[j * n + k];

			tmp1 *= 2.0;
			for (j = i; j < n; j++)
				x[j * n + k] -= tmp1 * a[j * n + i];

		}
		synchronize(total_threads);

		if (my_rank == 0)
			a[i * n + i] = tmp2;
	}

	first_row = n * my_rank;
	first_row = first_row/total_threads;
	last_row = n * (my_rank + 1);
	last_row = last_row/total_threads;

	for (k = first_row; k < last_row; k++)
		for (i = n - 1; i >= 0; i--)
		{
			tmp1 = x[i * n + k];
			for (j = i + 1; j < n; j++)
				tmp1 -= a[i * n + j] * x[j * n + k];
            /*if (a[i * n + i] < 1e-100){
				*status = -1;
                printf ("status in func = %d", *status);
				pthread_exit(NULL);
                return -1;
			}*/
			x[i * n + k] = tmp1/a[i * n + i];
		}

	return 0;
}