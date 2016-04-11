/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <jni.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <cpu-features.h>
#include <math.h>
#include "helloneon-intrinsics.h"

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

void sqrtSerial(int N,
                float initialGuess,
                float values[],
                float output[])
{

    static const float kThreshold = 0.00001f;
    int i=0;

    for (i = 0; i<N; i++) {

        float x = values[i];
        float guess = initialGuess;

        float error = fabs(guess * guess * x - 1.f);

        while (error > kThreshold) {
            guess = (3.f * guess - x * guess * guess * guess) * 0.5f;
            error = fabs(guess * guess * x - 1.f);
        }

        output[i] = x * guess;
    }
}

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   apps/samples/hello-neon/project/src/com/example/neon/HelloNeon.java
 */
jstring
Java_com_example_xgzhu_myneonlibrary_NeonWrapper_stringFromJNI( JNIEnv* env,
                                               jobject thiz )
{
    char*  str;
    char buffer[512];
    AndroidCpuFamily family;
    uint64_t features;
    double  t0, t1, time_c, time_neon;

    #define N 2000
    const float initialGuess = 1.0f;

    float values[N];
    float output[N];
    float gold[N];

    /* setup input for sqrt - whatever */
    {
        unsigned int i=0;
        for (i = 0; i<N; i++)
        {
            // random input values
//            values[i] = 0.001f + 2.998f * rand() / RAND_MAX;
            values[i] = 2.888f;
            output[i] = 0.f;
        }

        // generate a gold version to check results
        for (i=0; i<N; i++)
            gold[i] = sqrt(values[i]);
    }

    /* Benchmark small FIR filter loop - C version */
    t0 = now_ms();
    {
        double minSerial = 1e30;
        unsigned int i=0;
        sqrtSerial(N, initialGuess, values, output);
    }
    t1 = now_ms();
    time_c = t1 - t0;

    asprintf(&str, "SQRT benchmark:\nC version          : %g ms\n", time_c);
    strlcpy(buffer, str, sizeof buffer);
    free(str);

    strlcat(buffer, "Neon version   : ", sizeof buffer);

    family = android_getCpuFamily();
    if ((family != ANDROID_CPU_FAMILY_ARM) &&
        (family != ANDROID_CPU_FAMILY_X86))
    {
        strlcat(buffer, "Not an ARM and not an X86 CPU !\n", sizeof buffer);
        goto EXIT;
    }

    features = android_getCpuFeatures();
    if (((features & ANDROID_CPU_ARM_FEATURE_ARMv7) == 0) &&
        ((features & ANDROID_CPU_X86_FEATURE_SSSE3) == 0))
    {
        strlcat(buffer, "Not an ARMv7 and not an X86 SSSE3 CPU !\n", sizeof buffer);
        goto EXIT;
    }

    /* HAVE_NEON is defined in Android.mk ! */
#ifdef HAVE_NEON
    if (((features & ANDROID_CPU_ARM_FEATURE_NEON) == 0) &&
        ((features & ANDROID_CPU_X86_FEATURE_SSSE3) == 0))
    {
        strlcat(buffer, "CPU doesn't support NEON !\n", sizeof buffer);
        goto EXIT;
    }

    /* Benchmark small FIR filter loop - Neon version */
    t0 = now_ms();
    {
        sqrt_neon_intrinsics(N, initialGuess, values, output);
    }
    t1 = now_ms();
    time_neon = t1 - t0;
    asprintf(&str, "%g ms (x%g faster)\n", time_neon, time_c / (time_neon < 1e-6 ? 1. : time_neon));
    strlcat(buffer, str, sizeof buffer);
    free(str);

    int error = 0, i;
    for (i=0; i<N; i++) {
        if (fabs(output[i] - gold[i]) > 1e-4) {
            D("neon[%d] = %d expected %d", i, output[i], gold[i]);
            error = 1;
        }
    }
    if (error == 1) {
            asprintf(&str, "ERROR\n");
            strlcat(buffer, str, sizeof buffer);
            free(str);
    }

#else /* !HAVE_NEON */
    strlcat(buffer, "Program not compiled with ARMv7 support !\n", sizeof buffer);
#endif /* !HAVE_NEON */
EXIT:
    return (*env)->NewStringUTF(env, buffer);
}

JNIEXPORT jstring JNICALL
Java_com_example_neon_HelloNeon_test(JNIEnv *env, jobject instance) {

    return (*env)->NewStringUTF(env, "HELLO MADAM.");
}

