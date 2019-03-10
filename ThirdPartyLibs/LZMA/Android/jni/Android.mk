LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := lzma
LOCAL_SRC_FILES    := ../../lzma/C/7zStream.c ../../lzma/C/LzFind.c ../../lzma/C/LzmaDec.c ../../lzma/C/LzmaEnc.c
LOCAL_CFLAGS       := -D_7ZIP_ST -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPPFLAGS     := -D_7ZIP_ST -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPP_FEATURES := # rtti exceptions
LOCAL_ARM_NEON     := true # force NEON usage for all files

include $(BUILD_STATIC_LIBRARY)
