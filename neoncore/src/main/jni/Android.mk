LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := neoncore

LOCAL_SRC_FILES := neon_matrix.c neon_collection.c helper_set.c

ifeq ($(TARGET_ARCH_ABI),$(filter $(TARGET_ARCH_ABI), armeabi-v7a x86))
    LOCAL_CFLAGS := -DHAVE_NEON=1
ifeq ($(TARGET_ARCH_ABI),x86)
    LOCAL_CFLAGS += -mssse3
endif
    LOCAL_SRC_FILES += neon_matrix_intrinsics.c.neon neon_collection_intrinsics.c.neon neon_collection_intrinsics_sort.c.neon neon_collection_intrinsics_fft.c.neon
endif

LOCAL_STATIC_LIBRARIES := cpufeatures

LOCAL_LDLIBS := -llog

LOCAL_CFLAGS += -fopenmp
LOCAL_LDFLAGS += -fopenmp

include $(BUILD_SHARED_LIBRARY)

$(call import-module,cpufeatures)
