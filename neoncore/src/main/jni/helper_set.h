//
// Created by 朱晓光 on 4/14/16.
//

#ifndef HELLO_NEON_HELPER_SET_H
#define HELLO_NEON_HELPER_SET_H

#include "macro_define.h"

/* complex number and computations - discarded */
/*
typedef struct complex_double {
    double real;
    double imag;
} comp_double;
typedef struct complex_float {
    float real;
    float imag;
} comp_float;
comp_float comp_float_add(comp_float a, comp_float b);
comp_float comp_float_minus(comp_float a, comp_float b);
comp_float comp_float_multi(comp_float a, comp_float b);
*/

void swap(int* a, int* b);
int get_reverse(int i, int len);

void fft_float(float * real, float * imag, int len, int reverse);

void combsort(int * arr, int len);

void vector_int(int* x, int len, int * var, int num_var);
void vector_float(float* x, int len, float * var, int num_var);

#endif //HELLO_NEON_HELPER_SET_H
