//
// Created by chaoya on 4/17/16.
// all the sequential functions are implement directly in this file.
// all the intrinsics functions call neon-based functions in MatrixCalc-intrinsics.c
// notice: block-based multiply only contains "int" edition.

#include <jni.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <cpu-features.h>
#include "neon_matrix_intrinsics.h"
#include <android/log.h>

// block size for sequential multiply
#define BLOCK_M 32
#define BLOCK_N 32
#define BLOCK_K 32

/** mul int naive intrinsics **/
/*
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1int_1mul_1intrin(JNIEnv *env, jobject instance, jintArray a_,
                                                jint m_a, jint n_a, jintArray b_, jint m_b, jint n_b,
                                                jintArray result_) {
    #ifdef HAVE_NEON
    jint *a = (*env)->GetIntArrayElements(env, a_, NULL);
    jint *b = (*env)->GetIntArrayElements(env, b_, NULL);
    jint *result = (*env)->GetIntArrayElements(env, result_, NULL);

    matrix_int_mul(a, m_a, n_a, b, m_b, n_b, result);

    (*env)->ReleaseIntArrayElements(env, a_, a, 0);
    (*env)->ReleaseIntArrayElements(env, b_, b, 0);
    (*env)->ReleaseIntArrayElements(env, result_, result, 0);
#endif
}
*/

/** mul int naive sequential **/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1int_1mul_1sequential(JNIEnv *env, jobject instance, jintArray a_,
                                                               jint m_a, jint n_a, jintArray b_, jint m_b, jint n_b,
                                                               jintArray result_) {
#ifdef HAVE_NEON
    jint *a = (*env)->GetIntArrayElements(env, a_, NULL);
    jint *b = (*env)->GetIntArrayElements(env, b_, NULL);
    jint *result = (*env)->GetIntArrayElements(env, result_, NULL);

    //matrix_mul(a, m_a, n_a, b, m_b, n_b, result);

    int sum, i, j ,k;
    for (i = 0; i < m_a; i++) {
        for (j = 0; j < n_b; j++) {
            sum = 0;
            for (k = 0; k < n_a; k++) {
                //c[i][j] = a[][] + b[][];
                sum += a[i * n_a + k] * b[k * n_b + j];
            }
            result[i * n_b + j] = sum;
            //__android_log_print(ANDROID_LOG_VERBOSE, "sequential", "%d, %d, %d", i, j, sum);
        }
    }

    (*env)->ReleaseIntArrayElements(env, a_, a, 0);
    (*env)->ReleaseIntArrayElements(env, b_, b, 0);
    (*env)->ReleaseIntArrayElements(env, result_, result, 0);
#endif
}

/** mul int block sequential **/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1int_1mul_1block_1sequential(JNIEnv *env, jobject instance, jintArray a_,
                                                                      jint m_a, jint n_a, jintArray b_, jint m_b, jint n_b,
                                                                      jintArray result_) {
#ifdef HAVE_NEON
    jint *a = (*env)->GetIntArrayElements(env, a_, NULL);
    jint *b = (*env)->GetIntArrayElements(env, b_, NULL);
    jint *result = (*env)->GetIntArrayElements(env, result_, NULL);


    int sum, i, j ,k, iblock, jblock, kblock;

    for (iblock = 0; iblock < m_a; iblock += BLOCK_M)
        for (jblock = 0; jblock < n_b; jblock += BLOCK_N)
            for (kblock = 0; kblock < n_a; kblock += BLOCK_K)
                for (i = 0; i < BLOCK_M; i++) {
                    if (i + iblock >= m_a) {
                        break;
                    }
                    for (j = 0; j < BLOCK_N; j++) {
                        if (j + jblock >= n_b) {
                            break;
                        }
                        sum = 0;
                        for (k = 0; k < BLOCK_K; k++) {
                            if (k + kblock >= n_a) {
                                break;
                            }
                            sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + (j + jblock)];
                        }
                        result[(i + iblock) * n_b + (j + jblock)] += sum;
                    }
                }

    (*env)->ReleaseIntArrayElements(env, a_, a, 0);
    (*env)->ReleaseIntArrayElements(env, b_, b, 0);
    (*env)->ReleaseIntArrayElements(env, result_, result, 0);
#endif
}

/** mul int block intrinsics **/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1int_1mul_1block_1intrin1(JNIEnv *env, jobject instance, jintArray a_,
                                                                   jint m_a, jint n_a, jintArray b_, jint m_b, jint n_b,
                                                                   jintArray result_) {
#ifdef HAVE_NEON
    jint *a = (*env)->GetIntArrayElements(env, a_, NULL);
    jint *b = (*env)->GetIntArrayElements(env, b_, NULL);
    jint *result = (*env)->GetIntArrayElements(env, result_, NULL);

    matrix_int_mul_block(a, m_a, n_a, b, m_b, n_b, result);

    (*env)->ReleaseIntArrayElements(env, a_, a, 0);
    (*env)->ReleaseIntArrayElements(env, b_, b, 0);
    (*env)->ReleaseIntArrayElements(env, result_, result, 0);
#endif
}

/** mul float naive intrinsics**/
/*
JNIEXPORT void JNICALL
 Java_com_example_neon_NeonWrapper_matrix_1float_1mul_1intrin(JNIEnv *env, jobject instance, jfloatArray a_,
                                                jint m_a, jint n_a, jfloatArray b_, jint m_b, jint n_b,
                                                jfloatArray result_) {
 #ifdef HAVE_NEON
     jfloat *a = (*env)->GetFloatArrayElements(env, a_, NULL);
     jfloat *b = (*env)->GetFloatArrayElements(env, b_, NULL);
     jfloat *result = (*env)->GetFloatArrayElements(env, result_, NULL);

     matrix_float_mul(a, m_a, n_a, b, m_b, n_b, result);

     (*env)->ReleaseFloatArrayElements(env, a_, a, 0);
     (*env)->ReleaseFloatArrayElements(env, b_, b, 0);
     (*env)->ReleaseFloatArrayElements(env, result_, result, 0);
 #endif
 }
 */

/** mul float naive sequential**/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1float_1mul_1sequential(JNIEnv *env, jobject instance, jfloatArray a_,
                                                                 jint m_a, jint n_a, jfloatArray b_, jint m_b, jint n_b,
                                                                 jfloatArray result_) {
#ifdef HAVE_NEON
    jfloat *a = (*env)->GetFloatArrayElements(env, a_, NULL);
    jfloat *b = (*env)->GetFloatArrayElements(env, b_, NULL);
    jfloat *result = (*env)->GetFloatArrayElements(env, result_, NULL);

    //matrix_mul(a, m_a, n_a, b, m_b, n_b, result);

    int i, j ,k;
    float sum;
    for (i = 0; i < m_a; i++) {
        for (j = 0; j < n_b; j++) {
            sum = 0;
            for (k = 0; k < n_a; k++) {
                //c[i][j] = a[][] + b[][];
                sum += a[i * n_a + k] * b[k * n_b + j];
            }
            result[i * n_b + j] = sum;
            //__android_log_print(ANDROID_LOG_VERBOSE, "sequential", "%d, %d, %d", i, j, sum);
        }
    }

    (*env)->ReleaseFloatArrayElements(env, a_, a, 0);
    (*env)->ReleaseFloatArrayElements(env, b_, b, 0);
    (*env)->ReleaseFloatArrayElements(env, result_, result, 0);
#endif
}


/** mul float block sequential **/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1float_1mul_1block_1sequential(JNIEnv *env, jobject instance, jfloatArray a_,
                                                                        jint m_a, jint n_a, jfloatArray b_, jint m_b, jint n_b,
                                                                        jfloatArray result_) {
#ifdef HAVE_NEON
    jfloat *a = (*env)->GetFloatArrayElements(env, a_, NULL);
    jfloat *b = (*env)->GetFloatArrayElements(env, b_, NULL);
    jfloat *result = (*env)->GetFloatArrayElements(env, result_, NULL);


    int i, j ,k, iblock, jblock, kblock;
    float sum;


    for (iblock = 0; iblock < m_a; iblock += BLOCK_M)
        for (jblock = 0; jblock < n_b; jblock += BLOCK_N)
            for (kblock = 0; kblock < n_a; kblock += BLOCK_K)
                for (i = 0; i < BLOCK_M; i++) {
                    if (i + iblock >= m_a) {
                        break;
                    }
                    for (j = 0; j < BLOCK_N; j++) {
                        if (j + jblock >= n_b) {
                            break;
                        }
                        sum = 0;
                        for (k = 0; k < BLOCK_K; k++) {
                            if (k + kblock >= n_a) {
                                break;
                            }
                            sum += a[(i + iblock) * n_a + (k + kblock)] * b[(k + kblock) * n_b + (j + jblock)];
                        }
                        result[(i + iblock) * n_b + (j + jblock)] += sum;
                    }
                }

    (*env)->ReleaseFloatArrayElements(env, a_, a, 0);
    (*env)->ReleaseFloatArrayElements(env, b_, b, 0);
    (*env)->ReleaseFloatArrayElements(env, result_, result, 0);
#endif
}

/** mul float block intrinsics **/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1float_1mul_1block_1intrin1(JNIEnv *env, jobject instance, jfloatArray a_,
                                                                     jint m_a, jint n_a, jfloatArray b_, jint m_b, jint n_b,
                                                                     jfloatArray result_) {
#ifdef HAVE_NEON
    jfloat *a = (*env)->GetFloatArrayElements(env, a_, NULL);
    jfloat *b = (*env)->GetFloatArrayElements(env, b_, NULL);
    jfloat *result = (*env)->GetFloatArrayElements(env, result_, NULL);

    matrix_float_mul_block(a, m_a, n_a, b, m_b, n_b, result);

    (*env)->ReleaseFloatArrayElements(env, a_, a, 0);
    (*env)->ReleaseFloatArrayElements(env, b_, b, 0);
    (*env)->ReleaseFloatArrayElements(env, result_, result, 0);
#endif
}

/** mul float block intrinsics **/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1float_1mul_1block_1intrin2(JNIEnv *env, jobject instance, jfloatArray a_,
                                                                     jint m_a, jint n_a, jfloatArray b_, jint m_b, jint n_b,
                                                                     jfloatArray result_) {
#ifdef HAVE_NEON
    jfloat *a = (*env)->GetFloatArrayElements(env, a_, NULL);
    jfloat *b = (*env)->GetFloatArrayElements(env, b_, NULL);
    jfloat *result = (*env)->GetFloatArrayElements(env, result_, NULL);

    matrix_float_mul_block_v2(a, m_a, n_a, b, m_b, n_b, result);

    (*env)->ReleaseFloatArrayElements(env, a_, a, 0);
    (*env)->ReleaseFloatArrayElements(env, b_, b, 0);
    (*env)->ReleaseFloatArrayElements(env, result_, result, 0);
#endif
}

/** mul short naive intrinsics**/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1short_1mul_1intrin(JNIEnv *env, jobject instance, jshortArray a_,
                                                             jint m_a, jint n_a, jshortArray b_, jint m_b, jint n_b,
                                                             jshortArray result_) {
#ifdef HAVE_NEON
    jshort *a = (*env)->GetShortArrayElements(env, a_, NULL);
    jshort *b = (*env)->GetShortArrayElements(env, b_, NULL);
    jshort *result = (*env)->GetShortArrayElements(env, result_, NULL);

    matrix_short_mul(a, m_a, n_a, b, m_b, n_b, result);

    (*env)->ReleaseShortArrayElements(env, a_, a, 0);
    (*env)->ReleaseShortArrayElements(env, b_, b, 0);
    (*env)->ReleaseShortArrayElements(env, result_, result, 0);
#endif
}

/** mul float naive sequential**/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1short_1mul_1sequential(JNIEnv *env, jobject instance, jshortArray a_,
                                                                 jint m_a, jint n_a, jshortArray b_, jint m_b, jint n_b,
                                                                 jshortArray result_) {
#ifdef HAVE_NEON
    jshort *a = (*env)->GetShortArrayElements(env, a_, NULL);
    jshort *b = (*env)->GetShortArrayElements(env, b_, NULL);
    jshort *result = (*env)->GetShortArrayElements(env, result_, NULL);

    //matrix_mul(a, m_a, n_a, b, m_b, n_b, result);

    int i, j ,k;
    short sum;
    for (i = 0; i < m_a; i++) {
        for (j = 0; j < n_b; j++) {
            sum = 0;
            for (k = 0; k < n_a; k++) {
                //c[i][j] = a[][] + b[][];
                sum += a[i * n_a + k] * b[k * n_b + j];
            }
            result[i * n_b + j] = sum;
            //__android_log_print(ANDROID_LOG_VERBOSE, "sequential", "%d, %d, %d", i, j, sum);
        }
    }

    (*env)->ReleaseShortArrayElements(env, a_, a, 0);
    (*env)->ReleaseShortArrayElements(env, b_, b, 0);
    (*env)->ReleaseShortArrayElements(env, result_, result, 0);
#endif
}

/**************************************** transpose *********************************************/

/** transpose int intrinsic**/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1int_1transpose_1intrin(JNIEnv *env, jobject instance, jintArray a_,
                                                                 jint m_a, jint n_a, jintArray result_) {
#ifdef HAVE_NEON
    jint *a = (*env)->GetIntArrayElements(env, a_, NULL);
    jint *result = (*env)->GetIntArrayElements(env, result_, NULL);

    matrix_transpose_int_neon(a, m_a, n_a, result);

    (*env)->ReleaseIntArrayElements(env, a_, a, 0);
    (*env)->ReleaseIntArrayElements(env, result_, result, 0);
#endif
}

/** transpose int sequential**/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1int_1transpose_1sequential(JNIEnv *env, jobject instance, jintArray a_,
                                                                     jint m_a, jint n_a, jintArray result_) {
#ifdef HAVE_NEON
    jint *a = (*env)->GetIntArrayElements(env, a_, NULL);
    jint *result = (*env)->GetIntArrayElements(env, result_, NULL);

    int i, j;
    for (i = 0; i < m_a; i++) {
        for (j = 0;j < n_a; j++) {
            result[j * n_a + i] = a[i * m_a + j];
        }
    }

    (*env)->ReleaseIntArrayElements(env, a_, a, 0);
    (*env)->ReleaseIntArrayElements(env, result_, result, 0);
#endif
}

/** transpose float intrinsic**/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1float_1transpose_1intrin(JNIEnv *env, jobject instance, jfloatArray a_,
                                                                   jint m_a, jint n_a, jfloatArray result_) {
#ifdef HAVE_NEON
    jfloat *a = (*env)->GetFloatArrayElements(env, a_, NULL);
    jfloat *result = (*env)->GetFloatArrayElements(env, result_, NULL);

    matrix_transpose_float_neon(a, m_a, n_a, result);

    (*env)->ReleaseFloatArrayElements(env, a_, a, 0);
    (*env)->ReleaseFloatArrayElements(env, result_, result, 0);
#endif
}

/** transpose float sequential**/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1float_1transpose_1sequential(JNIEnv *env, jobject instance, jfloatArray a_,
                                                                       jint m_a, jint n_a, jfloatArray result_) {
#ifdef HAVE_NEON
    jfloat *a = (*env)->GetFloatArrayElements(env, a_, NULL);
    jfloat *result = (*env)->GetFloatArrayElements(env, result_, NULL);

    int i, j;
    for (i = 0; i < m_a; i++) {
        for (j = 0;j < n_a; j++) {
            result[j * n_a + i] = a[i * m_a + j];
        }
    }

    (*env)->ReleaseFloatArrayElements(env, a_, a, 0);
    (*env)->ReleaseFloatArrayElements(env, result_, result, 0);
#endif
}

/** mul int block intrinsics **/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1int_1add_1intrin(JNIEnv *env, jobject instance, jintArray a_,
                                                           jint m_a, jint n_a, jintArray b_, jint m_b, jint n_b,
                                                           jintArray result_) {
#ifdef HAVE_NEON
    jint *a = (*env)->GetIntArrayElements(env, a_, NULL);
    jint *b = (*env)->GetIntArrayElements(env, b_, NULL);
    jint *result = (*env)->GetIntArrayElements(env, result_, NULL);

    matrix_int_add(a, m_a, n_a, b, m_b, n_b, result);

    (*env)->ReleaseIntArrayElements(env, a_, a, 0);
    (*env)->ReleaseIntArrayElements(env, b_, b, 0);
    (*env)->ReleaseIntArrayElements(env, result_, result, 0);
#endif
}

/** mul float block intrinsics **/
JNIEXPORT void JNICALL
Java_com_example_neon_NeonWrapper_matrix_1float_1add_1intrin(JNIEnv *env, jobject instance, jfloatArray a_,
                                                             jint m_a, jint n_a, jfloatArray b_, jint m_b, jint n_b,
                                                             jfloatArray result_) {
#ifdef HAVE_NEON
    jfloat *a = (*env)->GetFloatArrayElements(env, a_, NULL);
    jfloat *b = (*env)->GetFloatArrayElements(env, b_, NULL);
    jfloat *result = (*env)->GetFloatArrayElements(env, result_, NULL);

    matrix_float_add(a, m_a, n_a, b, m_b, n_b, result);

    (*env)->ReleaseFloatArrayElements(env, a_, a, 0);
    (*env)->ReleaseFloatArrayElements(env, b_, b, 0);
    (*env)->ReleaseFloatArrayElements(env, result_, result, 0);
#endif
}