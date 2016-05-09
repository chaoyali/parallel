//
// Created by 朱晓光 on 4/14/16.
//


#include <jni.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cpu-features.h>
#include <android/log.h>
#include <omp.h>
#include "neon_collection_intrinsics.h"
#include "neon_collection_intrinsics_sort.h"
#include "neon_collection_intrinsics_fft.h"
#include "helper_set.h"

#define DEBUG 0

#if DEBUG
#include <android/log.h>
#  define  D(x...)  __android_log_print(ANDROID_LOG_INFO,"helloneon",x)
#else
#  define  D(...)  do {} while (0)
#endif

/* return current time in milliseconds */
static double
now_ms(void)
{
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0*res.tv_sec + (double)res.tv_nsec/1e6;
}

int cmpfunc_int (const void * a, const void * b)
{
    return ( *(int*)a - *(int*)b );
}

int cmpfunc_float (const void * a, const void * b)
{
    return ( *(float*)a - *(float*)b );
}

int cmpfunc_double (const void * a, const void * b)
{
    return ( *(double*)a - *(double*)b );
}

static int can_use_neon()
{
    AndroidCpuFamily family;
    uint64_t features;
    family = android_getCpuFamily();
    if ((family != ANDROID_CPU_FAMILY_ARM) &&
        (family != ANDROID_CPU_FAMILY_X86))
    {
        return 0;
    }
    features = android_getCpuFeatures();
    if (((features & ANDROID_CPU_ARM_FEATURE_ARMv7) == 0) &&
        ((features & ANDROID_CPU_X86_FEATURE_SSSE3) == 0))
    {
        return 0;
    }
#ifdef HAVE_NEON
    if (((features & ANDROID_CPU_ARM_FEATURE_NEON) == 0) &&
        ((features & ANDROID_CPU_X86_FEATURE_SSSE3) == 0))
    {
        return 0;
    }
#endif
    return 1;
}


void
Java_fastandroid_neoncore_collection_FaCollection_sort_1int(JNIEnv *env, jobject instance, jintArray array_,
                                                            jint len) {
    jint *array = (*env)->GetIntArrayElements(env, array_, NULL);
#ifdef HAVE_NEON
    if (can_use_neon()) {
        combsort_intrinsics_int(array, len);
    }
    else {
        qsort(array, len, sizeof(int), cmpfunc_int);
    }
#else
    qsort(array, len, sizeof(int), cmpfunc_int);
#endif

    (*env)->ReleaseIntArrayElements(env, array_, array, 0);
}

JNIEXPORT void JNICALL
Java_fastandroid_neoncore_collection_FaCollection_qsort_1int(JNIEnv *env, jclass type,
                                                             jintArray array_, jint len) {
    jint *array = (*env)->GetIntArrayElements(env, array_, NULL);

    qsort(array, len, sizeof(int), cmpfunc_int);

    (*env)->ReleaseIntArrayElements(env, array_, array, 0);
}

JNIEXPORT void JNICALL
Java_fastandroid_neoncore_collection_FaCollection_sort_1float(JNIEnv *env, jclass type, jfloatArray array_,
                                                              jint len) {
    jfloat *array = (*env)->GetFloatArrayElements(env, array_, NULL);

#ifdef HAVE_NEON
    if (can_use_neon()) {
        combsort_intrinsics_float(array, len);
    }
    else {
        qsort(array, len, sizeof(float), cmpfunc_float);
    }
#else
    qsort(array, len, sizeof(float), cmpfunc_float);
#endif

    (*env)->ReleaseFloatArrayElements(env, array_, array, 0);
}

JNIEXPORT void JNICALL
Java_fastandroid_neoncore_collection_FaCollection_qsort_1float(JNIEnv *env, jclass type, jfloatArray array_,
                                                              jint len) {
    jfloat *array = (*env)->GetFloatArrayElements(env, array_, NULL);

    qsort(array, len, sizeof(float), cmpfunc_float);

    (*env)->ReleaseFloatArrayElements(env, array_, array, 0);
}

JNIEXPORT void JNICALL
Java_fastandroid_neoncore_collection_FaCollection_qsort_1double(JNIEnv *env, jclass type,
                                                               jdoubleArray array_, jint len) {
    jdouble *array = (*env)->GetDoubleArrayElements(env, array_, NULL);

    qsort(array, len, sizeof(double), cmpfunc_double);

    (*env)->ReleaseDoubleArrayElements(env, array_, array, 0);
}


void
Java_fastandroid_neoncore_collection_FaCollection_fft_1float(JNIEnv *env, jobject instance,
                                                             jfloatArray real_, jfloatArray imag_,
                                                             jint len, jint reverse) {
    jfloat *real = (*env)->GetFloatArrayElements(env, real_, NULL);
    jfloat *imag = (*env)->GetFloatArrayElements(env, imag_, NULL);
    int done = 0;

#ifdef HAVE_NEON
    if (can_use_neon()) {
        fft_intrinsics_float(real, imag, len, reverse);
        done = 1;
    }
#endif

    if (!done) {
        fft_float(real,imag, len, reverse);
    }

    (*env)->ReleaseFloatArrayElements(env, real_, real, 0);
    (*env)->ReleaseFloatArrayElements(env, imag_, imag, 0);
}


JNIEXPORT void JNICALL
Java_fastandroid_neoncore_collection_FaCollection_vector_1int(JNIEnv *env, jclass type, jintArray x_,
                                                              jint len, jintArray var_, jint num_var) {
    jint *x = (*env)->GetIntArrayElements(env, x_, NULL);
    jint *var = (*env)->GetIntArrayElements(env, var_, NULL);
    int done = 0;

#ifdef HAVE_NEON
    if (can_use_neon()) {
        vector_intrinsics_int(x, len, var, num_var);
        done = 1;
    }
#endif

    if (!done) {
        vector_int(x, len, var, num_var);
    }

    (*env)->ReleaseIntArrayElements(env, x_, x, 0);
    (*env)->ReleaseIntArrayElements(env, var_, var, 0);
}

JNIEXPORT void JNICALL
Java_fastandroid_neoncore_collection_FaCollection_fft_1float_1c(JNIEnv *env, jclass type,
                                                                jfloatArray real_,
                                                                jfloatArray imag_, jint len,
                                                                jint reverse) {
    jfloat *real = (*env)->GetFloatArrayElements(env, real_, NULL);
    jfloat *imag = (*env)->GetFloatArrayElements(env, imag_, NULL);

    fft_float(real, imag, len, reverse);

    (*env)->ReleaseFloatArrayElements(env, real_, real, 0);
    (*env)->ReleaseFloatArrayElements(env, imag_, imag, 0);
}

JNIEXPORT void JNICALL
Java_fastandroid_neoncore_collection_FaCollection_sort_1int_1c(JNIEnv *env, jclass type,
                                                               jintArray array_, jint len) {
    jint *array = (*env)->GetIntArrayElements(env, array_, NULL);

    combsort(array, len);

    (*env)->ReleaseIntArrayElements(env, array_, array, 0);
}

JNIEXPORT void JNICALL
Java_fastandroid_neoncore_collection_FaCollection_vector_1float(JNIEnv *env, jclass type,
                                                                jfloatArray x_, jint len,
                                                                jfloatArray var_, jint num_var) {
    jfloat *x = (*env)->GetFloatArrayElements(env, x_, NULL);
    jfloat *var = (*env)->GetFloatArrayElements(env, var_, NULL);
    int done = 0;
#ifdef HAVE_NEON
    vector_intrinsics_float(x, len, var, num_var);
#endif
    if (!done) {
        vector_float(x, len, var, num_var);
    }
    (*env)->ReleaseFloatArrayElements(env, x_, x, 0);
    (*env)->ReleaseFloatArrayElements(env, var_, var, 0);
}



/////////////// Following are test code.


JNIEXPORT jstring JNICALL
Java_fastandroid_neoncore_collection_FaCollection_fft_1float_1test(JNIEnv *env, jobject instance,
                                                                   jfloatArray real_, jfloatArray imag_,
                                                                   jint len) {
    int i;
    char* str;

    if (!can_use_neon()) {
        char buffer[1024] = "No neon available in this device. But you can still use our api.\n";
        return (*env)->NewStringUTF(env, buffer);
    }
    char buffer[1024] = "hello fft\n";
#ifdef HAVE_NEON
    for (i = 10; i < 20; i++) {

        asprintf(&str, "length: 2^%d.\n", i);
        strlcat(buffer, str, sizeof(buffer));
        free(str);

        int len = 1<<i, iter, iter_num = 5;
        double t0, t1, t_neon = 0, t_ser = 0;
        float* r1 = (float *) malloc(len * sizeof(float));
        float* r2 = (float *) malloc(len * sizeof(float));
        float* i1 = (float *) malloc(len * sizeof(float));
        float* i2 = (float *) malloc(len * sizeof(float));
        for (iter = 0; iter < iter_num; iter++) {
            int j;
            for (j = 0; j < len; j++) {
                r1[j] = (float)cos(M_PI * j / 10);
                r2[j] = r1[j];
                i1[j] = 0;
                i2[j] = 0;
            }

            t0 = now_ms();
            fft_float(r1, i1, len, 0);
            t1 = now_ms();
            t_ser += t1 - t0;

            t0 = now_ms();
            fft_intrinsics_float(r2, i2, len, 0);
            t1 = now_ms();
            t_neon += t1 - t0;

            for (j = 0; j < len; j++) {
                if (r1[j] > r2[j] + 0.01 || r1[j] < r2[j] - 0.01) {
                    asprintf(&str, "ERROR!! at %d, %g != %g\n", j, r1[j], r2[j]);
                    strlcat(buffer, str, sizeof(buffer));
                    free(str);
                    break;
                }
            }
        }
        free(r1);
        free(r2);
        free(i1);
        free(i2);
        asprintf(&str, "c version: %g ms.\n", t_ser/iter_num);
        strlcat(buffer, str, sizeof(buffer));
        free(str);
        asprintf(&str, "neon version: %g ms (x%.3g faster than c).\n", t_neon/iter_num, t_ser/t_neon);
        strlcat(buffer, str, sizeof(buffer));
        free(str);
    }
#endif

    return (*env)->NewStringUTF(env, buffer);
}

JNIEXPORT jstring JNICALL
Java_fastandroid_neoncore_collection_FaCollection_sort_1int_1test(JNIEnv *env, jclass type) {

    int i;
    char* str;
    if (!can_use_neon()) {
        char buffer[1024] = "No neon available in this device. But you can still use our api.\n";
        return (*env)->NewStringUTF(env, buffer);
    }
    char buffer[1024] = "hello sort\n";
#ifdef HAVE_NEON
    for (i = 18; i < 23; i++) {

        asprintf(&str, "length: 2^%d.\n", i);
        strlcat(buffer, str, sizeof(buffer));
        free(str);
        double t0, t1, t_neon=0, t_ser=0, t_qs=0;

        int len = 1<<i;
        int* a1 = (int *) malloc(len * sizeof(int));
        int* a2 = (int *) malloc(len * sizeof(int));
        int* a3 = (int *) malloc(len * sizeof(int));
        int j, iter, iter_num = 4;
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "FirstHERE!!!!");

        for (iter = 0; iter < iter_num; iter++) {
            for (j = 0; j < len; j++) {
                a1[j] = (j + len / 2) * (j % 77) - (j % 111) * (j - len/3);
                a2[j] = a1[j];
                a3[j] = a1[j];
            }


            t0 = now_ms();
            qsort(a1, len, sizeof(int), cmpfunc_int);
            t1 = now_ms();
            t_qs += t1 - t0;

            t0 = now_ms();
            combsort(a2, len);
            t1 = now_ms();
            t_ser += t1 - t0;

            t0 = now_ms();
            combsort_intrinsics_int(a3, len);
            t1 = now_ms();
            t_neon += t1 - t0;

            for (j = 0; j < len; j++) {
                if (a1[j] != a3[j]) {
                    asprintf(&str, "ERROR!! at %d, %d != %d\n", j, a1[j], a3[j]);
                    strlcat(buffer, str, sizeof(buffer));
                    free(str);
                    break;
                }
            }
        }
        asprintf(&str, "qs version: %g ms.\t", t_qs);
        strlcat(buffer, str, sizeof(buffer));
        free(str);
        asprintf(&str, "c version: %g ms.\n", t_ser);
        strlcat(buffer, str, sizeof(buffer));
        free(str);
        asprintf(&str, "neon version: %g ms (x%.3g faster than c, x%.3g faster than qs).\n", t_neon, t_ser/t_neon, t_qs/t_neon);
        strlcat(buffer, str, sizeof(buffer));
        free(str);

//        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "SECONDHERE!!!!");

        free(a1);
        free(a2);
        free(a3);
    }
#endif

    return (*env)->NewStringUTF(env, buffer);
}


JNIEXPORT jstring JNICALL
Java_fastandroid_neoncore_collection_FaCollection_vector_1int_1test(JNIEnv *env, jclass type) {

    int i;
    char* str;
    if (!can_use_neon()) {
        char buffer[1024] = "No neon available in this device. But you can still use our api.\n";
        return (*env)->NewStringUTF(env, buffer);
    }
    char buffer[1024] = "hello vector\n";
#ifdef HAVE_NEON
    for (i = 0; i < 50; i+=5) {

        asprintf(&str, "var_num length: %d.\n", i);
        strlcat(buffer, str, sizeof(buffer));
        free(str);

        int len = 1<<14;
        int num_var = i;
        int* var = (int *) malloc(num_var * sizeof(int));

        double t0, t1, t_neon = 0, t_ser = 0;

        int* a1 = (int *) malloc(len * sizeof(int));
        int* a2 = (int *) malloc(len * sizeof(int));

        int j, iter, iter_len = 20;
        for (iter = 0; iter < iter_len; iter ++) {
            for (j = 0; j < len; j++) {
                a1[j] = j + (j * iter) % 77;
                a2[j] = a1[j];
            }
            for (j = 0; j < num_var; j++) {
                var[j] = num_var - j + 1;
            }

            t0 = now_ms();
            vector_int(a1, len, var, num_var);
            t1 = now_ms();
            t_ser += t1 - t0;


            t0 = now_ms();
            vector_intrinsics_int(a2, len, var, num_var);
            t1 = now_ms();
            t_neon += t1 - t0;

            for (j = 0; j < len; j++) {
                if (a1[j] != a2[j]) {
                    asprintf(&str, "ERROR!! at %d, %d != %d\n", j, a1[j], a2[j]);
                    strlcat(buffer, str, sizeof(buffer));
                    free(str);
                    break;
                }
            }
        }
        asprintf(&str, "c version: %g ms.\nneon version: %g ms (x%.3g faster than c).\n", t_ser/iter_len, t_neon/iter_len, t_ser/t_neon);
        strlcat(buffer, str, sizeof(buffer));
        free(str);

        free(a1);
        free(a2);
        free(var);
    }
#endif

    return (*env)->NewStringUTF(env, buffer);
}