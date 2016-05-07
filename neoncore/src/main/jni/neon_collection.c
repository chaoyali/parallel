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

    qsort(array, len, sizeof(float), cmpfunc_float);

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


JNIEXPORT jstring JNICALL
Java_fastandroid_neoncore_collection_FaCollection_fft_1float_1test(JNIEnv *env, jobject instance,
                                                                   jfloatArray real_, jfloatArray imag_,
                                                                   jint len) {
    jfloat *real = (*env)->GetFloatArrayElements(env, real_, NULL);
    jfloat *imag = (*env)->GetFloatArrayElements(env, imag_, NULL);

    double t0, t1, t_neon, t_ser;

    t0 = now_ms();

    fft_float(real, imag, len, 0);

    t1 = now_ms();
    t_ser = t1 - t0;


#ifdef HAVE_NEON
    t0 = now_ms();
    fft_intrinsics_float(real, imag, len, 0);
    t1 = now_ms();
    t_neon = t1 - t0;
#endif

    (*env)->ReleaseFloatArrayElements(env, real_, real, 0);
    (*env)->ReleaseFloatArrayElements(env, imag_, imag, 0);

    char buffer[512] = "hello fft\n";
    char* str;
    asprintf(&str, "%g ms neon vs %g ms serial (x%g faster)\n", t_neon, t_ser, t_ser / (t_neon < 1e-6 ? 1. : t_neon));
    strlcat(buffer, str, sizeof buffer);
    free(str);
    return (*env)->NewStringUTF(env, buffer);
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
