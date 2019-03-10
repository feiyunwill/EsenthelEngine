LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := brotli
LOCAL_SRC_FILES    := \
   ../../lib/common/dictionary.c \
   ../../lib/dec/bit_reader.c \
   ../../lib/dec/decode.c \
   ../../lib/dec/huffman.c \
   ../../lib/dec/state.c \
   ../../lib/enc/backward_references.c \
   ../../lib/enc/bit_cost.c \
   ../../lib/enc/block_splitter.c \
   ../../lib/enc/brotli_bit_stream.c \
   ../../lib/enc/cluster.c \
   ../../lib/enc/compress_fragment.c \
   ../../lib/enc/compress_fragment_two_pass.c \
   ../../lib/enc/encode.c \
   ../../lib/enc/entropy_encode.c \
   ../../lib/enc/histogram.c \
   ../../lib/enc/literal_cost.c \
   ../../lib/enc/memory.c \
   ../../lib/enc/metablock.c \
   ../../lib/enc/static_dict.c \
   ../../lib/enc/utf8_util.c
LOCAL_CFLAGS       := -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPPFLAGS     := -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPP_FEATURES := # rtti exceptions
LOCAL_ARM_NEON     := true # force NEON usage for all files

include $(BUILD_STATIC_LIBRARY)
