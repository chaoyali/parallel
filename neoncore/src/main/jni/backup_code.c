//
// Created by 朱晓光 on 5/3/16.
//



//    {
//        int i;
////        #pragma omp parallel for schedule(static)
//        int tmp[4];
//        for (i = 0; i < l - left; i++) {
//            vst1q_s32(tmp, arr[i]);
//            lst[i] = tmp[0];
//            lst[l + i] = tmp[1];
//            lst[l*2 + i] = tmp[2];
//            lst[l*3 + i] = tmp[3];
//        }
//        for (; i < l; i++) {
//            vst1q_s32(tmp, arr[i]);
//            lst[i] = tmp[0];
//            lst[l + i] = tmp[1];
//            lst[l*2 + i] = tmp[2];
//        }
//    }

//    {
//        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "SECONDHERE!!!!");
//        int i;
//        for (i = 0; i < len; i++) {
//            __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "ans[%d]=%d\t", i, ans[i]);
//

//
//jstring
//Java_com_example_xgzhu_nalib_NACollection_test(JNIEnv *env, jobject instance) {
//    char* str;
//    char buffer[512];
//    int array[N];
//    int arr_cp[N];
//    int i;
//    double  t0, t1, time_c, time_qs, time_neon;
//    AndroidCpuFamily family;
//    uint64_t features;
//
//    /* set initial value */
//    for (i = 0; i < N; i++) {
//        array[i] = (i*(i+N/2)) % (2*N+31);
//    }
//
//    /* Benchmark sort_AA serial loop - C version */
//    t0 = now_ms();
//    {
//        memcpy(arr_cp, array, sizeof(int) * N);
//        qsort(arr_cp, N, sizeof(int), cmpfunc);
//    }
//    t1 = now_ms();
//    time_qs = t1 - t0;
//    asprintf(&str, "SORT benchmark:\nQS version        : %g ms\n", time_qs);
//    strlcpy(buffer, str, sizeof buffer);
//    free(str);
//
//    /* Benchmark quick_sort serial loop - C version */
//    t0 = now_ms();
//    {
//        memcpy(arr_cp, array, sizeof(int) * N);
//        combsort(arr_cp, N);
//    }
//    t1 = now_ms();
//    time_c = t1 - t0;
//    asprintf(&str, "C version          : %g ms (x%g faster)\n", time_c, time_qs / (time_c < 1e-6 ? 1. : time_c));
//    strlcat(buffer, str, sizeof buffer);
//    free(str);
//
//    strlcat(buffer, "Neon version   : ", sizeof buffer);
//    family = android_getCpuFamily();
//    if ((family != ANDROID_CPU_FAMILY_ARM) &&
//        (family != ANDROID_CPU_FAMILY_X86))
//    {
//        strlcat(buffer, "Not an ARM and not an X86 CPU !\n", sizeof buffer);
//        goto EXIT;
//    }
//
//    features = android_getCpuFeatures();
//    if (((features & ANDROID_CPU_ARM_FEATURE_ARMv7) == 0) &&
//        ((features & ANDROID_CPU_X86_FEATURE_SSSE3) == 0))
//    {
//        strlcat(buffer, "Not an ARMv7 and not an X86 SSSE3 CPU !\n", sizeof buffer);
//        goto EXIT;
//    }
//
//#ifdef HAVE_NEON
//    if (((features & ANDROID_CPU_ARM_FEATURE_NEON) == 0) &&
//        ((features & ANDROID_CPU_X86_FEATURE_SSSE3) == 0))
//    {
//        strlcat(buffer, "CPU doesn't support NEON !\n", sizeof buffer);
//        goto EXIT;
//    }
//    t0 = now_ms();
////    {
////        int i;
////        #pragma omp parallel for schedule(guided, 1)
////        for ( i = 0; i < 2; i ++) {
////            combsort(array + N / 2 * i, N / 2);
//////            qsort(array + N / 2 * i, N / 2, sizeof(int), cmpfunc);
////        }
////    }
//    combsort_intrinsics(array, N);
//
//    t1 = now_ms();
//    time_neon = t1 - t0;
//    asprintf(&str, "%g ms (x%g faster)\n", time_neon, time_qs / (time_neon < 1e-6 ? 1. : time_neon));
//    strlcat(buffer, str, sizeof buffer);
//    free(str);
//
//    {
//        int error = 0, i;
//        for (i=0; i<N; i++) {
//            if (fabs(array[i] - arr_cp[i]) > 1e-4) {
//                D("neon[%d] = %d expected %d", i, array[i], arr_cp[i]);
//                error = 1;
//            }
//        }
//        if (error == 1) {
//                asprintf(&str, "ERROR\n");
//                strlcat(buffer, str, sizeof buffer);
//                free(str);
//        }
//    }
//
//#endif
//
//    EXIT:
//    return (*env)->NewStringUTF(env, buffer);
//}
//
//
//JNIEXPORT void JNICALL
//Java_com_example_xgzhu_nalib_NACollection_sort_1int(JNIEnv *env, jobject instance, jintArray array_,
//jint len) {
//jint *array = (*env)->GetIntArrayElements(env, array_, NULL);
//#ifdef HAVE_NEON
//combsort_intrinsics(array, len);
////    combsort(array, len);
////    qsort(array, len, sizeof(int), cmpfunc);
//#endif
//(*env)->ReleaseIntArrayElements(env, array_, array, 0);
//}