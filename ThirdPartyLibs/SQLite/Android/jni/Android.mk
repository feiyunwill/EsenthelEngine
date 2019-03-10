LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := sqlite
LOCAL_SRC_FILES    := ../../sqlite3.c
LOCAL_CFLAGS       := -fshort-wchar -fomit-frame-pointer
LOCAL_CPPFLAGS     := -fshort-wchar -fomit-frame-pointer
LOCAL_CPP_FEATURES := # rtti exceptions
LOCAL_ARM_NEON     := true # force NEON usage for all files

include $(BUILD_STATIC_LIBRARY)
