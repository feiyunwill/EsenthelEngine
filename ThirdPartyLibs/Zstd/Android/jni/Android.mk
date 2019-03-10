LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := zstd
LOCAL_CFLAGS       := -I../lib -I../lib/common -I../../LZ4 -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPPFLAGS     := -I../lib -I../lib/common -I../../LZ4 -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPP_FEATURES := # rtti exceptions
LOCAL_ARM_NEON     := true # force NEON usage for all files
LOCAL_SRC_FILES    := \
   ../../lib/common/entropy_common.c \
   ../../lib/common/error_private.c \
   ../../lib/common/fse_decompress.c \
   ../../lib/common/pool.c \
   ../../lib/common/threading.c \
   ../../lib/common/zstd_common.c \
   ../../lib/compress/fse_compress.c \
   ../../lib/compress/huf_compress.c \
   ../../lib/compress/zstd_compress.c \
   ../../lib/compress/zstd_double_fast.c \
   ../../lib/compress/zstd_fast.c \
   ../../lib/compress/zstd_lazy.c \
   ../../lib/compress/zstd_ldm.c \
   ../../lib/compress/zstd_opt.c \
   ../../lib/compress/zstdmt_compress.c \
   ../../lib/decompress/huf_decompress.c \
   ../../lib/decompress/zstd_decompress.c \
   ../../lib/dictBuilder/cover.c \
   ../../lib/dictBuilder/divsufsort.c \
   ../../lib/dictBuilder/zdict.c

include $(BUILD_STATIC_LIBRARY)
