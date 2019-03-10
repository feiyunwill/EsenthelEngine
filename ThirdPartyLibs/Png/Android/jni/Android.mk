LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := png
LOCAL_SRC_FILES    := ../../src/png.c ../../src/pngerror.c ../../src/pngget.c ../../src/pngmem.c ../../src/pngpread.c ../../src/pngread.c ../../src/pngrio.c ../../src/pngrtran.c ../../src/pngrutil.c ../../src/pngset.c ../../src/pngtrans.c ../../src/pngwio.c ../../src/pngwrite.c ../../src/pngwtran.c ../../src/pngwutil.c
LOCAL_CFLAGS       := -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPPFLAGS     := -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPP_FEATURES := exceptions # rtti
LOCAL_ARM_NEON     := true # force NEON usage for all files

ifeq ($(TARGET_ARCH_ABI), $(filter $(TARGET_ARCH_ABI), armeabi-v7a arm64-v8a))
   LOCAL_SRC_FILES += \
      ../../src/arm/arm_init.c \
      ../../src/arm/filter_neon.S \
      ../../src/arm/filter_neon_intrinsics.c
endif

include $(BUILD_STATIC_LIBRARY)
