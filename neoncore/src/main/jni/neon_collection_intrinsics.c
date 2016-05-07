//
// Created by 朱晓光 on 4/14/16.
//

#include "neon_collection_intrinsics.h"
#include "macro_define.h"
#include "helper_set.h"
#include <arm_neon.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <omp.h>
#include <math.h>
#include <assert.h>



/* Int Version */
void vector_intrinsics_int(int* x, int len, int * var, int num_var) {
    int l = len >> 2;
    int left = len - (l<<2);
    int i;
    int32x4_t deal_left;
    if (left > 0) {
        deal_left = vld1q_s32(x + len - 4);
    }
    for (i = 0; i < l; i++) {
        int32x4_t ans = vdupq_n_s32(var[0]);
        int32x4_t x_tmp = vld1q_s32(x + i*4);
        int32x4_t x_cul = x_tmp;
        int j;
        for (j = 1; j < num_var; j++) {
            int32x4_t var_v = vdupq_n_s32(var[j]);
            ans = vmlaq_s32(ans, x_cul, var_v);
            x_cul = vmulq_s32(x_cul, x_tmp);
        }
        vst1q_s32(x + i*4, ans);
    }
    if (left > 0) {
        int32x4_t ans = vdupq_n_s32(var[0]);
        int32x4_t x_tmp = deal_left;
        int32x4_t x_cul = x_tmp;
        int j = 1;
        for (j = 1; j < num_var; j++) {
            int32x4_t var_v = vdupq_n_s32(var[j]);
            ans = vmlaq_s32(ans, x_cul, var_v);
            x_cul = vmulq_s32(x_cul, x_tmp);
        }
        vst1q_s32(x + len - 4, ans);
    }
}


/* Float Version */
void vector_intrinsics_float(float* x, int len, float* var, int num_var) {
    int l = len >> 2;
    int left = len - (l<<2);
    int i;
    float32x4_t deal_left;
    if (left > 0) {
        deal_left = vld1q_f32(x + len - 4);
    }
    for (i = 0; i < l; i++) {
        float32x4_t ans = vdupq_n_f32(var[0]);
        float32x4_t x_tmp = vld1q_f32(x + i*4);
        float32x4_t x_cul = x_tmp;
        int j;
        for (j = 1; j < num_var; j++) {
            float32x4_t var_v = vdupq_n_f32(var[j]);
            ans = vmlaq_f32(ans, x_cul, var_v);
            x_cul = vmulq_f32(x_cul, x_tmp);
        }
        vst1q_f32(x + i*4, ans);
    }
    if (left > 0) {
        float32x4_t ans = vdupq_n_f32(var[0]);
        float32x4_t x_tmp = deal_left;
        float32x4_t x_cul = x_tmp;
        int j = 1;
        for (j = 1; j < num_var; j++) {
            float32x4_t var_v = vdupq_n_f32(var[j]);
            ans = vmlaq_f32(ans, x_cul, var_v);
            x_cul = vmulq_f32(x_cul, x_tmp);
        }
        vst1q_f32(x + len - 4, ans);
    }
}


///* Complex Version */
//void vector_intrinsics_complex(float* real, float* imag, int len, float* var, int num_var) {
//
//}
