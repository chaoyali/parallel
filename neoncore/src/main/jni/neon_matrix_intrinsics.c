//
// Created by chaoya on 4/17/16.
//

#include "neon_matrix_intrinsics.h"
#include <arm_neon.h>
#include <android/log.h>

#define LANES_INT_NUM 4
#define LANES_SHORT_NUM 8
#define LANES_FLOAT_NUM 4
#define LANES_LONG_NUM 2

#define BLOCK_M 32
#define BLOCK_N 32
#define BLOCK_K 32

#define BLOCK_TRANSPOSE 8

/*
void matrix_int_mul(int* a, int m_a, int n_a, int* b, int m_b, int n_b, int* result) {
    int i, j ,k, l;
    int mod4 = n_a%LANES_INT_NUM;
    for (i = 0; i < m_a; i++) {
        for (j = 0; j < n_b; j++) {
            int32x4_t sum_vect = vdupq_n_s32(0);
            int sum = 0;
            for (k = 0; k < n_a/LANES_INT_NUM; k++) {
                int32x4_t a_vec = vld1q_s32(a + i * n_a + k * LANES_INT_NUM);
                int32x4_t b_vec;

                b_vec = vld1q_lane_s32(b + k * LANES_INT_NUM * n_b + j, b_vec, 0);
                b_vec = vld1q_lane_s32(b + k * LANES_INT_NUM * n_b + j + n_b, b_vec, 1);
                b_vec = vld1q_lane_s32(b + k * LANES_INT_NUM * n_b + j + n_b * 2, b_vec, 2);
                b_vec = vld1q_lane_s32(b + k * LANES_INT_NUM * n_b + j + n_b * 3, b_vec, 3);
                roid_log_print(ANDROID_LOG_VERBOSE, "paralleM", "%d, %d, %d, %d, %d, %d", i, j, vgetq_lane_s32(tmp_mul, 0), vgetq_lane_s32(tmp_mul, 1), vgetq_lane_s32(tmp_mul, 2), vgetq_lane_s32(tmp_mul, 3));
                sum_vect = vmlaq_s32(sum_vect, a_vec, b_vec);
            }
            sum = vgetq_lane_s32(sum_vect, 0) + vgetq_lane_s32(sum_vect, 1) + vgetq_lane_s32(sum_vect, 2) + vgetq_lane_s32(sum_vect, 3);

            // deal with left over
            if (k == n_a/LANES_INT_NUM && mod4 != 0) {

                for (k = n_a - mod4; k < n_a; k++) {
                    sum += a[i * n_a + k] * b[k * n_b + j];
                }
            }
            result[i * n_b + j] = sum;
        }
    }
}
*/

void matrix_int_mul_block(int* a, int m_a, int n_a, int* b, int m_b, int n_b, int* result) {

    int sum, i, j ,k, iblock, jblock, kblock;

    for (iblock = 0; iblock < m_a; iblock += BLOCK_M)
        for (jblock = 0; jblock < n_b; jblock += BLOCK_N)
            for (kblock = 0; kblock < n_a; kblock += BLOCK_K) {

                // left over
                // for those that i have left over, while j doesn't have.
                if (iblock + BLOCK_M > m_a && jblock + BLOCK_N <= n_b) {

                    if (kblock + BLOCK_K > n_a) {
                        for (i = 0; i < m_a%BLOCK_M; i++)
                            for (j = 0; j < BLOCK_N; j++) {
                                sum = 0;
                                for (k = 0; k < n_a%BLOCK_K; k++) {
                                    sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + (j + jblock)];
                                }
                                result[(i + iblock) * n_b + (j + jblock)] += sum;
                                //__android_log_print(ANDROID_LOG_VERBOSE, "paralleA", "%d, %d, %d, %d", kblock, i + iblock, j + jblock, k + kblock);
                            }
                        continue;
                    }
                    //__android_log_print(ANDROID_LOG_VERBOSE, "doti", "%d", 1);

                    for (i = 0; i < m_a%BLOCK_M; i++) {
                        for (j = 0; j < BLOCK_N/LANES_INT_NUM; j++) {
                            int32x4_t sum_vect = vdupq_n_s32(0);
                            for (k = 0; k < BLOCK_K; k++) {
                                int32x4_t a_vec = vdupq_n_s32(a[(i + iblock) * n_a + (k + kblock)]);
                                int32x4_t b_vec = vld1q_s32(b + (k + kblock) * n_b + (j * LANES_INT_NUM + jblock));
                                sum_vect = vmlaq_s32(sum_vect, a_vec, b_vec);
                            }
                            int32x4_t oldresult = vld1q_s32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock));
                            vst1q_s32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock), vaddq_s32(oldresult, sum_vect));
                        }
                    }
                    continue;
                }

                // j has leftover, i doesn't have
                if (jblock + BLOCK_N > n_b && iblock + BLOCK_M <= m_a) {

                    //__android_log_print(ANDROID_LOG_VERBOSE, "paralleA", "%d", jblock);

                    // if k has leftover, use sequential.
                    if (kblock + BLOCK_K > n_a) {
                        //__android_log_print(ANDROID_LOG_VERBOSE, "never", "%d", jblock);
                        for (i = 0; i < BLOCK_M; i++)
                            for (j = 0; j < n_b % BLOCK_N; j++) {
                                sum = 0;
                                for (k = 0; k < n_a % BLOCK_K; k++) {
                                    sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + (j + jblock)];
                                }
                                result[(i + iblock) * n_b + (j + jblock)] += sum;
                                //__android_log_print(ANDROID_LOG_VERBOSE, "paralleA", "%d, %d, %d, %d", kblock, i + iblock, j + jblock, k + kblock);
                            }
                        continue;
                    }

                    for (i = 0; i < BLOCK_M; i++) {
                        for (j = 0; j < BLOCK_N/LANES_INT_NUM; j++) {

                            // j has leftover again in the block
                            if (j * LANES_INT_NUM + jblock + LANES_INT_NUM > n_b) {
                                //__android_log_print(ANDROID_LOG_VERBOSE, "yes", "%d", jblock);

                                //if (n_b - kblock >= 4) {
                                //    int32x4_t oldresult = vld1q_s32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock));
                                //    vst1q_s32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock), vaddq_s32(oldresult, sum_vect));
                                //}


                                int newJ, newK;
                                for (newJ = j * LANES_INT_NUM + jblock; newJ < n_b; newJ++) {
                                    int sum = 0;
                                    for (k = 0; k < BLOCK_K; k++) {
                                        sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + newJ];
                                        //__android_log_print(ANDROID_LOG_VERBOSE, "what", "%d", sum);
                                    }
                                    result[(i + iblock) * n_b + newJ] += sum;
                                }


                                continue;
                            }

                            int32x4_t sum_vect = vdupq_n_s32(0);
                            for (k = 0; k < BLOCK_K; k++) {
                                int32x4_t a_vec = vdupq_n_s32(a[(i + iblock) * n_a + (k + kblock)]);
                                int32x4_t b_vec = vld1q_s32(b + (k + kblock) * n_b + (j * LANES_INT_NUM + jblock));
                                sum_vect = vmlaq_s32(sum_vect, a_vec, b_vec);
                            }
                            int32x4_t oldresult = vld1q_s32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock));
                            vst1q_s32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock), vaddq_s32(oldresult, sum_vect));
                        }
                    }

                    continue;
                }


                // deanl with j has left over, or ij have left over at same time
                if (iblock + BLOCK_M > m_a || jblock + BLOCK_N > n_b) {
                    int maxI = iblock + BLOCK_M > m_a ? m_a%BLOCK_M : BLOCK_M;
                    int maxJ = jblock + BLOCK_N > n_b ? n_b%BLOCK_N : BLOCK_N;
                    int maxK = kblock + BLOCK_K > n_a ? n_a%BLOCK_K : BLOCK_K;
                    for (i = 0; i < maxI; i++) {
                        for (j = 0; j < maxJ; j++) {
                            sum = 0;
                            for (k = 0; k < maxK; k++) {
                                sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + (j + jblock)];
                            }
                            result[(i + iblock) * n_b + (j + jblock)] += sum;
                        }
                    }
                    continue;
                }

                // if kblock have leftover and i,j don't have, do the job in sequential.
                if (kblock + BLOCK_K > n_a) {
                    for (i = 0; i < BLOCK_M; i++)
                        for (j = 0; j < BLOCK_N; j++) {
                            sum = 0;
                            for (k = 0; k < n_a%BLOCK_K; k++) {
                                sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + (j + jblock)];
                            }
                            result[(i + iblock) * n_b + (j + jblock)] += sum;
                            //__android_log_print(ANDROID_LOG_VERBOSE, "paralleA", "%d, %d, %d, %d", kblock, i + iblock, j + jblock, k + kblock);

                        }
                    continue;
                }

                for (i = 0; i < BLOCK_M; i++) {
                    for (j = 0; j < BLOCK_N/LANES_INT_NUM; j++) {
                        int32x4_t sum_vect = vdupq_n_s32(0);
                        for (k = 0; k < BLOCK_K; k++) {
                            int32x4_t a_vec = vdupq_n_s32(a[(i + iblock) * n_a + (k + kblock)]);
                            int32x4_t b_vec = vld1q_s32(b + (k + kblock) * n_b + (j * LANES_INT_NUM + jblock));
                            sum_vect = vmlaq_s32(sum_vect, a_vec, b_vec);
                        }
                        int32x4_t oldresult = vld1q_s32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock));
                        vst1q_s32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock), vaddq_s32(oldresult, sum_vect));
                    }
                }
            }

}

/*
void matrix_float_mul(float* a, int m_a, int n_a, float* b, int m_b, int n_b, float* result) {
    int i, j ,k, l;
    int mod4 = n_a%LANES_FLOAT_NUM;
    for (i = 0; i < m_a; i++) {
        for (j = 0; j < n_b; j++) {
            float32x4_t sum_vect = vdupq_n_f32(0);
            float sum = 0;
            for (k = 0; k < n_a/LANES_FLOAT_NUM; k++) {
                float32x4_t a_vec = vld1q_f32(a + i * n_a + k * LANES_FLOAT_NUM);
                float32x4_t b_vec;

                b_vec = vld1q_lane_f32(b + k * LANES_FLOAT_NUM * n_b + j, b_vec, 0);
                b_vec = vld1q_lane_f32(b + k * LANES_FLOAT_NUM * n_b + j + n_b, b_vec, 1);
                b_vec = vld1q_lane_f32(b + k * LANES_FLOAT_NUM * n_b + j + n_b * 2, b_vec, 2);
                b_vec = vld1q_lane_f32(b + k * LANES_FLOAT_NUM * n_b + j + n_b * 3, b_vec, 3);
                //sum_vect = vaddq_s32(sum_vect, tmp_mul);
                //__android_log_print(ANDROID_LOG_VERBOSE, "paralleA", "%d, %d, %d, %d, %d, %d", i, j, vgetq_lane_s32(a_vec, 0), vgetq_lane_s32(a_vec, 1), vgetq_lane_s32(a_vec, 2), vgetq_lane_s32(a_vec, 3));
                //__android_log_print(ANDROID_LOG_VERBOSE, "paralleB", "%d, %d, %d, %d, %d, %d", i, j, vgetq_lane_s32(b_vec, 0), vgetq_lane_s32(b_vec, 1), vgetq_lane_s32(b_vec, 2), vgetq_lane_s32(b_vec, 3));
                //__android_log_print(ANDROID_LOG_VERBOSE, "paralleM", "%d, %d, %d, %d, %d, %d", i, j, vgetq_lane_s32(tmp_mul, 0), vgetq_lane_s32(tmp_mul, 1), vgetq_lane_s32(tmp_mul, 2), vgetq_lane_s32(tmp_mul, 3));
                sum_vect = vmlaq_f32(sum_vect, a_vec, b_vec);
            }
            sum = vgetq_lane_f32(sum_vect, 0) + vgetq_lane_f32(sum_vect, 1) + vgetq_lane_f32(sum_vect, 2) + vgetq_lane_f32(sum_vect, 3);

            if (k == n_a/LANES_FLOAT_NUM && mod4 != 0) {

                for (k = n_a - mod4; k < n_a; k++) {
                    sum += a[i * n_a + k] * b[k * n_b + j];
                }
            }
            result[i * n_b + j] = sum;
        }
    }
}
*/

void matrix_float_mul_block(float* a, int m_a, int n_a, float* b, int m_b, int n_b, float* result) {

    int i, j ,k, iblock, jblock, kblock;
    float sum;

    for (iblock = 0; iblock < m_a; iblock += BLOCK_M)
        for (jblock = 0; jblock < n_b; jblock += BLOCK_N)
            for (kblock = 0; kblock < n_a; kblock += BLOCK_K) {

                // left over
                // for those that i have left over, while j doesn't have.
                if (iblock + BLOCK_M > m_a && jblock + BLOCK_N <= n_b) {

                    if (kblock + BLOCK_K > n_a) {
                        for (i = 0; i < m_a%BLOCK_M; i++)
                            for (j = 0; j < BLOCK_N; j++) {
                                sum = 0;
                                for (k = 0; k < n_a%BLOCK_K; k++) {
                                    sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + (j + jblock)];
                                }
                                result[(i + iblock) * n_b + (j + jblock)] += sum;
                                //__android_log_print(ANDROID_LOG_VERBOSE, "paralleA", "%d, %d, %d, %d", kblock, i + iblock, j + jblock, k + kblock);
                            }
                        continue;
                    }
                    //__android_log_print(ANDROID_LOG_VERBOSE, "doti", "%d", 1);

                    for (i = 0; i < m_a%BLOCK_M; i++) {
                        for (j = 0; j < BLOCK_N/LANES_INT_NUM; j++) {
                            float32x4_t sum_vect = vdupq_n_f32(0);
                            for (k = 0; k < BLOCK_K; k++) {
                                float32x4_t a_vec = vdupq_n_f32(a[(i + iblock) * n_a + (k + kblock)]);
                                float32x4_t b_vec = vld1q_f32(b + (k + kblock) * n_b + (j * LANES_INT_NUM + jblock));
                                sum_vect = vmlaq_f32(sum_vect, a_vec, b_vec);
                            }
                            float32x4_t oldresult = vld1q_f32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock));
                            vst1q_f32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock), vaddq_f32(oldresult, sum_vect));
                        }
                    }
                    continue;
                }

                // j has leftover, i doesn't have
                if (jblock + BLOCK_N > n_b && iblock + BLOCK_M <= m_a) {

                    //__android_log_print(ANDROID_LOG_VERBOSE, "paralleA", "%d", jblock);

                    // if k has leftover, use sequential.
                    if (kblock + BLOCK_K > n_a) {
                        //__android_log_print(ANDROID_LOG_VERBOSE, "never", "%d", jblock);
                        for (i = 0; i < BLOCK_M; i++)
                            for (j = 0; j < n_b % BLOCK_N; j++) {
                                sum = 0;
                                for (k = 0; k < n_a % BLOCK_K; k++) {
                                    sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + (j + jblock)];
                                }
                                result[(i + iblock) * n_b + (j + jblock)] += sum;
                                //__android_log_print(ANDROID_LOG_VERBOSE, "paralleA", "%d, %d, %d, %d", kblock, i + iblock, j + jblock, k + kblock);
                            }
                        continue;
                    }

                    for (i = 0; i < BLOCK_M; i++) {
                        for (j = 0; j < BLOCK_N/LANES_INT_NUM; j++) {

                            // j has leftover again in the block
                            if (j * LANES_INT_NUM + jblock + LANES_INT_NUM > n_b) {
                                //__android_log_print(ANDROID_LOG_VERBOSE, "yes", "%d", jblock);

                                //if (n_b - kblock >= 4) {
                                //    int32x4_t oldresult = vld1q_f32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock));
                                //    vst1q_f32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock), vaddq_f32(oldresult, sum_vect));
                                //}


                                int newJ, newK;
                                for (newJ = j * LANES_INT_NUM + jblock; newJ < n_b; newJ++) {
                                    float sum = 0;
                                    for (k = 0; k < BLOCK_K; k++) {
                                        sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + newJ];
                                        //__android_log_print(ANDROID_LOG_VERBOSE, "what", "%d", sum);
                                    }
                                    result[(i + iblock) * n_b + newJ] += sum;
                                }


                                continue;
                            }

                            float32x4_t sum_vect = vdupq_n_f32(0);
                            for (k = 0; k < BLOCK_K; k++) {
                                float32x4_t a_vec = vdupq_n_f32(a[(i + iblock) * n_a + (k + kblock)]);
                                float32x4_t b_vec = vld1q_f32(b + (k + kblock) * n_b + (j * LANES_INT_NUM + jblock));
                                sum_vect = vmlaq_f32(sum_vect, a_vec, b_vec);
                            }
                            float32x4_t oldresult = vld1q_f32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock));
                            vst1q_f32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock), vaddq_f32(oldresult, sum_vect));
                        }
                    }

                    continue;
                }

                // deanl with j has left over, or ij have left over at same time
                if (iblock + BLOCK_M > m_a || jblock + BLOCK_N > n_b) {
                    int maxI = iblock + BLOCK_M > m_a ? m_a%BLOCK_M : BLOCK_M;
                    int maxJ = jblock + BLOCK_N > n_b ? n_b%BLOCK_N : BLOCK_N;
                    int maxK = kblock + BLOCK_K > n_a ? n_a%BLOCK_K : BLOCK_K;
                    for (i = 0; i < maxI; i++) {
                        for (j = 0; j < maxJ; j++) {
                            sum = 0;
                            for (k = 0; k < maxK; k++) {
                                sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + (j + jblock)];
                            }
                            result[(i + iblock) * n_b + (j + jblock)] += sum;
                        }
                    }
                    continue;
                }

                // if kblock have leftover and i,j don't have, do the job in sequential.
                if (kblock + BLOCK_K > n_a) {
                    for (i = 0; i < BLOCK_M; i++)
                        for (j = 0; j < BLOCK_N; j++) {
                            sum = 0;
                            for (k = 0; k < n_a%BLOCK_K; k++) {
                                sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + (j + jblock)];
                            }
                            result[(i + iblock) * n_b + (j + jblock)] += sum;
                            //__android_log_print(ANDROID_LOG_VERBOSE, "paralleA", "%d, %d, %d, %d", kblock, i + iblock, j + jblock, k + kblock);

                        }
                    continue;
                }

                for (i = 0; i < BLOCK_M; i++) {
                    for (j = 0; j < BLOCK_N/LANES_INT_NUM; j++) {
                        float32x4_t sum_vect = vdupq_n_f32(0);
                        for (k = 0; k < BLOCK_K; k++) {
                            float32x4_t a_vec = vdupq_n_f32(a[(i + iblock) * n_a + (k + kblock)]);
                            float32x4_t b_vec = vld1q_f32(b + (k + kblock) * n_b + (j * LANES_INT_NUM + jblock));
                            sum_vect = vmlaq_f32(sum_vect, a_vec, b_vec);
                        }
                        float32x4_t oldresult = vld1q_f32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock));
                        vst1q_f32(result + (i + iblock) * n_b + (j * LANES_INT_NUM + jblock), vaddq_f32(oldresult, sum_vect));
                    }
                }
            }
}

void matrix_float_mul_block_v2(float* a, int m_a, int n_a, float* b, int m_b, int n_b, float* result) {

    int i, j ,k, iblock, jblock, kblock;
    float sum;

    for (iblock = 0; iblock < m_a; iblock += BLOCK_M)
        for (jblock = 0; jblock < n_b; jblock += BLOCK_N)
            for (kblock = 0; kblock < n_a; kblock += BLOCK_K) {

                for (i = 0; i < BLOCK_M; i++) {
                    for (j = 0; j < BLOCK_N; j++) {
                        float c = result[(i + iblock) * n_b + (j + jblock)];

                        for (k = 0; k < BLOCK_K; k+=LANES_FLOAT_NUM) {
                            float32x4_t a_vec = vld1q_f32(a + (i + iblock) * n_a + (k + kblock));
                            float32x4_t b_vec = vld1q_f32(b + (j + jblock) * n_b + (k + kblock));
                            //sum_vect = vmlaq_f32(sum_vect, a_vec, b_vec);
                            float32x4_t mul = vmulq_f32(a_vec, b_vec);
                            c += (vgetq_lane_f32(mul, 0) + vgetq_lane_f32(mul, 1) + vgetq_lane_f32(mul, 2) + vgetq_lane_f32(mul, 3));

                        }
                        //vst1q_f32(result, mul);
                        result[(i + iblock) * n_b + (j + jblock)] = c;
                    }
                }
            }
}

void matrix_short_mul(short* a, int m_a, int n_a, short* b, int m_b, int n_b, short* result) {
    int i, j ,k, l;
    int mod4 = n_a%LANES_SHORT_NUM;
    for (i = 0; i < m_a; i++) {
        for (j = 0; j < n_b; j++) {
            int16x8_t sum_vect = vdupq_n_s16(0);
            short sum = 0;
            for (k = 0; k < n_a/LANES_SHORT_NUM; k++) {
                int16x8_t a_vec = vld1q_s16(a + i * n_a + k * LANES_SHORT_NUM);
                int16x8_t b_vec;

                b_vec = vld1q_lane_s16(b + k * LANES_SHORT_NUM * n_b + j, b_vec, 0);
                b_vec = vld1q_lane_s16(b + k * LANES_SHORT_NUM * n_b + j + n_b, b_vec, 1);
                b_vec = vld1q_lane_s16(b + k * LANES_SHORT_NUM * n_b + j + n_b * 2, b_vec, 2);
                b_vec = vld1q_lane_s16(b + k * LANES_SHORT_NUM * n_b + j + n_b * 3, b_vec, 3);
                b_vec = vld1q_lane_s16(b + k * LANES_SHORT_NUM * n_b + j + n_b * 4, b_vec, 4);
                b_vec = vld1q_lane_s16(b + k * LANES_SHORT_NUM * n_b + j + n_b * 5, b_vec, 5);
                b_vec = vld1q_lane_s16(b + k * LANES_SHORT_NUM * n_b + j + n_b * 6, b_vec, 6);
                b_vec = vld1q_lane_s16(b + k * LANES_SHORT_NUM * n_b + j + n_b * 7, b_vec, 7);

                sum_vect = vmlaq_s16(sum_vect, a_vec, b_vec);
            }
            sum = vgetq_lane_s16(sum_vect, 0) + vgetq_lane_s16(sum_vect, 1) + vgetq_lane_s16(sum_vect, 2) + vgetq_lane_s16(sum_vect, 3) + vgetq_lane_s16(sum_vect, 4) + vgetq_lane_s16(sum_vect, 5) + vgetq_lane_s16(sum_vect, 6) + vgetq_lane_s16(sum_vect, 7);

            if (k == n_a/LANES_SHORT_NUM && mod4 != 0) {

                for (k = n_a - mod4; k < n_a; k++) {
                    sum += a[i * n_a + k] * b[k * n_b + j];
                }
            }
            result[i * n_b + j] = sum;
        }
    }
}

// http://www.cnblogs.com/esing/p/4471543.html
// input should be 4*4 matirx
void matrix_transpose_intrin_4x4(int* a, int m_a, int n_a, int* result) {
    int32x4_t a_vec = vld1q_s32(a);
    int32x4_t b_vec = vld1q_s32(a + 4);
    int32x4_t c_vec = vld1q_s32(a + 8);
    int32x4_t d_vec = vld1q_s32(a + 12);

    int32x4x2_t a_2 = vzipq_s32 (a_vec, c_vec);
    int32x4x2_t b_2 = vzipq_s32 (b_vec, d_vec);

    int32x4x2_t a_3 = vzipq_s32 (a_2.val[0], b_2.val[0]);
    int32x4x2_t b_3 = vzipq_s32 (a_2.val[1], b_2.val[1]);

    vst1q_s32(result + 0,  a_3.val[0]);
    vst1q_s32(result + 4,  a_3.val[1]);
    vst1q_s32(result + 8,  b_3.val[0]);
    vst1q_s32(result + 12, b_3.val[1]);
}


/*
 [a_l, a_h;
  b_l, b_h;
  c_l, c_h;
  d_l, d_h;
  e_l, e_h;
  f_l, f_h;
  g_l, g_h;
  h_l, h_h]
*/
// input should be 8*8 matirx
void matrix_transpose_intrin_8x8(int* a, int m_a, int n_a, int* result) {
    // a
    int32x4_t vec1_l = vld1q_s32(a);
    int32x4_t vec1_h = vld1q_s32(a + 4);
    // e
    int32x4_t vec2_l = vld1q_s32(a + 32);
    int32x4_t vec2_h = vld1q_s32(a + 36);

    // (t0_l, t0_h) = (a[0], e[0], a[1], e[1], a[2], e[2], a[3], e[3])
    // (t1_l, t1_h) = (a[4], e[4], a[5], e[5], a[6], e[6], a[7], e[7])
    int32x4x2_t t0 = vzipq_s32(vec1_l, vec2_l);
    int32x4x2_t t1 = vzipq_s32(vec1_h, vec2_h);


    // b
    vec1_l = vld1q_s32(a + 8);
    vec1_h = vld1q_s32(a + 4 + 8);
    // f
    vec2_l = vld1q_s32(a + 32 + 8);
    vec2_h = vld1q_s32(a + 32 + 8);

    // (t2_l, t2_h) = (b[0], f[0], b[1], f[1], b[2], f[2], b[3], f[3])
    // (t3_l, t3_h) = (b[4], f[4], b[5], f[5], b[6], f[6], b[7], f[7])
    int32x4x2_t t2 = vzipq_s32(vec1_l, vec2_l);
    int32x4x2_t t3 = vzipq_s32(vec1_h, vec2_h);


    // c
    vec1_l = vld1q_s32(a + 8 * 2);
    vec1_h = vld1q_s32(a + 4 + 8 * 2);
    // g
    vec2_l = vld1q_s32(a + 32 + 8 * 2);
    vec2_h = vld1q_s32(a + 32 + 8 * 2);

    // (t4_l, t4_h) = (c[0], g[0], c[1], g[1], c[2], g[2], c[3], g[3])
    // (t5_l, t5_h) = (c[4], g[4], c[5], g[5], c[6], g[6], c[7], g[7])
    int32x4x2_t t4 = vzipq_s32(vec1_l, vec2_l);
    int32x4x2_t t5 = vzipq_s32(vec1_h, vec2_h);


    // d
    vec1_l = vld1q_s32(a + 8 * 3);
    vec1_h = vld1q_s32(a + 4 + 8 * 3);
    // h
    vec2_l = vld1q_s32(a + 32 + 8 * 3);
    vec2_h = vld1q_s32(a + 32 + 8 * 3);

    // (t6_l, t6_h) = (d[0], h[0], d[1], h[1], d[2], h[2], d[3], h[3])
    // (t7_l, t7_h) = (d[4], h[4], d[5], h[5], d[6], h[6], d[7], h[7])
    int32x4x2_t t6 = vzipq_s32(vec1_l, vec2_l);
    int32x4x2_t t7 = vzipq_s32(vec1_h, vec2_h);


    // (m0_l, m0_h) = (a[0], c[0], e[0], g[0], a[1], c[1], e[1], g[1])
    int32x4x2_t m0 = vzipq_s32(t0.val[0], t4.val[0]);
    // (m1_l, m1_h) = (a[2], c[2], e[2], g[2], a[3], c[3], e[3], g[3])
    int32x4x2_t m1 = vzipq_s32(t0.val[1], t4.val[1]);

    // (m2_l, m2_h) = (a[4], c[4], e[4], g[4], a[5], c[5], e[5], g[5])
    int32x4x2_t m2 = vzipq_s32(t1.val[0], t5.val[0]);
    // (m3_l, m3_h) = (a[6], c[6], e[6], g[6], a[7], c[7], e[7], g[7])
    int32x4x2_t m3 = vzipq_s32(t1.val[1], t5.val[1]);

    // (m4_l, m4_h) = (b[0], d[0], f[0], h[0], b[1], d[1], f[1], h[1])
    int32x4x2_t m4 = vzipq_s32(t2.val[0], t6.val[0]);
    // (m5_l, m5_h) = (b[2], d[2], f[2], h[2], b[3], d[3], f[3], h[3])
    int32x4x2_t m5 = vzipq_s32(t2.val[1], t6.val[1]);

    // (m6_l, m6_h) = (b[4], d[4], f[4], h[4], b[5], d[5], f[5], h[5])
    int32x4x2_t m6 = vzipq_s32(t3.val[0], t7.val[0]);
    // (m7_l, m7_h) = (b[6], d[6], f[6], h[6], b[7], d[7], f[7], h[7])
    int32x4x2_t m7 = vzipq_s32(t3.val[1], t7.val[1]);



    t0 = vzipq_s32(m0.val[0], m4.val[0]);
    t1 = vzipq_s32(m0.val[1], m4.val[1]);

    t2 = vzipq_s32(m1.val[0], m5.val[0]);
    t3 = vzipq_s32(m1.val[1], m5.val[1]);

    t4 = vzipq_s32(m2.val[0], m6.val[0]);
    t5 = vzipq_s32(m2.val[1], m6.val[1]);

    t6 = vzipq_s32(m3.val[0], m7.val[0]);
    t7 = vzipq_s32(m3.val[1], m7.val[1]);



    vst1q_s32(result + 0,  t0.val[0]);
    vst1q_s32(result + 4,  t0.val[1]);
    vst1q_s32(result + 8,  t1.val[0]);
    vst1q_s32(result + 12, t1.val[1]);
    vst1q_s32(result + 16, t2.val[0]);
    vst1q_s32(result + 20, t2.val[1]);
    vst1q_s32(result + 24, t3.val[0]);
    vst1q_s32(result + 28, t3.val[1]);

    vst1q_s32(result + 32, t4.val[0]);
    vst1q_s32(result + 36, t4.val[1]);
    vst1q_s32(result + 40, t5.val[0]);
    vst1q_s32(result + 44, t5.val[1]);
    vst1q_s32(result + 48, t6.val[0]);
    vst1q_s32(result + 52, t6.val[1]);
    vst1q_s32(result + 56, t7.val[0]);
    vst1q_s32(result + 60, t7.val[1]);
}

// transoise that applied to all sizes matrix
void matrix_transpose_int_neon(int* a, int m_a, int n_a, int* result) {
    int i, j;

    // i * n_a + j
    for (i = 0; i < m_a; i += BLOCK_TRANSPOSE) {
        if (i + BLOCK_TRANSPOSE > m_a) {
            i -= (BLOCK_TRANSPOSE - m_a%BLOCK_TRANSPOSE);
        }
        for (j = 0; j < n_a; j += BLOCK_TRANSPOSE) {
            if (j + BLOCK_TRANSPOSE > n_a) {
                j -= (BLOCK_TRANSPOSE - n_a%BLOCK_TRANSPOSE);
            }
            //__android_log_print(ANDROID_LOG_VERBOSE, "sequential", "%d, %d", i, j);
            // a
            int32x4_t vec1_l = vld1q_s32(a + i * n_a + j);
            int32x4_t vec1_h = vld1q_s32(a + i * n_a + j + 4);
            // e
            int32x4_t vec2_l = vld1q_s32(a + (i + 4) * n_a + j);
            int32x4_t vec2_h = vld1q_s32(a + (i + 4) * n_a + j + 4);

            // (t0_l, t0_h) = (a[0], e[0], a[1], e[1], a[2], e[2], a[3], e[3])
            // (t1_l, t1_h) = (a[4], e[4], a[5], e[5], a[6], e[6], a[7], e[7])
            int32x4x2_t t0 = vzipq_s32(vec1_l, vec2_l);
            int32x4x2_t t1 = vzipq_s32(vec1_h, vec2_h);


            // b
            vec1_l = vld1q_s32(a + (i + 1) * n_a + j);
            vec1_h = vld1q_s32(a + (i + 1) * n_a + j + 4);
            // f
            vec2_l = vld1q_s32(a + (i + 5) * n_a + j);
            vec2_h = vld1q_s32(a + (i + 5) * n_a + j + 4);

            // (t2_l, t2_h) = (b[0], f[0], b[1], f[1], b[2], f[2], b[3], f[3])
            // (t3_l, t3_h) = (b[4], f[4], b[5], f[5], b[6], f[6], b[7], f[7])
            int32x4x2_t t2 = vzipq_s32(vec1_l, vec2_l);
            int32x4x2_t t3 = vzipq_s32(vec1_h, vec2_h);

            // c
            vec1_l = vld1q_s32(a + (i + 2) * n_a + j);
            vec1_h = vld1q_s32(a + (i + 2) * n_a + j + 4);
            // g
            vec2_l = vld1q_s32(a + (i + 6) * n_a + j);
            vec2_h = vld1q_s32(a + (i + 6) * n_a + j + 4);

            // (t4_l, t4_h) = (c[0], g[0], c[1], g[1], c[2], g[2], c[3], g[3])
            // (t5_l, t5_h) = (c[4], g[4], c[5], g[5], c[6], g[6], c[7], g[7])
            int32x4x2_t t4 = vzipq_s32(vec1_l, vec2_l);
            int32x4x2_t t5 = vzipq_s32(vec1_h, vec2_h);


            // d
            vec1_l = vld1q_s32(a + (i + 3) * n_a + j);
            vec1_h = vld1q_s32(a + (i + 3) * n_a + j + 4);
            // h
            vec2_l = vld1q_s32(a + (i + 7) * n_a + j);
            vec2_h = vld1q_s32(a + (i + 7) * n_a + j + 4);

            // (t6_l, t6_h) = (d[0], h[0], d[1], h[1], d[2], h[2], d[3], h[3])
            // (t7_l, t7_h) = (d[4], h[4], d[5], h[5], d[6], h[6], d[7], h[7])
            int32x4x2_t t6 = vzipq_s32(vec1_l, vec2_l);
            int32x4x2_t t7 = vzipq_s32(vec1_h, vec2_h);



            // (m0_l, m0_h) = (a[0], c[0], e[0], g[0], a[1], c[1], e[1], g[1])
            int32x4x2_t m0 = vzipq_s32(t0.val[0], t4.val[0]);
            // (m1_l, m1_h) = (a[2], c[2], e[2], g[2], a[3], c[3], e[3], g[3])
            int32x4x2_t m1 = vzipq_s32(t0.val[1], t4.val[1]);

            // (m2_l, m2_h) = (a[4], c[4], e[4], g[4], a[5], c[5], e[5], g[5])
            int32x4x2_t m2 = vzipq_s32(t1.val[0], t5.val[0]);
            // (m3_l, m3_h) = (a[6], c[6], e[6], g[6], a[7], c[7], e[7], g[7])
            int32x4x2_t m3 = vzipq_s32(t1.val[1], t5.val[1]);

            // (m4_l, m4_h) = (b[0], d[0], f[0], h[0], b[1], d[1], f[1], h[1])
            int32x4x2_t m4 = vzipq_s32(t2.val[0], t6.val[0]);
            // (m5_l, m5_h) = (b[2], d[2], f[2], h[2], b[3], d[3], f[3], h[3])
            int32x4x2_t m5 = vzipq_s32(t2.val[1], t6.val[1]);

            // (m6_l, m6_h) = (b[4], d[4], f[4], h[4], b[5], d[5], f[5], h[5])
            int32x4x2_t m6 = vzipq_s32(t3.val[0], t7.val[0]);
            // (m7_l, m7_h) = (b[6], d[6], f[6], h[6], b[7], d[7], f[7], h[7])
            int32x4x2_t m7 = vzipq_s32(t3.val[1], t7.val[1]);


            t0 = vzipq_s32(m0.val[0], m4.val[0]);
            t1 = vzipq_s32(m0.val[1], m4.val[1]);

            t2 = vzipq_s32(m1.val[0], m5.val[0]);
            t3 = vzipq_s32(m1.val[1], m5.val[1]);

            t4 = vzipq_s32(m2.val[0], m6.val[0]);
            t5 = vzipq_s32(m2.val[1], m6.val[1]);

            t6 = vzipq_s32(m3.val[0], m7.val[0]);
            t7 = vzipq_s32(m3.val[1], m7.val[1]);

            vst1q_s32(result + (j + 0) * m_a + i,  t0.val[0]);
            vst1q_s32(result + (j + 0) * m_a + i + 4,  t0.val[1]);
            vst1q_s32(result + (j + 1) * m_a + i,  t1.val[0]);
            vst1q_s32(result + (j + 1) * m_a + i + 4, t1.val[1]);
            vst1q_s32(result + (j + 2) * m_a + i, t2.val[0]);
            vst1q_s32(result + (j + 2) * m_a + i + 4, t2.val[1]);
            vst1q_s32(result + (j + 3) * m_a + i, t3.val[0]);
            vst1q_s32(result + (j + 3) * m_a + i + 4, t3.val[1]);

            vst1q_s32(result + (j + 4) * m_a + i, t4.val[0]);
            vst1q_s32(result + (j + 4) * m_a + i + 4, t4.val[1]);
            vst1q_s32(result + (j + 5) * m_a + i, t5.val[0]);
            vst1q_s32(result + (j + 5) * m_a + i + 4, t5.val[1]);
            vst1q_s32(result + (j + 6) * m_a + i, t6.val[0]);
            vst1q_s32(result + (j + 6) * m_a + i + 4, t6.val[1]);
            vst1q_s32(result + (j + 7) * m_a + i, t7.val[0]);
            vst1q_s32(result + (j + 7) * m_a + i + 4, t7.val[1]);
        }
    }
}

void matrix_transpose_float_neon(float* a, int m_a, int n_a, float* result) {
    int i, j;

    // i * n_a + j
    for (i = 0; i < m_a; i += BLOCK_TRANSPOSE) {
        if (i + BLOCK_TRANSPOSE > m_a) {
            i -= (BLOCK_TRANSPOSE - m_a%BLOCK_TRANSPOSE);
        }
        for (j = 0; j < n_a; j += BLOCK_TRANSPOSE) {
            if (j + BLOCK_TRANSPOSE > n_a) {
                j -= (BLOCK_TRANSPOSE - n_a%BLOCK_TRANSPOSE);
            }
            //__android_log_print(ANDROID_LOG_VERBOSE, "sequential", "%d, %d", i, j);
            // a
            float32x4_t vec1_l = vld1q_f32(a + i * n_a + j);
            float32x4_t vec1_h = vld1q_f32(a + i * n_a + j + 4);
            // e
            float32x4_t vec2_l = vld1q_f32(a + (i + 4) * n_a + j);
            float32x4_t vec2_h = vld1q_f32(a + (i + 4) * n_a + j + 4);

            // (t0_l, t0_h) = (a[0], e[0], a[1], e[1], a[2], e[2], a[3], e[3])
            // (t1_l, t1_h) = (a[4], e[4], a[5], e[5], a[6], e[6], a[7], e[7])
            float32x4x2_t t0 = vzipq_f32(vec1_l, vec2_l);
            float32x4x2_t t1 = vzipq_f32(vec1_h, vec2_h);


            // b
            vec1_l = vld1q_f32(a + (i + 1) * n_a + j);
            vec1_h = vld1q_f32(a + (i + 1) * n_a + j + 4);
            // f
            vec2_l = vld1q_f32(a + (i + 5) * n_a + j);
            vec2_h = vld1q_f32(a + (i + 5) * n_a + j + 4);

            // (t2_l, t2_h) = (b[0], f[0], b[1], f[1], b[2], f[2], b[3], f[3])
            // (t3_l, t3_h) = (b[4], f[4], b[5], f[5], b[6], f[6], b[7], f[7])
            float32x4x2_t t2 = vzipq_f32(vec1_l, vec2_l);
            float32x4x2_t t3 = vzipq_f32(vec1_h, vec2_h);

            // c
            vec1_l = vld1q_f32(a + (i + 2) * n_a + j);
            vec1_h = vld1q_f32(a + (i + 2) * n_a + j + 4);
            // g
            vec2_l = vld1q_f32(a + (i + 6) * n_a + j);
            vec2_h = vld1q_f32(a + (i + 6) * n_a + j + 4);

            // (t4_l, t4_h) = (c[0], g[0], c[1], g[1], c[2], g[2], c[3], g[3])
            // (t5_l, t5_h) = (c[4], g[4], c[5], g[5], c[6], g[6], c[7], g[7])
            float32x4x2_t t4 = vzipq_f32(vec1_l, vec2_l);
            float32x4x2_t t5 = vzipq_f32(vec1_h, vec2_h);


            // d
            vec1_l = vld1q_f32(a + (i + 3) * n_a + j);
            vec1_h = vld1q_f32(a + (i + 3) * n_a + j + 4);
            // h
            vec2_l = vld1q_f32(a + (i + 7) * n_a + j);
            vec2_h = vld1q_f32(a + (i + 7) * n_a + j + 4);

            // (t6_l, t6_h) = (d[0], h[0], d[1], h[1], d[2], h[2], d[3], h[3])
            // (t7_l, t7_h) = (d[4], h[4], d[5], h[5], d[6], h[6], d[7], h[7])
            float32x4x2_t t6 = vzipq_f32(vec1_l, vec2_l);
            float32x4x2_t t7 = vzipq_f32(vec1_h, vec2_h);



            // (m0_l, m0_h) = (a[0], c[0], e[0], g[0], a[1], c[1], e[1], g[1])
            float32x4x2_t m0 = vzipq_f32(t0.val[0], t4.val[0]);
            // (m1_l, m1_h) = (a[2], c[2], e[2], g[2], a[3], c[3], e[3], g[3])
            float32x4x2_t m1 = vzipq_f32(t0.val[1], t4.val[1]);

            // (m2_l, m2_h) = (a[4], c[4], e[4], g[4], a[5], c[5], e[5], g[5])
            float32x4x2_t m2 = vzipq_f32(t1.val[0], t5.val[0]);
            // (m3_l, m3_h) = (a[6], c[6], e[6], g[6], a[7], c[7], e[7], g[7])
            float32x4x2_t m3 = vzipq_f32(t1.val[1], t5.val[1]);

            // (m4_l, m4_h) = (b[0], d[0], f[0], h[0], b[1], d[1], f[1], h[1])
            float32x4x2_t m4 = vzipq_f32(t2.val[0], t6.val[0]);
            // (m5_l, m5_h) = (b[2], d[2], f[2], h[2], b[3], d[3], f[3], h[3])
            float32x4x2_t m5 = vzipq_f32(t2.val[1], t6.val[1]);

            // (m6_l, m6_h) = (b[4], d[4], f[4], h[4], b[5], d[5], f[5], h[5])
            float32x4x2_t m6 = vzipq_f32(t3.val[0], t7.val[0]);
            // (m7_l, m7_h) = (b[6], d[6], f[6], h[6], b[7], d[7], f[7], h[7])
            float32x4x2_t m7 = vzipq_f32(t3.val[1], t7.val[1]);


            t0 = vzipq_f32(m0.val[0], m4.val[0]);
            t1 = vzipq_f32(m0.val[1], m4.val[1]);

            t2 = vzipq_f32(m1.val[0], m5.val[0]);
            t3 = vzipq_f32(m1.val[1], m5.val[1]);

            t4 = vzipq_f32(m2.val[0], m6.val[0]);
            t5 = vzipq_f32(m2.val[1], m6.val[1]);

            t6 = vzipq_f32(m3.val[0], m7.val[0]);
            t7 = vzipq_f32(m3.val[1], m7.val[1]);

            vst1q_f32(result + (j + 0) * m_a + i,  t0.val[0]);
            vst1q_f32(result + (j + 0) * m_a + i + 4,  t0.val[1]);
            vst1q_f32(result + (j + 1) * m_a + i,  t1.val[0]);
            vst1q_f32(result + (j + 1) * m_a + i + 4, t1.val[1]);
            vst1q_f32(result + (j + 2) * m_a + i, t2.val[0]);
            vst1q_f32(result + (j + 2) * m_a + i + 4, t2.val[1]);
            vst1q_f32(result + (j + 3) * m_a + i, t3.val[0]);
            vst1q_f32(result + (j + 3) * m_a + i + 4, t3.val[1]);

            vst1q_f32(result + (j + 4) * m_a + i, t4.val[0]);
            vst1q_f32(result + (j + 4) * m_a + i + 4, t4.val[1]);
            vst1q_f32(result + (j + 5) * m_a + i, t5.val[0]);
            vst1q_f32(result + (j + 5) * m_a + i + 4, t5.val[1]);
            vst1q_f32(result + (j + 6) * m_a + i, t6.val[0]);
            vst1q_f32(result + (j + 6) * m_a + i + 4, t6.val[1]);
            vst1q_f32(result + (j + 7) * m_a + i, t7.val[0]);
            vst1q_f32(result + (j + 7) * m_a + i + 4, t7.val[1]);
        }
    }
}

void matrix_int_add(int* a, int m_a, int n_a, int* b, int m_b, int n_b, int* results) {
    int i, j;
    for (i = 0; i < m_a; i++) {
        for (j = 0; j < n_a; j+=4) {
            if (j + 4 > n_a) {
                int k, kmax = n_a%4;
                for (k = 0; k < kmax; k++) {
                    results[i * n_a + k] = a[i * n_a + k] + b[i * n_a + k];
                }
                continue;
            }
            int32x4_t a_vec = vld1q_s32(a + i * n_a + j);
            int32x4_t b_vec = vld1q_s32(b + i * n_a + j);
            int32x4_t result = vaddq_s32(a_vec, b_vec);
            vst1q_s32(results + i * n_a + j, result);
        }
    }
}

void matrix_float_add(float* a, int m_a, int n_a, float* b, int m_b, int n_b, float* results) {
    int i, j;
    for (i = 0; i < m_a; i++) {
        for (j = 0; j < n_a; j+=4) {
            if (j + 4 > n_a) {
                int k, kmax = n_a%4;
                for (k = 0; k < kmax; k++) {
                    results[i * n_a + k] = a[i * n_a + k] + b[i * n_a + k];
                }
                continue;
            }
            float32x4_t a_vec = vld1q_f32(a + i * n_a + j);
            float32x4_t b_vec = vld1q_f32(b + i * n_a + j);
            float32x4_t result = vaddq_f32(a_vec, b_vec);
            vst1q_f32(results + i * n_a + j, result);
        }
    }
}
