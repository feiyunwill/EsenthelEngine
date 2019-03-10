LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := jpeg
LOCAL_SRC_FILES    := ../../lib/jaricom.c ../../lib/jcapimin.c ../../lib/jcapistd.c ../../lib/jcarith.c ../../lib/jccoefct.c ../../lib/jccolor.c ../../lib/jcdctmgr.c ../../lib/jchuff.c ../../lib/jcinit.c ../../lib/jcmainct.c ../../lib/jcmarker.c ../../lib/jcmaster.c ../../lib/jcomapi.c ../../lib/jcparam.c ../../lib/jcprepct.c ../../lib/jcsample.c ../../lib/jctrans.c ../../lib/jdapimin.c ../../lib/jdapistd.c ../../lib/jdarith.c ../../lib/jdatadst.c ../../lib/jdatasrc.c ../../lib/jdcoefct.c ../../lib/jdcolor.c ../../lib/jddctmgr.c ../../lib/jdhuff.c ../../lib/jdinput.c ../../lib/jdmainct.c ../../lib/jdmarker.c ../../lib/jdmaster.c ../../lib/jdmerge.c ../../lib/jdpostct.c ../../lib/jdsample.c ../../lib/jdtrans.c ../../lib/jerror.c ../../lib/jfdctflt.c ../../lib/jfdctfst.c ../../lib/jfdctint.c ../../lib/jidctflt.c ../../lib/jidctfst.c ../../lib/jidctint.c ../../lib/jmemmgr.c ../../lib/jmemnobs.c ../../lib/jquant1.c ../../lib/jquant2.c ../../lib/jutils.c ../../lib/jidctred.c ../../lib/jcphuff.c ../../lib/jdphuff.c
LOCAL_CFLAGS       := -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPPFLAGS     := -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPP_FEATURES := exceptions # rtti
LOCAL_ARM_NEON     := true # force NEON usage for all files

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
   LOCAL_SRC_FILES += ../../lib/simd/jsimd_arm.c.neon ../../lib/simd/jsimd_arm_neon.S.neon
endif

ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
   LOCAL_SRC_FILES += ../../lib/simd/jsimd_arm64.c.neon ../../lib/simd/jsimd_arm64_neon.S.neon
endif

ifeq ($(TARGET_ARCH_ABI), x86)
   LOCAL_SRC_FILES += ../../lib/jsimd_none.c
endif

include $(BUILD_STATIC_LIBRARY)
