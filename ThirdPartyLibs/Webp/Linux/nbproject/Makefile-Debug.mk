#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/27549008/alpha_dec.o \
	${OBJECTDIR}/_ext/27549008/buffer_dec.o \
	${OBJECTDIR}/_ext/27549008/frame_dec.o \
	${OBJECTDIR}/_ext/27549008/idec_dec.o \
	${OBJECTDIR}/_ext/27549008/io_dec.o \
	${OBJECTDIR}/_ext/27549008/quant_dec.o \
	${OBJECTDIR}/_ext/27549008/tree_dec.o \
	${OBJECTDIR}/_ext/27549008/vp8_dec.o \
	${OBJECTDIR}/_ext/27549008/vp8l_dec.o \
	${OBJECTDIR}/_ext/27549008/webp_dec.o \
	${OBJECTDIR}/_ext/a470e235/anim_decode.o \
	${OBJECTDIR}/_ext/a470e235/demux.o \
	${OBJECTDIR}/_ext/275491c7/alpha_processing.o \
	${OBJECTDIR}/_ext/275491c7/alpha_processing_mips_dsp_r2.o \
	${OBJECTDIR}/_ext/275491c7/alpha_processing_neon.o \
	${OBJECTDIR}/_ext/275491c7/alpha_processing_sse2.o \
	${OBJECTDIR}/_ext/275491c7/alpha_processing_sse41.o \
	${OBJECTDIR}/_ext/275491c7/argb.o \
	${OBJECTDIR}/_ext/275491c7/argb_mips_dsp_r2.o \
	${OBJECTDIR}/_ext/275491c7/argb_sse2.o \
	${OBJECTDIR}/_ext/275491c7/cost.o \
	${OBJECTDIR}/_ext/275491c7/cost_mips32.o \
	${OBJECTDIR}/_ext/275491c7/cost_mips_dsp_r2.o \
	${OBJECTDIR}/_ext/275491c7/cost_sse2.o \
	${OBJECTDIR}/_ext/275491c7/cpu.o \
	${OBJECTDIR}/_ext/275491c7/dec.o \
	${OBJECTDIR}/_ext/275491c7/dec_clip_tables.o \
	${OBJECTDIR}/_ext/275491c7/dec_mips32.o \
	${OBJECTDIR}/_ext/275491c7/dec_mips_dsp_r2.o \
	${OBJECTDIR}/_ext/275491c7/dec_msa.o \
	${OBJECTDIR}/_ext/275491c7/dec_neon.o \
	${OBJECTDIR}/_ext/275491c7/dec_sse2.o \
	${OBJECTDIR}/_ext/275491c7/dec_sse41.o \
	${OBJECTDIR}/_ext/275491c7/enc.o \
	${OBJECTDIR}/_ext/275491c7/enc_avx2.o \
	${OBJECTDIR}/_ext/275491c7/enc_mips32.o \
	${OBJECTDIR}/_ext/275491c7/enc_mips_dsp_r2.o \
	${OBJECTDIR}/_ext/275491c7/enc_msa.o \
	${OBJECTDIR}/_ext/275491c7/enc_neon.o \
	${OBJECTDIR}/_ext/275491c7/enc_sse2.o \
	${OBJECTDIR}/_ext/275491c7/enc_sse41.o \
	${OBJECTDIR}/_ext/275491c7/filters.o \
	${OBJECTDIR}/_ext/275491c7/filters_mips_dsp_r2.o \
	${OBJECTDIR}/_ext/275491c7/filters_msa.o \
	${OBJECTDIR}/_ext/275491c7/filters_neon.o \
	${OBJECTDIR}/_ext/275491c7/filters_sse2.o \
	${OBJECTDIR}/_ext/275491c7/lossless.o \
	${OBJECTDIR}/_ext/275491c7/lossless_enc.o \
	${OBJECTDIR}/_ext/275491c7/lossless_enc_mips32.o \
	${OBJECTDIR}/_ext/275491c7/lossless_enc_mips_dsp_r2.o \
	${OBJECTDIR}/_ext/275491c7/lossless_enc_msa.o \
	${OBJECTDIR}/_ext/275491c7/lossless_enc_neon.o \
	${OBJECTDIR}/_ext/275491c7/lossless_enc_sse2.o \
	${OBJECTDIR}/_ext/275491c7/lossless_enc_sse41.o \
	${OBJECTDIR}/_ext/275491c7/lossless_mips_dsp_r2.o \
	${OBJECTDIR}/_ext/275491c7/lossless_msa.o \
	${OBJECTDIR}/_ext/275491c7/lossless_neon.o \
	${OBJECTDIR}/_ext/275491c7/lossless_sse2.o \
	${OBJECTDIR}/_ext/275491c7/rescaler.o \
	${OBJECTDIR}/_ext/275491c7/rescaler_mips32.o \
	${OBJECTDIR}/_ext/275491c7/rescaler_mips_dsp_r2.o \
	${OBJECTDIR}/_ext/275491c7/rescaler_msa.o \
	${OBJECTDIR}/_ext/275491c7/rescaler_neon.o \
	${OBJECTDIR}/_ext/275491c7/rescaler_sse2.o \
	${OBJECTDIR}/_ext/275491c7/ssim.o \
	${OBJECTDIR}/_ext/275491c7/ssim_sse2.o \
	${OBJECTDIR}/_ext/275491c7/upsampling.o \
	${OBJECTDIR}/_ext/275491c7/upsampling_mips_dsp_r2.o \
	${OBJECTDIR}/_ext/275491c7/upsampling_msa.o \
	${OBJECTDIR}/_ext/275491c7/upsampling_neon.o \
	${OBJECTDIR}/_ext/275491c7/upsampling_sse2.o \
	${OBJECTDIR}/_ext/275491c7/yuv.o \
	${OBJECTDIR}/_ext/275491c7/yuv_mips32.o \
	${OBJECTDIR}/_ext/275491c7/yuv_mips_dsp_r2.o \
	${OBJECTDIR}/_ext/275491c7/yuv_sse2.o \
	${OBJECTDIR}/_ext/275494e0/alpha_enc.o \
	${OBJECTDIR}/_ext/275494e0/analysis_enc.o \
	${OBJECTDIR}/_ext/275494e0/backward_references_enc.o \
	${OBJECTDIR}/_ext/275494e0/config_enc.o \
	${OBJECTDIR}/_ext/275494e0/cost_enc.o \
	${OBJECTDIR}/_ext/275494e0/delta_palettization_enc.o \
	${OBJECTDIR}/_ext/275494e0/filter_enc.o \
	${OBJECTDIR}/_ext/275494e0/frame_enc.o \
	${OBJECTDIR}/_ext/275494e0/histogram_enc.o \
	${OBJECTDIR}/_ext/275494e0/iterator_enc.o \
	${OBJECTDIR}/_ext/275494e0/near_lossless_enc.o \
	${OBJECTDIR}/_ext/275494e0/picture_csp_enc.o \
	${OBJECTDIR}/_ext/275494e0/picture_enc.o \
	${OBJECTDIR}/_ext/275494e0/picture_psnr_enc.o \
	${OBJECTDIR}/_ext/275494e0/picture_rescale_enc.o \
	${OBJECTDIR}/_ext/275494e0/picture_tools_enc.o \
	${OBJECTDIR}/_ext/275494e0/predictor_enc.o \
	${OBJECTDIR}/_ext/275494e0/quant_enc.o \
	${OBJECTDIR}/_ext/275494e0/syntax_enc.o \
	${OBJECTDIR}/_ext/275494e0/token_enc.o \
	${OBJECTDIR}/_ext/275494e0/tree_enc.o \
	${OBJECTDIR}/_ext/275494e0/vp8l_enc.o \
	${OBJECTDIR}/_ext/275494e0/webp_enc.o \
	${OBJECTDIR}/_ext/2754b3d6/anim_encode.o \
	${OBJECTDIR}/_ext/2754b3d6/muxedit.o \
	${OBJECTDIR}/_ext/2754b3d6/muxinternal.o \
	${OBJECTDIR}/_ext/2754b3d6/muxread.o \
	${OBJECTDIR}/_ext/a5673337/bit_reader_utils.o \
	${OBJECTDIR}/_ext/a5673337/bit_writer_utils.o \
	${OBJECTDIR}/_ext/a5673337/color_cache_utils.o \
	${OBJECTDIR}/_ext/a5673337/filters_utils.o \
	${OBJECTDIR}/_ext/a5673337/huffman_encode_utils.o \
	${OBJECTDIR}/_ext/a5673337/huffman_utils.o \
	${OBJECTDIR}/_ext/a5673337/quant_levels_dec_utils.o \
	${OBJECTDIR}/_ext/a5673337/quant_levels_utils.o \
	${OBJECTDIR}/_ext/a5673337/random_utils.o \
	${OBJECTDIR}/_ext/a5673337/rescaler_utils.o \
	${OBJECTDIR}/_ext/a5673337/thread_utils.o \
	${OBJECTDIR}/_ext/a5673337/utils.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblinux.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblinux.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblinux.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblinux.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblinux.a

${OBJECTDIR}/_ext/27549008/alpha_dec.o: ../src/dec/alpha_dec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/27549008
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/27549008/alpha_dec.o ../src/dec/alpha_dec.c

${OBJECTDIR}/_ext/27549008/buffer_dec.o: ../src/dec/buffer_dec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/27549008
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/27549008/buffer_dec.o ../src/dec/buffer_dec.c

${OBJECTDIR}/_ext/27549008/frame_dec.o: ../src/dec/frame_dec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/27549008
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/27549008/frame_dec.o ../src/dec/frame_dec.c

${OBJECTDIR}/_ext/27549008/idec_dec.o: ../src/dec/idec_dec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/27549008
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/27549008/idec_dec.o ../src/dec/idec_dec.c

${OBJECTDIR}/_ext/27549008/io_dec.o: ../src/dec/io_dec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/27549008
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/27549008/io_dec.o ../src/dec/io_dec.c

${OBJECTDIR}/_ext/27549008/quant_dec.o: ../src/dec/quant_dec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/27549008
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/27549008/quant_dec.o ../src/dec/quant_dec.c

${OBJECTDIR}/_ext/27549008/tree_dec.o: ../src/dec/tree_dec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/27549008
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/27549008/tree_dec.o ../src/dec/tree_dec.c

${OBJECTDIR}/_ext/27549008/vp8_dec.o: ../src/dec/vp8_dec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/27549008
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/27549008/vp8_dec.o ../src/dec/vp8_dec.c

${OBJECTDIR}/_ext/27549008/vp8l_dec.o: ../src/dec/vp8l_dec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/27549008
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/27549008/vp8l_dec.o ../src/dec/vp8l_dec.c

${OBJECTDIR}/_ext/27549008/webp_dec.o: ../src/dec/webp_dec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/27549008
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/27549008/webp_dec.o ../src/dec/webp_dec.c

${OBJECTDIR}/_ext/a470e235/anim_decode.o: ../src/demux/anim_decode.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a470e235
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a470e235/anim_decode.o ../src/demux/anim_decode.c

${OBJECTDIR}/_ext/a470e235/demux.o: ../src/demux/demux.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a470e235
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a470e235/demux.o ../src/demux/demux.c

${OBJECTDIR}/_ext/275491c7/alpha_processing.o: ../src/dsp/alpha_processing.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/alpha_processing.o ../src/dsp/alpha_processing.c

${OBJECTDIR}/_ext/275491c7/alpha_processing_mips_dsp_r2.o: ../src/dsp/alpha_processing_mips_dsp_r2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/alpha_processing_mips_dsp_r2.o ../src/dsp/alpha_processing_mips_dsp_r2.c

${OBJECTDIR}/_ext/275491c7/alpha_processing_neon.o: ../src/dsp/alpha_processing_neon.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/alpha_processing_neon.o ../src/dsp/alpha_processing_neon.c

${OBJECTDIR}/_ext/275491c7/alpha_processing_sse2.o: ../src/dsp/alpha_processing_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/alpha_processing_sse2.o ../src/dsp/alpha_processing_sse2.c

${OBJECTDIR}/_ext/275491c7/alpha_processing_sse41.o: ../src/dsp/alpha_processing_sse41.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/alpha_processing_sse41.o ../src/dsp/alpha_processing_sse41.c

${OBJECTDIR}/_ext/275491c7/argb.o: ../src/dsp/argb.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/argb.o ../src/dsp/argb.c

${OBJECTDIR}/_ext/275491c7/argb_mips_dsp_r2.o: ../src/dsp/argb_mips_dsp_r2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/argb_mips_dsp_r2.o ../src/dsp/argb_mips_dsp_r2.c

${OBJECTDIR}/_ext/275491c7/argb_sse2.o: ../src/dsp/argb_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/argb_sse2.o ../src/dsp/argb_sse2.c

${OBJECTDIR}/_ext/275491c7/cost.o: ../src/dsp/cost.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/cost.o ../src/dsp/cost.c

${OBJECTDIR}/_ext/275491c7/cost_mips32.o: ../src/dsp/cost_mips32.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/cost_mips32.o ../src/dsp/cost_mips32.c

${OBJECTDIR}/_ext/275491c7/cost_mips_dsp_r2.o: ../src/dsp/cost_mips_dsp_r2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/cost_mips_dsp_r2.o ../src/dsp/cost_mips_dsp_r2.c

${OBJECTDIR}/_ext/275491c7/cost_sse2.o: ../src/dsp/cost_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/cost_sse2.o ../src/dsp/cost_sse2.c

${OBJECTDIR}/_ext/275491c7/cpu.o: ../src/dsp/cpu.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/cpu.o ../src/dsp/cpu.c

${OBJECTDIR}/_ext/275491c7/dec.o: ../src/dsp/dec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/dec.o ../src/dsp/dec.c

${OBJECTDIR}/_ext/275491c7/dec_clip_tables.o: ../src/dsp/dec_clip_tables.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/dec_clip_tables.o ../src/dsp/dec_clip_tables.c

${OBJECTDIR}/_ext/275491c7/dec_mips32.o: ../src/dsp/dec_mips32.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/dec_mips32.o ../src/dsp/dec_mips32.c

${OBJECTDIR}/_ext/275491c7/dec_mips_dsp_r2.o: ../src/dsp/dec_mips_dsp_r2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/dec_mips_dsp_r2.o ../src/dsp/dec_mips_dsp_r2.c

${OBJECTDIR}/_ext/275491c7/dec_msa.o: ../src/dsp/dec_msa.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/dec_msa.o ../src/dsp/dec_msa.c

${OBJECTDIR}/_ext/275491c7/dec_neon.o: ../src/dsp/dec_neon.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/dec_neon.o ../src/dsp/dec_neon.c

${OBJECTDIR}/_ext/275491c7/dec_sse2.o: ../src/dsp/dec_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/dec_sse2.o ../src/dsp/dec_sse2.c

${OBJECTDIR}/_ext/275491c7/dec_sse41.o: ../src/dsp/dec_sse41.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/dec_sse41.o ../src/dsp/dec_sse41.c

${OBJECTDIR}/_ext/275491c7/enc.o: ../src/dsp/enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/enc.o ../src/dsp/enc.c

${OBJECTDIR}/_ext/275491c7/enc_avx2.o: ../src/dsp/enc_avx2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/enc_avx2.o ../src/dsp/enc_avx2.c

${OBJECTDIR}/_ext/275491c7/enc_mips32.o: ../src/dsp/enc_mips32.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/enc_mips32.o ../src/dsp/enc_mips32.c

${OBJECTDIR}/_ext/275491c7/enc_mips_dsp_r2.o: ../src/dsp/enc_mips_dsp_r2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/enc_mips_dsp_r2.o ../src/dsp/enc_mips_dsp_r2.c

${OBJECTDIR}/_ext/275491c7/enc_msa.o: ../src/dsp/enc_msa.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/enc_msa.o ../src/dsp/enc_msa.c

${OBJECTDIR}/_ext/275491c7/enc_neon.o: ../src/dsp/enc_neon.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/enc_neon.o ../src/dsp/enc_neon.c

${OBJECTDIR}/_ext/275491c7/enc_sse2.o: ../src/dsp/enc_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/enc_sse2.o ../src/dsp/enc_sse2.c

${OBJECTDIR}/_ext/275491c7/enc_sse41.o: ../src/dsp/enc_sse41.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/enc_sse41.o ../src/dsp/enc_sse41.c

${OBJECTDIR}/_ext/275491c7/filters.o: ../src/dsp/filters.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/filters.o ../src/dsp/filters.c

${OBJECTDIR}/_ext/275491c7/filters_mips_dsp_r2.o: ../src/dsp/filters_mips_dsp_r2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/filters_mips_dsp_r2.o ../src/dsp/filters_mips_dsp_r2.c

${OBJECTDIR}/_ext/275491c7/filters_msa.o: ../src/dsp/filters_msa.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/filters_msa.o ../src/dsp/filters_msa.c

${OBJECTDIR}/_ext/275491c7/filters_neon.o: ../src/dsp/filters_neon.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/filters_neon.o ../src/dsp/filters_neon.c

${OBJECTDIR}/_ext/275491c7/filters_sse2.o: ../src/dsp/filters_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/filters_sse2.o ../src/dsp/filters_sse2.c

${OBJECTDIR}/_ext/275491c7/lossless.o: ../src/dsp/lossless.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless.o ../src/dsp/lossless.c

${OBJECTDIR}/_ext/275491c7/lossless_enc.o: ../src/dsp/lossless_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless_enc.o ../src/dsp/lossless_enc.c

${OBJECTDIR}/_ext/275491c7/lossless_enc_mips32.o: ../src/dsp/lossless_enc_mips32.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless_enc_mips32.o ../src/dsp/lossless_enc_mips32.c

${OBJECTDIR}/_ext/275491c7/lossless_enc_mips_dsp_r2.o: ../src/dsp/lossless_enc_mips_dsp_r2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless_enc_mips_dsp_r2.o ../src/dsp/lossless_enc_mips_dsp_r2.c

${OBJECTDIR}/_ext/275491c7/lossless_enc_msa.o: ../src/dsp/lossless_enc_msa.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless_enc_msa.o ../src/dsp/lossless_enc_msa.c

${OBJECTDIR}/_ext/275491c7/lossless_enc_neon.o: ../src/dsp/lossless_enc_neon.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless_enc_neon.o ../src/dsp/lossless_enc_neon.c

${OBJECTDIR}/_ext/275491c7/lossless_enc_sse2.o: ../src/dsp/lossless_enc_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless_enc_sse2.o ../src/dsp/lossless_enc_sse2.c

${OBJECTDIR}/_ext/275491c7/lossless_enc_sse41.o: ../src/dsp/lossless_enc_sse41.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless_enc_sse41.o ../src/dsp/lossless_enc_sse41.c

${OBJECTDIR}/_ext/275491c7/lossless_mips_dsp_r2.o: ../src/dsp/lossless_mips_dsp_r2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless_mips_dsp_r2.o ../src/dsp/lossless_mips_dsp_r2.c

${OBJECTDIR}/_ext/275491c7/lossless_msa.o: ../src/dsp/lossless_msa.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless_msa.o ../src/dsp/lossless_msa.c

${OBJECTDIR}/_ext/275491c7/lossless_neon.o: ../src/dsp/lossless_neon.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless_neon.o ../src/dsp/lossless_neon.c

${OBJECTDIR}/_ext/275491c7/lossless_sse2.o: ../src/dsp/lossless_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/lossless_sse2.o ../src/dsp/lossless_sse2.c

${OBJECTDIR}/_ext/275491c7/rescaler.o: ../src/dsp/rescaler.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/rescaler.o ../src/dsp/rescaler.c

${OBJECTDIR}/_ext/275491c7/rescaler_mips32.o: ../src/dsp/rescaler_mips32.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/rescaler_mips32.o ../src/dsp/rescaler_mips32.c

${OBJECTDIR}/_ext/275491c7/rescaler_mips_dsp_r2.o: ../src/dsp/rescaler_mips_dsp_r2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/rescaler_mips_dsp_r2.o ../src/dsp/rescaler_mips_dsp_r2.c

${OBJECTDIR}/_ext/275491c7/rescaler_msa.o: ../src/dsp/rescaler_msa.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/rescaler_msa.o ../src/dsp/rescaler_msa.c

${OBJECTDIR}/_ext/275491c7/rescaler_neon.o: ../src/dsp/rescaler_neon.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/rescaler_neon.o ../src/dsp/rescaler_neon.c

${OBJECTDIR}/_ext/275491c7/rescaler_sse2.o: ../src/dsp/rescaler_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/rescaler_sse2.o ../src/dsp/rescaler_sse2.c

${OBJECTDIR}/_ext/275491c7/ssim.o: ../src/dsp/ssim.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/ssim.o ../src/dsp/ssim.c

${OBJECTDIR}/_ext/275491c7/ssim_sse2.o: ../src/dsp/ssim_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/ssim_sse2.o ../src/dsp/ssim_sse2.c

${OBJECTDIR}/_ext/275491c7/upsampling.o: ../src/dsp/upsampling.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/upsampling.o ../src/dsp/upsampling.c

${OBJECTDIR}/_ext/275491c7/upsampling_mips_dsp_r2.o: ../src/dsp/upsampling_mips_dsp_r2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/upsampling_mips_dsp_r2.o ../src/dsp/upsampling_mips_dsp_r2.c

${OBJECTDIR}/_ext/275491c7/upsampling_msa.o: ../src/dsp/upsampling_msa.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/upsampling_msa.o ../src/dsp/upsampling_msa.c

${OBJECTDIR}/_ext/275491c7/upsampling_neon.o: ../src/dsp/upsampling_neon.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/upsampling_neon.o ../src/dsp/upsampling_neon.c

${OBJECTDIR}/_ext/275491c7/upsampling_sse2.o: ../src/dsp/upsampling_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/upsampling_sse2.o ../src/dsp/upsampling_sse2.c

${OBJECTDIR}/_ext/275491c7/yuv.o: ../src/dsp/yuv.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/yuv.o ../src/dsp/yuv.c

${OBJECTDIR}/_ext/275491c7/yuv_mips32.o: ../src/dsp/yuv_mips32.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/yuv_mips32.o ../src/dsp/yuv_mips32.c

${OBJECTDIR}/_ext/275491c7/yuv_mips_dsp_r2.o: ../src/dsp/yuv_mips_dsp_r2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/yuv_mips_dsp_r2.o ../src/dsp/yuv_mips_dsp_r2.c

${OBJECTDIR}/_ext/275491c7/yuv_sse2.o: ../src/dsp/yuv_sse2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275491c7
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275491c7/yuv_sse2.o ../src/dsp/yuv_sse2.c

${OBJECTDIR}/_ext/275494e0/alpha_enc.o: ../src/enc/alpha_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/alpha_enc.o ../src/enc/alpha_enc.c

${OBJECTDIR}/_ext/275494e0/analysis_enc.o: ../src/enc/analysis_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/analysis_enc.o ../src/enc/analysis_enc.c

${OBJECTDIR}/_ext/275494e0/backward_references_enc.o: ../src/enc/backward_references_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/backward_references_enc.o ../src/enc/backward_references_enc.c

${OBJECTDIR}/_ext/275494e0/config_enc.o: ../src/enc/config_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/config_enc.o ../src/enc/config_enc.c

${OBJECTDIR}/_ext/275494e0/cost_enc.o: ../src/enc/cost_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/cost_enc.o ../src/enc/cost_enc.c

${OBJECTDIR}/_ext/275494e0/delta_palettization_enc.o: ../src/enc/delta_palettization_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/delta_palettization_enc.o ../src/enc/delta_palettization_enc.c

${OBJECTDIR}/_ext/275494e0/filter_enc.o: ../src/enc/filter_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/filter_enc.o ../src/enc/filter_enc.c

${OBJECTDIR}/_ext/275494e0/frame_enc.o: ../src/enc/frame_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/frame_enc.o ../src/enc/frame_enc.c

${OBJECTDIR}/_ext/275494e0/histogram_enc.o: ../src/enc/histogram_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/histogram_enc.o ../src/enc/histogram_enc.c

${OBJECTDIR}/_ext/275494e0/iterator_enc.o: ../src/enc/iterator_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/iterator_enc.o ../src/enc/iterator_enc.c

${OBJECTDIR}/_ext/275494e0/near_lossless_enc.o: ../src/enc/near_lossless_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/near_lossless_enc.o ../src/enc/near_lossless_enc.c

${OBJECTDIR}/_ext/275494e0/picture_csp_enc.o: ../src/enc/picture_csp_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/picture_csp_enc.o ../src/enc/picture_csp_enc.c

${OBJECTDIR}/_ext/275494e0/picture_enc.o: ../src/enc/picture_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/picture_enc.o ../src/enc/picture_enc.c

${OBJECTDIR}/_ext/275494e0/picture_psnr_enc.o: ../src/enc/picture_psnr_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/picture_psnr_enc.o ../src/enc/picture_psnr_enc.c

${OBJECTDIR}/_ext/275494e0/picture_rescale_enc.o: ../src/enc/picture_rescale_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/picture_rescale_enc.o ../src/enc/picture_rescale_enc.c

${OBJECTDIR}/_ext/275494e0/picture_tools_enc.o: ../src/enc/picture_tools_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/picture_tools_enc.o ../src/enc/picture_tools_enc.c

${OBJECTDIR}/_ext/275494e0/predictor_enc.o: ../src/enc/predictor_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/predictor_enc.o ../src/enc/predictor_enc.c

${OBJECTDIR}/_ext/275494e0/quant_enc.o: ../src/enc/quant_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/quant_enc.o ../src/enc/quant_enc.c

${OBJECTDIR}/_ext/275494e0/syntax_enc.o: ../src/enc/syntax_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/syntax_enc.o ../src/enc/syntax_enc.c

${OBJECTDIR}/_ext/275494e0/token_enc.o: ../src/enc/token_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/token_enc.o ../src/enc/token_enc.c

${OBJECTDIR}/_ext/275494e0/tree_enc.o: ../src/enc/tree_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/tree_enc.o ../src/enc/tree_enc.c

${OBJECTDIR}/_ext/275494e0/vp8l_enc.o: ../src/enc/vp8l_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/vp8l_enc.o ../src/enc/vp8l_enc.c

${OBJECTDIR}/_ext/275494e0/webp_enc.o: ../src/enc/webp_enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/275494e0
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/275494e0/webp_enc.o ../src/enc/webp_enc.c

${OBJECTDIR}/_ext/2754b3d6/anim_encode.o: ../src/mux/anim_encode.c
	${MKDIR} -p ${OBJECTDIR}/_ext/2754b3d6
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2754b3d6/anim_encode.o ../src/mux/anim_encode.c

${OBJECTDIR}/_ext/2754b3d6/muxedit.o: ../src/mux/muxedit.c
	${MKDIR} -p ${OBJECTDIR}/_ext/2754b3d6
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2754b3d6/muxedit.o ../src/mux/muxedit.c

${OBJECTDIR}/_ext/2754b3d6/muxinternal.o: ../src/mux/muxinternal.c
	${MKDIR} -p ${OBJECTDIR}/_ext/2754b3d6
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2754b3d6/muxinternal.o ../src/mux/muxinternal.c

${OBJECTDIR}/_ext/2754b3d6/muxread.o: ../src/mux/muxread.c
	${MKDIR} -p ${OBJECTDIR}/_ext/2754b3d6
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2754b3d6/muxread.o ../src/mux/muxread.c

${OBJECTDIR}/_ext/a5673337/bit_reader_utils.o: ../src/utils/bit_reader_utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/bit_reader_utils.o ../src/utils/bit_reader_utils.c

${OBJECTDIR}/_ext/a5673337/bit_writer_utils.o: ../src/utils/bit_writer_utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/bit_writer_utils.o ../src/utils/bit_writer_utils.c

${OBJECTDIR}/_ext/a5673337/color_cache_utils.o: ../src/utils/color_cache_utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/color_cache_utils.o ../src/utils/color_cache_utils.c

${OBJECTDIR}/_ext/a5673337/filters_utils.o: ../src/utils/filters_utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/filters_utils.o ../src/utils/filters_utils.c

${OBJECTDIR}/_ext/a5673337/huffman_encode_utils.o: ../src/utils/huffman_encode_utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/huffman_encode_utils.o ../src/utils/huffman_encode_utils.c

${OBJECTDIR}/_ext/a5673337/huffman_utils.o: ../src/utils/huffman_utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/huffman_utils.o ../src/utils/huffman_utils.c

${OBJECTDIR}/_ext/a5673337/quant_levels_dec_utils.o: ../src/utils/quant_levels_dec_utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/quant_levels_dec_utils.o ../src/utils/quant_levels_dec_utils.c

${OBJECTDIR}/_ext/a5673337/quant_levels_utils.o: ../src/utils/quant_levels_utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/quant_levels_utils.o ../src/utils/quant_levels_utils.c

${OBJECTDIR}/_ext/a5673337/random_utils.o: ../src/utils/random_utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/random_utils.o ../src/utils/random_utils.c

${OBJECTDIR}/_ext/a5673337/rescaler_utils.o: ../src/utils/rescaler_utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/rescaler_utils.o ../src/utils/rescaler_utils.c

${OBJECTDIR}/_ext/a5673337/thread_utils.o: ../src/utils/thread_utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/thread_utils.o ../src/utils/thread_utils.c

${OBJECTDIR}/_ext/a5673337/utils.o: ../src/utils/utils.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5673337
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5673337/utils.o ../src/utils/utils.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
