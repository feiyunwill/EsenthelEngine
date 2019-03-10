LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := flac
LOCAL_SRC_FILES    := \
   ../../lib/src/libFLAC/bitmath.c \
   ../../lib/src/libFLAC/bitreader.c \
   ../../lib/src/libFLAC/bitwriter.c \
   ../../lib/src/libFLAC/cpu.c \
   ../../lib/src/libFLAC/crc.c \
   ../../lib/src/libFLAC/fixed.c \
   ../../lib/src/libFLAC/float.c \
   ../../lib/src/libFLAC/format.c \
   ../../lib/src/libFLAC/lpc.c \
   ../../lib/src/libFLAC/md5.c \
   ../../lib/src/libFLAC/memory.c \
   ../../lib/src/libFLAC/metadata_iterators.c \
   ../../lib/src/libFLAC/metadata_object.c \
   ../../lib/src/libFLAC/ogg_decoder_aspect.c \
   ../../lib/src/libFLAC/ogg_encoder_aspect.c \
   ../../lib/src/libFLAC/ogg_helper.c \
   ../../lib/src/libFLAC/ogg_mapping.c \
   ../../lib/src/libFLAC/stream_decoder.c \
   ../../lib/src/libFLAC/stream_encoder.c \
   ../../lib/src/libFLAC/stream_encoder_framing.c \
   ../../lib/src/libFLAC/window.c
LOCAL_CFLAGS       := -DFLAC__NO_ASM -DHAVE_LROUND=1 -DFLAC__HAS_OGG -DPACKAGE_VERSION=\"1.3.2\" -I../lib/include -I../lib/src/libFLAC/include -I../../Ogg/include -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPPFLAGS     := -DFLAC__NO_ASM -DHAVE_LROUND=1 -DFLAC__HAS_OGG -DPACKAGE_VERSION=\"1.3.2\" -I../lib/include -I../lib/src/libFLAC/include -I../../Ogg/include -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPP_FEATURES := # rtti exceptions
LOCAL_ARM_NEON     := true # force NEON usage for all files

include $(BUILD_STATIC_LIBRARY)
