//
// Created by 朱晓光 on 5/7/16.
//

#include "neon_collection_intrinsics_sort.h"
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

void combsort_intrinsics_int_merge(int * lst1, int len1, int * lst2, int len2);
void combsort_intrinsics_int_detail(int * lst, int len);


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




/* Float Version */

void combsort_intrinsics_float_merge(float * lst1, int len1, float * lst2, int len2);
void combsort_intrinsics_float_detail(float * lst, int len);



inline uint32_t vertor_cmpswap_f_bool(float32x4_t *a, float32x4_t  *b) {
    float32x4_t t = vminq_f32(*a, *b);
    *b = vmaxq_f32(*a, *b);
    uint32x4_t equ = vceqq_f32(*a, t);
    *a = t;
    return not_all_ones_cnt(equ);
}

inline void vertor_cmpswap_f(float32x4_t *a, float32x4_t  *b) {
    float32x4_t t = vminq_f32(*a, *b);
    *b = vmaxq_f32(*a, *b);
    *a = t;
}

inline uint32_t vertor_cmpswap_f_skew_bool(float32x4_t *a, float32x4_t  *b, float32x4_t *tmp) {

    // Explain: vextq_f32(a[4], b[4], 3)  =>  (a3,b0,b1,b2)
    float32x4_t tmp_t = vextq_f32(*tmp, *b, 1); // => (m,m,m,b0)
    float32x4_t tmp_b = vextq_f32(*b, *tmp, 1); // => (b1,b2,b3,m)
    uint32_t ans = vertor_cmpswap_f_bool(a, &tmp_b);
    *b = vextq_f32(tmp_t, tmp_b, 3);
    return ans;
}


inline void vertor_cmpswap_f_skew(float32x4_t *a, float32x4_t  *b, float32x4_t *tmp) {
    float32x4_t tmp_t = vextq_f32(*tmp, *b, 1); // => (m,m,m,b0)
    float32x4_t tmp_b = vextq_f32(*b, *tmp, 1); // => (b1,b2,b3,m)
    vertor_cmpswap_f(a, &tmp_b);
    *b = vextq_f32(tmp_t, tmp_b, 3);
}

inline void vertor_merge_f(float32x4_t *vMin, float32x4_t *vMax, float32x4_t *max_tmp) {
    float min = -INT_MAX;
    vertor_cmpswap_f(vMin, vMax);
    float32x4_t vMin_tmp = vextq_f32(*vMin, *max_tmp, 2); // => (b2,b3,m,m)
    float32x4_t vMin_tmp2 = vextq_f32(*max_tmp, *vMin, 2); // => (m,m,b0,b1)
    vertor_cmpswap_f(vMax, &vMin_tmp);
    vMin_tmp = vextq_f32(vMin_tmp2, vMin_tmp, 3); // => (b1,b2,b3,min)
    vMin_tmp = vld1q_lane_f32(&min, vMin_tmp, 3);
    vMin_tmp2 = vextq_f32(vMin_tmp, vMin_tmp2, 3);  // => (m,m,m,b0)
    vertor_cmpswap_f(&vMin_tmp, vMax);
    *vMin = vextq_f32(vMin_tmp2, vMin_tmp, 3);
}

inline sort_vector_f(float* v, int n) {
    int i = 0;
    while (i < n - 1) {
        int minj = i, j;
        float tmp;
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

void combsort_intrinsics_float(float * lst, int len) {
    if (len > CACHE_LINE) {
        int l = ((len / 2 + 3) >> 2) << 2, i;
        if (l < CACHE_LINE) {
#pragma omp parallel for
            for (i = 0; i < 2; i++) {
                int add = (i == 0) ? 0 : l;
                int leng = (i == 0) ? l : len - l;
                combsort_intrinsics_float_detail(lst + add, leng);
            }
        }
        else {
            combsort_intrinsics_float(lst, l);
            combsort_intrinsics_float(lst + l, len - l);
        }
        combsort_intrinsics_float_merge(lst, l, lst+l, len - l);
    }
    else {
        combsort_intrinsics_float_detail(lst, len);
    }
}
void combsort_intrinsics_float_merge(float * lst1, int len1, float * lst2, int len2) {

    float* lst0 = (float*)malloc(len1 * sizeof(float));
    memcpy(lst0, lst1, len1 * sizeof(float));
    int aPos = 0, bPos = 0, i = 0;
    float32x4_t vMin = vld1q_f32(lst1);
    float32x4_t vMax = vld1q_f32(lst2);
    float32x4_t backup = vld1q_f32(lst2 + len2);
    float32x4_t max_tmp = vdupq_n_f32(INT_MAX);
    aPos = aPos + 4;
    bPos = bPos + 4;
//    if (len2 & 0x03 != 0) { // switch the leftover to INT_MAX
    vst1q_f32(lst2 + len2, max_tmp);
//    }
    while (aPos < len1 && bPos < len2) {
        vertor_merge_f(&vMin, &vMax, &max_tmp);
        vst1q_f32(lst1 + i, vMin);
        i += 4;
        if (lst0[aPos] <= lst2[bPos]) {
            vMin = vld1q_f32(lst0 + aPos);
            aPos += 4;
        }
        else {
            vMin = vld1q_f32(lst2 + bPos);
            bPos += 4;
        }
    }
    vertor_merge_f(&vMin, &vMax, &max_tmp);
    vst1q_f32(lst1 + i, vMin);
    i += 4;
    if (bPos >= len2) {
        memcpy(lst1 + i, lst0 + aPos, len1 + len2 - i);
    }
    vst1q_f32(lst2 + len2, backup);
    free(lst0);
}

void combsort_intrinsics_float_detail(float * lst, int len) {

    if (len <= 5) {
        sort_vector_f(lst, 5);
        return;
    }

    int l = ((len + 3) >> 2);  // Round to next 4x
    int gap = l / SHRINK_FACTOR;
    float32x4_t max_vec = vdupq_n_f32(INT_MAX);

    int left = (l << 2) - len;

    float32x4_t *arr = (float32x4_t *) malloc(l * sizeof(float32x4_t));
    {
        int i, offset;
        for (i = 0; i < l - left; i++) {
            sort_vector_f(lst + (i<<2), 4);
            arr[i] = vld1q_f32(lst + (i<<2));
//            arr[i] = (int32x4_t)v_load((void*)(lst + i*4));
        }
        offset = i;
        for (; i < l; i++) {
            sort_vector_f(lst + offset + i*3, 3);
            float tmp = lst[offset + i*3 + 3];
            lst[offset + i*3 + 3] = INT_MAX;
            arr[i] = vld1q_f32(lst + offset + i*3);
            lst[offset + i*3 + 3] = tmp;
        }
    }

    while (gap > 1) {
        int i;
        for (i = 0; i <  l - gap; i++) {
            vertor_cmpswap_f(&arr[i], &arr[i + gap]);
        }
        for (i = l - gap; i < l;  i++) {
            vertor_cmpswap_f_skew(&arr[i], &arr[i + gap - l], &max_vec);
        }
        gap = gap / SHRINK_FACTOR;
    }

    uint32_t finished = 0;
    while (!finished) {
        int i;
        finished = 1;
        for (i = 0; i < l - 1; i++) {
            finished &= vertor_cmpswap_f_bool(&arr[i], &arr[i + 1]);
        }
        finished &= vertor_cmpswap_f_skew_bool(&arr[l - 1], &arr[0], &max_vec);
    }

    {
        int i;
//        #pragma omp parallel for schedule(static)
        for (i = 0; i < l - left; i++) {
            lst[i] = vgetq_lane_f32(arr[i], 0);
            lst[l + i] = vgetq_lane_f32(arr[i], 1);
            lst[l*2 + i] = vgetq_lane_f32(arr[i], 2);
            lst[l*3 + i] = vgetq_lane_f32(arr[i], 3);
        }
        for (; i < l; i++) {
            lst[i] = vgetq_lane_f32(arr[i], 0);
            lst[l + i] = vgetq_lane_f32(arr[i], 1);
            lst[l*2 + i] = vgetq_lane_f32(arr[i], 2);
        }
    }

    free(arr);
}
