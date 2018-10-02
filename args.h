#ifndef __ARGS_H_INCLUDED__
#define __ARGS_H_INCLUDED__
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fenv.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

int get_args(int *matrixSize, char **inFileName, int *verbose, char **formula, int *max_out, 
             int *total_threads, int argc, char **argv);
void help();

#endif /* __ARGS_H_INCLUDED__ */