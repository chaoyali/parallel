//
// Created by chaoya on 4/17/16.
//

#ifndef HELLO_NEON_MATRIXCALC_INTRINSICS_H
#define HELLO_NEON_MATRIXCALC_INTRINSICS_H

void matrix_int_mul_block(int* a, int m_a, int n_a, int* b, int m_b, int n_b, int* result);
void matrix_float_mul_block(float* a, int m_a, int n_a, float* b, int m_b, int n_b, float* result);
void matrix_short_mul(short* a, int m_a, int n_a, short* b, int m_b, int n_b, short* result);
void matrix_transpose_int_neon(int* a, int m_a, int n_a, int* result);
void matrix_transpose_float_neon(float* a, int m_a, int n_a, float* result);

#endif //HELLO_NEON_MATRIXCALC_INTRINSICS_H
