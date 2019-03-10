LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := theora
LOCAL_SRC_FILES    := ../../lib/bitpack.c ../../lib/decapiwrapper.c ../../lib/decinfo.c ../../lib/decode.c ../../lib/dequant.c ../../lib/encapiwrapper.c ../../lib/encfrag.c ../../lib/encinfo.c ../../lib/encode.c ../../lib/encoder_disabled.c ../../lib/enquant.c ../../lib/fdct.c ../../lib/fragment.c ../../lib/huffdec.c ../../lib/huffenc.c ../../lib/idct.c ../../lib/info.c ../../lib/internal.c ../../lib/mathops.c ../../lib/mcenc.c ../../lib/quant.c ../../lib/rate.c ../../lib/state.c ../../lib/tokenize.c ../../lib/analyze.c ../../lib/apiwrapper.c
LOCAL_CFLAGS       := -I../include -I../../lib -I../../Ogg/include -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPPFLAGS     := -I../include -I../../lib -I../../Ogg/include -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPP_FEATURES := # rtti exceptions
LOCAL_ARM_NEON     := true # force NEON usage for all files

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
   LOCAL_SRC_FILES += ../../lib/armbits-gnu.S.neon ../../lib/armfrag-gnu.S.neon ../../lib/armidct-gnu.S.neon ../../lib/armloop-gnu.S.neon ../../lib/arm/armstate.c ../../lib/arm/armcpu.c
   LOCAL_CFLAGS    += -DOC_ARM_ASM=1 -DOC_ARM_ASM_EDSP=1 -DOC_ARM_ASM_MEDIA=1 -DOC_ARM_ASM_NEON=1 -DOC_ARM_CAN_UNALIGN=0 -DOC_ARM_CAN_UNALIGN_LDRD=0
   LOCAL_CPPFLAGS  += -DOC_ARM_ASM=1 -DOC_ARM_ASM_EDSP=1 -DOC_ARM_ASM_MEDIA=1 -DOC_ARM_ASM_NEON=1 -DOC_ARM_CAN_UNALIGN=0 -DOC_ARM_CAN_UNALIGN_LDRD=0
endif

ifeq ($(TARGET_ARCH_ABI), x86)
   LOCAL_SRC_FILES += ../../lib/x86/mmxencfrag.c ../../lib/x86/mmxfdct.c ../../lib/x86/mmxfrag.c ../../lib/x86/mmxidct.c ../../lib/x86/mmxstate.c ../../lib/x86/sse2encfrag.c ../../lib/x86/sse2fdct.c ../../lib/x86/sse2idct.c ../../lib/x86/x86cpu.c ../../lib/x86/x86enc.c ../../lib/x86/x86enquant.c ../../lib/x86/x86state.c
   LOCAL_CFLAGS    += -DOC_X86_ASM=1
   LOCAL_CPPFLAGS  += -DOC_X86_ASM=1
endif

include $(BUILD_STATIC_LIBRARY)
