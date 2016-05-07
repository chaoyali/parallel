//
// Created by 朱晓光 on 4/14/16.
//

#ifndef HELLO_NEON_NEON_COLLECTION_INTRINSICS_H
#define HELLO_NEON_NEON_COLLECTION_INTRINSICS_H

#include "helper_set.h"

void combsort_intrinsics_int(int * lst, int len);
void fft_intrinsics_float(float * real, float * imag, int len, int reverse);
void vector_intrinsics_int(int* x, int len, int * var, int num_var);

#endif //HELLO_NEON_NEON_COLLECTION_INTRINSICS_H
