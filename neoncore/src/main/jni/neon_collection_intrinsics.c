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

void combsort_intrinsics_int_merge(int * lst1, int len1, int * lst2, int len2);
void combsort_intrinsics_int_detail(int * lst, int len);

// Following is for AA sort.

//inline uint32_t not_all_ones(uint32x4_t v)
//{
//    uint32x2_t tmp = vand_u32(vget_low_u32(v), vget_high_u32(v));
//    return vget_lane_u32(vpmin_u32(tmp, tmp), 0);
//}

inline uint32_t not_all_ones_cnt(uint32x4_t v)
{
    return vgetq_lane_u32(v, 0) & vgetq_lane_u32(v, 1) & vgetq_lane_u32(v, 2) & vgetq_lane_u32(v, 3);
}

inline uint32_t vertor_cmpswap_bool(int32x4_t *a, int32x4_t  *b) {
    int32x4_t t = vminq_s32(*a, *b);
    *b = vmaxq_s32(*a, *b);
    uint32x4_t equ = vceqq_s32(*a, t);
    *a = t;
    return not_all_ones_cnt(equ);
}

inline void vertor_cmpswap(int32x4_t *a, int32x4_t  *b) {
    int32x4_t t = vminq_s32(*a, *b);
    *b = vmaxq_s32(*a, *b);
    *a = t;
}

inline uint32_t vertor_cmpswap_skew_bool(int32x4_t *a, int32x4_t  *b, int32x4_t *tmp) {

    // Explain: vextq_s32(a[4], b[4], 3)  =>  (a3,b0,b1,b2)
    int32x4_t tmp_t = vextq_s32(*tmp, *b, 1); // => (m,m,m,b0)
    int32x4_t tmp_b = vextq_s32(*b, *tmp, 1); // => (b1,b2,b3,m)
    uint32_t ans = vertor_cmpswap_bool(a, &tmp_b);
    *b = vextq_s32(tmp_t, tmp_b, 3);
    return ans;
}


inline void vertor_cmpswap_skew(int32x4_t *a, int32x4_t  *b, int32x4_t *tmp) {
    int32x4_t tmp_t = vextq_s32(*tmp, *b, 1); // => (m,m,m,b0)
    int32x4_t tmp_b = vextq_s32(*b, *tmp, 1); // => (b1,b2,b3,m)
    vertor_cmpswap(a, &tmp_b);
    *b = vextq_s32(tmp_t, tmp_b, 3);
}

inline void vector_merge(int32x4_t *vMin, int32x4_t *vMax, int32x4_t *max_tmp) {
    int min = -INT_MAX;
    vertor_cmpswap(vMin, vMax);
    int32x4_t vMin_tmp = vextq_s32(*vMin, *max_tmp, 2); // => (b2,b3,m,m)
    int32x4_t vMin_tmp2 = vextq_s32(*max_tmp, *vMin, 2); // => (m,m,b0,b1)
    vertor_cmpswap(vMax, &vMin_tmp);
    vMin_tmp = vextq_s32(vMin_tmp2, vMin_tmp, 3); // => (b1,b2,b3,min)
    vMin_tmp = vld1q_lane_s32(&min, vMin_tmp, 3);
    vMin_tmp2 = vextq_s32(vMin_tmp, vMin_tmp2, 3);  // => (m,m,m,b0)
    vertor_cmpswap(&vMin_tmp, vMax);
    *vMin = vextq_s32(vMin_tmp2, vMin_tmp, 3);
}

inline sort_vector(int* v, int n) {
    int i = 0;
    while (i < n - 1) {
        int minj = i, j, tmp;
        for (j = i + 1; j < n; j++) {
            if (v[j] < v[minj]) {
                minj = j;
            }
        }
        tmp = v[i];
        v[i] = v[minj];
        v[minj] = tmp;
        i++;
    }
}

void combsort_intrinsics_int(int * lst, int len) {
    if (len > CACHE_LINE) {
        int l = ((len / 2 + 3) >> 2) << 2, i;
        if (l < CACHE_LINE) {
#pragma omp parallel for
            for (i = 0; i < 2; i++) {
                int add = (i == 0) ? 0 : l;
                int leng = (i == 0) ? l : len - l;
                combsort_intrinsics_int_detail(lst + add, leng);
            }
        }
        else {
            combsort_intrinsics_int(lst, l);
            combsort_intrinsics_int(lst + l, len - l);
        }
        combsort_intrinsics_int_merge(lst, l, lst+l, len - l);
    }
    else {
        combsort_intrinsics_int_detail(lst, len);
    }
}
void combsort_intrinsics_int_merge(int * lst1, int len1, int * lst2, int len2) {

    int* lst0 = (int*)malloc(len1 * sizeof(int));
    memcpy(lst0, lst1, len1 * sizeof(int));
    int aPos = 0, bPos = 0, i = 0;
    int32x4_t vMin = vld1q_s32(lst1);
    int32x4_t vMax = vld1q_s32(lst2);
    int32x4_t backup = vld1q_s32(lst2 + len2);
    int32x4_t max_tmp = vdupq_n_s32(INT_MAX);
    aPos = aPos + 4;
    bPos = bPos + 4;
//    if (len2 & 0x03 != 0) { // switch the leftover to INT_MAX
    vst1q_s32(lst2 + len2, max_tmp);
//    }
    while (aPos < len1 && bPos < len2) {
        vector_merge(&vMin, &vMax, &max_tmp);
        vst1q_s32(lst1 + i, vMin);
        i += 4;
        if (lst0[aPos] <= lst2[bPos]) {
            vMin = vld1q_s32(lst0 + aPos);
            aPos += 4;
        }
        else {
            vMin = vld1q_s32(lst2 + bPos);
            bPos += 4;
        }
    }
    vector_merge(&vMin, &vMax, &max_tmp);
    vst1q_s32(lst1 + i, vMin);
    i += 4;
    if (bPos >= len2) {
        memcpy(lst1 + i, lst0 + aPos, len1 + len2 - i);
    }
    vst1q_s32(lst2 + len2, backup);
    free(lst0);
}

void combsort_intrinsics_int_detail(int * lst, int len) {

    if (len <= 5) {
        sort_vector(lst, 5);
        return;
    }

    int l = ((len + 3) >> 2);  // Round to next 4x
    int gap = l / SHRINK_FACTOR;
    int32x4_t max_vec = vdupq_n_s32(INT_MAX);

    int left = (l << 2) - len;

    int32x4_t *arr = (int32x4_t *) malloc(l * sizeof(int32x4_t));
    {
        int i, offset;
        for (i = 0; i < l - left; i++) {
            sort_vector(lst + (i<<2), 4);
            arr[i] = vld1q_s32(lst + (i<<2));
//            arr[i] = (int32x4_t)v_load((void*)(lst + i*4));
        }
        offset = i;
        for (; i < l; i++) {
            sort_vector(lst + offset + i*3, 3);
            int tmp = lst[offset + i*3 + 3];
            lst[offset + i*3 + 3] = INT_MAX;
            arr[i] = vld1q_s32(lst + offset + i*3);
            lst[offset + i*3 + 3] = tmp;
        }
    }

    while (gap > 1) {
        int i;
        for (i = 0; i <  l - gap; i++) {
            vertor_cmpswap(&arr[i], &arr[i + gap]);
        }
        for (i = l - gap; i < l;  i++) {
            vertor_cmpswap_skew(&arr[i], &arr[i + gap - l], &max_vec);
        }
        gap = gap / SHRINK_FACTOR;
    }

    uint32_t finished = 0;
    while (!finished) {
        int i;
        finished = 1;
        for (i = 0; i < l - 1; i++) {
            finished &= vertor_cmpswap_bool(&arr[i], &arr[i + 1]);
        }
        finished &= vertor_cmpswap_skew_bool(&arr[l - 1], &arr[0], &max_vec);
    }

    {
        int i;
//        #pragma omp parallel for schedule(static)
        for (i = 0; i < l - left; i++) {
            lst[i] = vgetq_lane_s32(arr[i], 0);
            lst[l + i] = vgetq_lane_s32(arr[i], 1);
            lst[l*2 + i] = vgetq_lane_s32(arr[i], 2);
            lst[l*3 + i] = vgetq_lane_s32(arr[i], 3);
        }
        for (; i < l; i++) {
            lst[i] = vgetq_lane_s32(arr[i], 0);
            lst[l + i] = vgetq_lane_s32(arr[i], 1);
            lst[l*2 + i] = vgetq_lane_s32(arr[i], 2);
        }
    }

    free(arr);
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




// Following is for basic vector operation
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
        int j = 1;
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
