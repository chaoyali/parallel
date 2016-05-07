//
// Created by 朱晓光 on 5/7/16.
//

#include "neon_collection_intrinsics_fft.h"
#include "macro_define.h"
#include "helper_set.h"
#include <arm_neon.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <omp.h>
#include <math.h>
#include <assert.h>

#define SWAP(a, b) \
{ \
    float tmp = a;\
    a = b;\
    b = tmp;\
}

#define COMP_FLOAT_ADD(dstr, dsti, ar, ai, br, bi) \
{\
    dstr = ar + br;\
    dsti = ai + bi;\
}

#define COMP_FLOAT_MINUS(dstr, dsti, ar, ai, br, bi) \
{\
    dstr = ar - br;\
    dsti = ai - bi;\
}

#define COMP_FLOAT_MULTI(dstr, dsti, ar, ai, br, bi) \
{\
    float arbr = ar * br, aibi = ai * bi, arbi = ar * bi, aibr = ai * br;\
    dstr = arbr - aibi;\
    dsti = arbi + aibr;\
}


// Following is for FFT.

inline void comp_float_add_intric(float32x4_t* dstr, float32x4_t* dsti, float32x4_t ar, float32x4_t ai, float32x4_t br, float32x4_t bi) {
    *dstr = vaddq_f32(ar, br);
    *dsti = vaddq_f32(ai, bi);
}

inline void comp_float_minus_intric(float32x4_t* dstr, float32x4_t* dsti, float32x4_t ar, float32x4_t ai, float32x4_t br, float32x4_t bi) {
    *dstr = vsubq_f32(ar, br);
    *dsti = vsubq_f32(ai, bi);
}


inline void comp_float_multi_intric(float32x4_t* dstr, float32x4_t* dsti, float32x4_t ar, float32x4_t ai, float32x4_t br, float32x4_t bi) {
    float32x4_t arbr = vmulq_f32(ar, br);
    float32x4_t aibi = vmulq_f32(ai, bi);
    float32x4_t aibr = vmulq_f32(ai, br);
    float32x4_t arbi = vmulq_f32(ar, bi);
    *dstr = vsubq_f32(arbr, aibi);
    *dsti = vaddq_f32(aibr, arbi);
}

void fft_intrinsics_float(float * real, float * imag, int len, int reverse) {
    int m = 2;
    int h = len >> 1;

    while (h >= 4) {
        float32x4_t w_m_real = vdupq_n_f32((float32_t) cos(M_PI * 2 / m));
        float32x4_t w_m_imag;
        if (reverse)
            w_m_imag = vdupq_n_f32((float32_t) sin(M_PI * 2 / m));
        else
            w_m_imag = vdupq_n_f32((float32_t) -sin(M_PI * 2 / m));
        float32x4_t w_real = vdupq_n_f32(1.0);
        float32x4_t w_imag = vdupq_n_f32(0.0);

        int b, k;
        for (b = 0; b < m/2; b++) {
            int bit = get_reverse(b, len);
            for (k = 0; k < h; k += 4) {

                float32x4_t real_f1 = vld1q_f32(real + bit + k), imag_f1 = vld1q_f32(imag + bit + k), real_f2 = vld1q_f32(real + bit + h + k), imag_f2 = vld1q_f32(imag + bit + h + k);
                float32x4_t u_real = real_f1, u_imag = imag_f1, t_real, t_imag;

                comp_float_multi_intric(&t_real, &t_imag, real_f2, imag_f2, w_real, w_imag);
                comp_float_add_intric(&(real_f1), &(imag_f1), u_real, u_imag, t_real, t_imag);
                comp_float_minus_intric(&(real_f2), &(imag_f2), u_real, u_imag, t_real, t_imag);

                vst1q_f32(real + bit + k, real_f1);
                vst1q_f32(imag + bit + k, imag_f1);
                vst1q_f32(real + bit + h + k, real_f2);
                vst1q_f32(imag + bit + h + k, imag_f2);
            }
            comp_float_multi_intric(&w_real, &w_imag, w_m_real, w_m_imag, w_real, w_imag);
        }
        m = m << 1;
        h = h >> 1;
    }

//    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "SECONDHERE!!!!");

    while (m <= len) {
        float w_m_real = (float)cos(M_PI * 2 / m);
        float w_m_imag = (float)-sin(M_PI * 2 / m);
        if (reverse) w_m_imag = -w_m_imag;

        float w_real = 1.0;
        float w_imag = 0.0;
        int b, k;
        for (b = 0; b < m/2; b++) {
            int bit = get_reverse(b, len);
            for (k = 0; k < h; k++) {
                float u_real = real[bit + k], u_imag = imag[bit + k], t_real, t_imag;
                COMP_FLOAT_MULTI(t_real, t_imag, w_real, w_imag, real[bit + h + k], imag[bit + h + k]);
                COMP_FLOAT_ADD(real[bit + k], imag[bit + k], u_real, u_imag, t_real, t_imag);
                COMP_FLOAT_MINUS(real[bit + h + k], imag[bit + h + k], u_real, u_imag, t_real, t_imag);
            }
            COMP_FLOAT_MULTI(w_real, w_imag, w_real, w_imag, w_m_real, w_m_imag);
        }

        m = m << 1;
        h = h >> 1;
    }

    {
        int i;
        for (i = 0; i < len; i++) {
            int bit = get_reverse(i, len);
            if (bit >= i)  continue;
            SWAP(real[i], real[bit]);
            SWAP(imag[i], imag[bit]);
        }
    }

}