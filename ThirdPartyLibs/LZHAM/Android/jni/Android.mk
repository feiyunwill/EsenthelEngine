LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := lzham
LOCAL_SRC_FILES    := \
   ../../Source/lzham_assert.cpp \
   ../../Source/lzham_checksum.cpp \
   ../../Source/lzham_huffman_codes.cpp \
   ../../Source/lzham_lzbase.cpp \
   ../../Source/lzham_lzcomp.cpp \
   ../../Source/lzham_lzcomp_internal.cpp \
   ../../Source/lzham_lzcomp_state.cpp \
   ../../Source/lzham_lzdecomp.cpp \
   ../../Source/lzham_lzdecompbase.cpp \
   ../../Source/lzham_match_accel.cpp \
   ../../Source/lzham_mem.cpp \
   ../../Source/lzham_platform.cpp \
   ../../Source/lzham_prefix_coding.cpp \
   ../../Source/lzham_pthreads_threading.cpp \
   ../../Source/lzham_symbol_codec.cpp \
   ../../Source/lzham_timer.cpp \
   ../../Source/lzham_vector.cpp \
   ../../Source/lzham_win32_threading.cpp
LOCAL_CFLAGS       := -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPPFLAGS     := -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPP_FEATURES := # rtti exceptions
LOCAL_ARM_NEON     := true # force NEON usage for all files

include $(BUILD_STATIC_LIBRARY)
