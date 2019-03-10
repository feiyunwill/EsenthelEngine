LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := ogg
LOCAL_SRC_FILES    := ../../src/bitwise.c ../../src/framing.c
LOCAL_CFLAGS       := -I../include -fshort-wchar -O3 -ffast-math -fomit-frame-pointer
LOCAL_CPPFLAGS     := -I../include -fshort-wchar -O3 -ffast-math -fomit-frame-pointer
LOCAL_CPP_FEATURES := # rtti exceptions
LOCAL_ARM_NEON     := true # force NEON usage for all files

include $(BUILD_STATIC_LIBRARY)
