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
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/13266880/aac_ram.o \
	${OBJECTDIR}/_ext/13266880/aac_rom.o \
	${OBJECTDIR}/_ext/13266880/aacdec_drc.o \
	${OBJECTDIR}/_ext/13266880/aacdec_hcr.o \
	${OBJECTDIR}/_ext/13266880/aacdec_hcr_bit.o \
	${OBJECTDIR}/_ext/13266880/aacdec_hcrs.o \
	${OBJECTDIR}/_ext/13266880/aacdec_pns.o \
	${OBJECTDIR}/_ext/13266880/aacdec_tns.o \
	${OBJECTDIR}/_ext/13266880/aacdecoder.o \
	${OBJECTDIR}/_ext/13266880/aacdecoder_lib.o \
	${OBJECTDIR}/_ext/13266880/block.o \
	${OBJECTDIR}/_ext/13266880/channel.o \
	${OBJECTDIR}/_ext/13266880/channelinfo.o \
	${OBJECTDIR}/_ext/13266880/conceal.o \
	${OBJECTDIR}/_ext/13266880/ldfiltbank.o \
	${OBJECTDIR}/_ext/13266880/pulsedata.o \
	${OBJECTDIR}/_ext/13266880/rvlc.o \
	${OBJECTDIR}/_ext/13266880/rvlcbit.o \
	${OBJECTDIR}/_ext/13266880/rvlcconceal.o \
	${OBJECTDIR}/_ext/13266880/stereo.o \
	${OBJECTDIR}/_ext/57684158/aacEnc_ram.o \
	${OBJECTDIR}/_ext/57684158/aacEnc_rom.o \
	${OBJECTDIR}/_ext/57684158/aacenc.o \
	${OBJECTDIR}/_ext/57684158/aacenc_lib.o \
	${OBJECTDIR}/_ext/57684158/aacenc_pns.o \
	${OBJECTDIR}/_ext/57684158/aacenc_tns.o \
	${OBJECTDIR}/_ext/57684158/adj_thr.o \
	${OBJECTDIR}/_ext/57684158/band_nrg.o \
	${OBJECTDIR}/_ext/57684158/bandwidth.o \
	${OBJECTDIR}/_ext/57684158/bit_cnt.o \
	${OBJECTDIR}/_ext/57684158/bitenc.o \
	${OBJECTDIR}/_ext/57684158/block_switch.o \
	${OBJECTDIR}/_ext/57684158/channel_map.o \
	${OBJECTDIR}/_ext/57684158/chaosmeasure.o \
	${OBJECTDIR}/_ext/57684158/dyn_bits.o \
	${OBJECTDIR}/_ext/57684158/grp_data.o \
	${OBJECTDIR}/_ext/57684158/intensity.o \
	${OBJECTDIR}/_ext/57684158/line_pe.o \
	${OBJECTDIR}/_ext/57684158/metadata_compressor.o \
	${OBJECTDIR}/_ext/57684158/metadata_main.o \
	${OBJECTDIR}/_ext/57684158/ms_stereo.o \
	${OBJECTDIR}/_ext/57684158/noisedet.o \
	${OBJECTDIR}/_ext/57684158/pnsparam.o \
	${OBJECTDIR}/_ext/57684158/pre_echo_control.o \
	${OBJECTDIR}/_ext/57684158/psy_configuration.o \
	${OBJECTDIR}/_ext/57684158/psy_main.o \
	${OBJECTDIR}/_ext/57684158/qc_main.o \
	${OBJECTDIR}/_ext/57684158/quantize.o \
	${OBJECTDIR}/_ext/57684158/sf_estim.o \
	${OBJECTDIR}/_ext/57684158/spreading.o \
	${OBJECTDIR}/_ext/57684158/tonality.o \
	${OBJECTDIR}/_ext/57684158/transform.o \
	${OBJECTDIR}/_ext/7dc747f6/FDK_bitbuffer.o \
	${OBJECTDIR}/_ext/7dc747f6/FDK_core.o \
	${OBJECTDIR}/_ext/7dc747f6/FDK_crc.o \
	${OBJECTDIR}/_ext/7dc747f6/FDK_hybrid.o \
	${OBJECTDIR}/_ext/7dc747f6/FDK_tools_rom.o \
	${OBJECTDIR}/_ext/7dc747f6/FDK_trigFcts.o \
	${OBJECTDIR}/_ext/7dc747f6/autocorr2nd.o \
	${OBJECTDIR}/_ext/7dc747f6/dct.o \
	${OBJECTDIR}/_ext/7dc747f6/fft.o \
	${OBJECTDIR}/_ext/7dc747f6/fft_rad2.o \
	${OBJECTDIR}/_ext/7dc747f6/fixpoint_math.o \
	${OBJECTDIR}/_ext/7dc747f6/mdct.o \
	${OBJECTDIR}/_ext/7dc747f6/qmf.o \
	${OBJECTDIR}/_ext/7dc747f6/scale.o \
	${OBJECTDIR}/_ext/6d6c8d0a/tpdec_adif.o \
	${OBJECTDIR}/_ext/6d6c8d0a/tpdec_adts.o \
	${OBJECTDIR}/_ext/6d6c8d0a/tpdec_asc.o \
	${OBJECTDIR}/_ext/6d6c8d0a/tpdec_drm.o \
	${OBJECTDIR}/_ext/6d6c8d0a/tpdec_latm.o \
	${OBJECTDIR}/_ext/6d6c8d0a/tpdec_lib.o \
	${OBJECTDIR}/_ext/b1ae65e2/tpenc_adif.o \
	${OBJECTDIR}/_ext/b1ae65e2/tpenc_adts.o \
	${OBJECTDIR}/_ext/b1ae65e2/tpenc_asc.o \
	${OBJECTDIR}/_ext/b1ae65e2/tpenc_latm.o \
	${OBJECTDIR}/_ext/b1ae65e2/tpenc_lib.o \
	${OBJECTDIR}/_ext/9f5aa0b8/limiter.o \
	${OBJECTDIR}/_ext/9f5aa0b8/pcmutils_lib.o \
	${OBJECTDIR}/_ext/eac66cc0/env_calc.o \
	${OBJECTDIR}/_ext/eac66cc0/env_dec.o \
	${OBJECTDIR}/_ext/eac66cc0/env_extr.o \
	${OBJECTDIR}/_ext/eac66cc0/huff_dec.o \
	${OBJECTDIR}/_ext/eac66cc0/lpp_tran.o \
	${OBJECTDIR}/_ext/eac66cc0/psbitdec.o \
	${OBJECTDIR}/_ext/eac66cc0/psdec.o \
	${OBJECTDIR}/_ext/eac66cc0/psdec_hybrid.o \
	${OBJECTDIR}/_ext/eac66cc0/sbr_crc.o \
	${OBJECTDIR}/_ext/eac66cc0/sbr_deb.o \
	${OBJECTDIR}/_ext/eac66cc0/sbr_dec.o \
	${OBJECTDIR}/_ext/eac66cc0/sbr_ram.o \
	${OBJECTDIR}/_ext/eac66cc0/sbr_rom.o \
	${OBJECTDIR}/_ext/eac66cc0/sbrdec_drc.o \
	${OBJECTDIR}/_ext/eac66cc0/sbrdec_freq_sca.o \
	${OBJECTDIR}/_ext/eac66cc0/sbrdecoder.o \
	${OBJECTDIR}/_ext/2f084598/bit_sbr.o \
	${OBJECTDIR}/_ext/2f084598/code_env.o \
	${OBJECTDIR}/_ext/2f084598/env_bit.o \
	${OBJECTDIR}/_ext/2f084598/env_est.o \
	${OBJECTDIR}/_ext/2f084598/fram_gen.o \
	${OBJECTDIR}/_ext/2f084598/invf_est.o \
	${OBJECTDIR}/_ext/2f084598/mh_det.o \
	${OBJECTDIR}/_ext/2f084598/nf_est.o \
	${OBJECTDIR}/_ext/2f084598/ps_bitenc.o \
	${OBJECTDIR}/_ext/2f084598/ps_encode.o \
	${OBJECTDIR}/_ext/2f084598/ps_main.o \
	${OBJECTDIR}/_ext/2f084598/resampler.o \
	${OBJECTDIR}/_ext/2f084598/sbr_encoder.o \
	${OBJECTDIR}/_ext/2f084598/sbr_misc.o \
	${OBJECTDIR}/_ext/2f084598/sbr_ram_.o \
	${OBJECTDIR}/_ext/2f084598/sbr_rom_.o \
	${OBJECTDIR}/_ext/2f084598/sbrenc_freq_sca.o \
	${OBJECTDIR}/_ext/2f084598/ton_corr.o \
	${OBJECTDIR}/_ext/2f084598/tran_det.o \
	${OBJECTDIR}/_ext/51becb56/conv_string.o \
	${OBJECTDIR}/_ext/51becb56/genericStds.o \
	${OBJECTDIR}/_ext/51becb56/wav_file.o


# C Compiler Flags
CFLAGS=-m64

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

${OBJECTDIR}/_ext/13266880/aac_ram.o: ../lib/libAACdec/src/aac_ram.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/aac_ram.o ../lib/libAACdec/src/aac_ram.cpp

${OBJECTDIR}/_ext/13266880/aac_rom.o: ../lib/libAACdec/src/aac_rom.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/aac_rom.o ../lib/libAACdec/src/aac_rom.cpp

${OBJECTDIR}/_ext/13266880/aacdec_drc.o: ../lib/libAACdec/src/aacdec_drc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/aacdec_drc.o ../lib/libAACdec/src/aacdec_drc.cpp

${OBJECTDIR}/_ext/13266880/aacdec_hcr.o: ../lib/libAACdec/src/aacdec_hcr.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/aacdec_hcr.o ../lib/libAACdec/src/aacdec_hcr.cpp

${OBJECTDIR}/_ext/13266880/aacdec_hcr_bit.o: ../lib/libAACdec/src/aacdec_hcr_bit.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/aacdec_hcr_bit.o ../lib/libAACdec/src/aacdec_hcr_bit.cpp

${OBJECTDIR}/_ext/13266880/aacdec_hcrs.o: ../lib/libAACdec/src/aacdec_hcrs.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/aacdec_hcrs.o ../lib/libAACdec/src/aacdec_hcrs.cpp

${OBJECTDIR}/_ext/13266880/aacdec_pns.o: ../lib/libAACdec/src/aacdec_pns.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/aacdec_pns.o ../lib/libAACdec/src/aacdec_pns.cpp

${OBJECTDIR}/_ext/13266880/aacdec_tns.o: ../lib/libAACdec/src/aacdec_tns.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/aacdec_tns.o ../lib/libAACdec/src/aacdec_tns.cpp

${OBJECTDIR}/_ext/13266880/aacdecoder.o: ../lib/libAACdec/src/aacdecoder.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/aacdecoder.o ../lib/libAACdec/src/aacdecoder.cpp

${OBJECTDIR}/_ext/13266880/aacdecoder_lib.o: ../lib/libAACdec/src/aacdecoder_lib.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/aacdecoder_lib.o ../lib/libAACdec/src/aacdecoder_lib.cpp

${OBJECTDIR}/_ext/13266880/block.o: ../lib/libAACdec/src/block.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/block.o ../lib/libAACdec/src/block.cpp

${OBJECTDIR}/_ext/13266880/channel.o: ../lib/libAACdec/src/channel.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/channel.o ../lib/libAACdec/src/channel.cpp

${OBJECTDIR}/_ext/13266880/channelinfo.o: ../lib/libAACdec/src/channelinfo.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/channelinfo.o ../lib/libAACdec/src/channelinfo.cpp

${OBJECTDIR}/_ext/13266880/conceal.o: ../lib/libAACdec/src/conceal.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/conceal.o ../lib/libAACdec/src/conceal.cpp

${OBJECTDIR}/_ext/13266880/ldfiltbank.o: ../lib/libAACdec/src/ldfiltbank.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/ldfiltbank.o ../lib/libAACdec/src/ldfiltbank.cpp

${OBJECTDIR}/_ext/13266880/pulsedata.o: ../lib/libAACdec/src/pulsedata.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/pulsedata.o ../lib/libAACdec/src/pulsedata.cpp

${OBJECTDIR}/_ext/13266880/rvlc.o: ../lib/libAACdec/src/rvlc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/rvlc.o ../lib/libAACdec/src/rvlc.cpp

${OBJECTDIR}/_ext/13266880/rvlcbit.o: ../lib/libAACdec/src/rvlcbit.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/rvlcbit.o ../lib/libAACdec/src/rvlcbit.cpp

${OBJECTDIR}/_ext/13266880/rvlcconceal.o: ../lib/libAACdec/src/rvlcconceal.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/rvlcconceal.o ../lib/libAACdec/src/rvlcconceal.cpp

${OBJECTDIR}/_ext/13266880/stereo.o: ../lib/libAACdec/src/stereo.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/13266880
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13266880/stereo.o ../lib/libAACdec/src/stereo.cpp

${OBJECTDIR}/_ext/57684158/aacEnc_ram.o: ../lib/libAACenc/src/aacEnc_ram.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/aacEnc_ram.o ../lib/libAACenc/src/aacEnc_ram.cpp

${OBJECTDIR}/_ext/57684158/aacEnc_rom.o: ../lib/libAACenc/src/aacEnc_rom.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/aacEnc_rom.o ../lib/libAACenc/src/aacEnc_rom.cpp

${OBJECTDIR}/_ext/57684158/aacenc.o: ../lib/libAACenc/src/aacenc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/aacenc.o ../lib/libAACenc/src/aacenc.cpp

${OBJECTDIR}/_ext/57684158/aacenc_lib.o: ../lib/libAACenc/src/aacenc_lib.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/aacenc_lib.o ../lib/libAACenc/src/aacenc_lib.cpp

${OBJECTDIR}/_ext/57684158/aacenc_pns.o: ../lib/libAACenc/src/aacenc_pns.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/aacenc_pns.o ../lib/libAACenc/src/aacenc_pns.cpp

${OBJECTDIR}/_ext/57684158/aacenc_tns.o: ../lib/libAACenc/src/aacenc_tns.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/aacenc_tns.o ../lib/libAACenc/src/aacenc_tns.cpp

${OBJECTDIR}/_ext/57684158/adj_thr.o: ../lib/libAACenc/src/adj_thr.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/adj_thr.o ../lib/libAACenc/src/adj_thr.cpp

${OBJECTDIR}/_ext/57684158/band_nrg.o: ../lib/libAACenc/src/band_nrg.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/band_nrg.o ../lib/libAACenc/src/band_nrg.cpp

${OBJECTDIR}/_ext/57684158/bandwidth.o: ../lib/libAACenc/src/bandwidth.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/bandwidth.o ../lib/libAACenc/src/bandwidth.cpp

${OBJECTDIR}/_ext/57684158/bit_cnt.o: ../lib/libAACenc/src/bit_cnt.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/bit_cnt.o ../lib/libAACenc/src/bit_cnt.cpp

${OBJECTDIR}/_ext/57684158/bitenc.o: ../lib/libAACenc/src/bitenc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/bitenc.o ../lib/libAACenc/src/bitenc.cpp

${OBJECTDIR}/_ext/57684158/block_switch.o: ../lib/libAACenc/src/block_switch.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/block_switch.o ../lib/libAACenc/src/block_switch.cpp

${OBJECTDIR}/_ext/57684158/channel_map.o: ../lib/libAACenc/src/channel_map.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/channel_map.o ../lib/libAACenc/src/channel_map.cpp

${OBJECTDIR}/_ext/57684158/chaosmeasure.o: ../lib/libAACenc/src/chaosmeasure.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/chaosmeasure.o ../lib/libAACenc/src/chaosmeasure.cpp

${OBJECTDIR}/_ext/57684158/dyn_bits.o: ../lib/libAACenc/src/dyn_bits.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/dyn_bits.o ../lib/libAACenc/src/dyn_bits.cpp

${OBJECTDIR}/_ext/57684158/grp_data.o: ../lib/libAACenc/src/grp_data.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/grp_data.o ../lib/libAACenc/src/grp_data.cpp

${OBJECTDIR}/_ext/57684158/intensity.o: ../lib/libAACenc/src/intensity.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/intensity.o ../lib/libAACenc/src/intensity.cpp

${OBJECTDIR}/_ext/57684158/line_pe.o: ../lib/libAACenc/src/line_pe.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/line_pe.o ../lib/libAACenc/src/line_pe.cpp

${OBJECTDIR}/_ext/57684158/metadata_compressor.o: ../lib/libAACenc/src/metadata_compressor.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/metadata_compressor.o ../lib/libAACenc/src/metadata_compressor.cpp

${OBJECTDIR}/_ext/57684158/metadata_main.o: ../lib/libAACenc/src/metadata_main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/metadata_main.o ../lib/libAACenc/src/metadata_main.cpp

${OBJECTDIR}/_ext/57684158/ms_stereo.o: ../lib/libAACenc/src/ms_stereo.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/ms_stereo.o ../lib/libAACenc/src/ms_stereo.cpp

${OBJECTDIR}/_ext/57684158/noisedet.o: ../lib/libAACenc/src/noisedet.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/noisedet.o ../lib/libAACenc/src/noisedet.cpp

${OBJECTDIR}/_ext/57684158/pnsparam.o: ../lib/libAACenc/src/pnsparam.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/pnsparam.o ../lib/libAACenc/src/pnsparam.cpp

${OBJECTDIR}/_ext/57684158/pre_echo_control.o: ../lib/libAACenc/src/pre_echo_control.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/pre_echo_control.o ../lib/libAACenc/src/pre_echo_control.cpp

${OBJECTDIR}/_ext/57684158/psy_configuration.o: ../lib/libAACenc/src/psy_configuration.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/psy_configuration.o ../lib/libAACenc/src/psy_configuration.cpp

${OBJECTDIR}/_ext/57684158/psy_main.o: ../lib/libAACenc/src/psy_main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/psy_main.o ../lib/libAACenc/src/psy_main.cpp

${OBJECTDIR}/_ext/57684158/qc_main.o: ../lib/libAACenc/src/qc_main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/qc_main.o ../lib/libAACenc/src/qc_main.cpp

${OBJECTDIR}/_ext/57684158/quantize.o: ../lib/libAACenc/src/quantize.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/quantize.o ../lib/libAACenc/src/quantize.cpp

${OBJECTDIR}/_ext/57684158/sf_estim.o: ../lib/libAACenc/src/sf_estim.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/sf_estim.o ../lib/libAACenc/src/sf_estim.cpp

${OBJECTDIR}/_ext/57684158/spreading.o: ../lib/libAACenc/src/spreading.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/spreading.o ../lib/libAACenc/src/spreading.cpp

${OBJECTDIR}/_ext/57684158/tonality.o: ../lib/libAACenc/src/tonality.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/tonality.o ../lib/libAACenc/src/tonality.cpp

${OBJECTDIR}/_ext/57684158/transform.o: ../lib/libAACenc/src/transform.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/57684158
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/57684158/transform.o ../lib/libAACenc/src/transform.cpp

${OBJECTDIR}/_ext/7dc747f6/FDK_bitbuffer.o: ../lib/libFDK/src/FDK_bitbuffer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/FDK_bitbuffer.o ../lib/libFDK/src/FDK_bitbuffer.cpp

${OBJECTDIR}/_ext/7dc747f6/FDK_core.o: ../lib/libFDK/src/FDK_core.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/FDK_core.o ../lib/libFDK/src/FDK_core.cpp

${OBJECTDIR}/_ext/7dc747f6/FDK_crc.o: ../lib/libFDK/src/FDK_crc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/FDK_crc.o ../lib/libFDK/src/FDK_crc.cpp

${OBJECTDIR}/_ext/7dc747f6/FDK_hybrid.o: ../lib/libFDK/src/FDK_hybrid.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/FDK_hybrid.o ../lib/libFDK/src/FDK_hybrid.cpp

${OBJECTDIR}/_ext/7dc747f6/FDK_tools_rom.o: ../lib/libFDK/src/FDK_tools_rom.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/FDK_tools_rom.o ../lib/libFDK/src/FDK_tools_rom.cpp

${OBJECTDIR}/_ext/7dc747f6/FDK_trigFcts.o: ../lib/libFDK/src/FDK_trigFcts.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/FDK_trigFcts.o ../lib/libFDK/src/FDK_trigFcts.cpp

${OBJECTDIR}/_ext/7dc747f6/autocorr2nd.o: ../lib/libFDK/src/autocorr2nd.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/autocorr2nd.o ../lib/libFDK/src/autocorr2nd.cpp

${OBJECTDIR}/_ext/7dc747f6/dct.o: ../lib/libFDK/src/dct.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/dct.o ../lib/libFDK/src/dct.cpp

${OBJECTDIR}/_ext/7dc747f6/fft.o: ../lib/libFDK/src/fft.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/fft.o ../lib/libFDK/src/fft.cpp

${OBJECTDIR}/_ext/7dc747f6/fft_rad2.o: ../lib/libFDK/src/fft_rad2.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/fft_rad2.o ../lib/libFDK/src/fft_rad2.cpp

${OBJECTDIR}/_ext/7dc747f6/fixpoint_math.o: ../lib/libFDK/src/fixpoint_math.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/fixpoint_math.o ../lib/libFDK/src/fixpoint_math.cpp

${OBJECTDIR}/_ext/7dc747f6/mdct.o: ../lib/libFDK/src/mdct.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/mdct.o ../lib/libFDK/src/mdct.cpp

${OBJECTDIR}/_ext/7dc747f6/qmf.o: ../lib/libFDK/src/qmf.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/qmf.o ../lib/libFDK/src/qmf.cpp

${OBJECTDIR}/_ext/7dc747f6/scale.o: ../lib/libFDK/src/scale.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/7dc747f6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7dc747f6/scale.o ../lib/libFDK/src/scale.cpp

${OBJECTDIR}/_ext/6d6c8d0a/tpdec_adif.o: ../lib/libMpegTPDec/src/tpdec_adif.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6d6c8d0a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6d6c8d0a/tpdec_adif.o ../lib/libMpegTPDec/src/tpdec_adif.cpp

${OBJECTDIR}/_ext/6d6c8d0a/tpdec_adts.o: ../lib/libMpegTPDec/src/tpdec_adts.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6d6c8d0a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6d6c8d0a/tpdec_adts.o ../lib/libMpegTPDec/src/tpdec_adts.cpp

${OBJECTDIR}/_ext/6d6c8d0a/tpdec_asc.o: ../lib/libMpegTPDec/src/tpdec_asc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6d6c8d0a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6d6c8d0a/tpdec_asc.o ../lib/libMpegTPDec/src/tpdec_asc.cpp

${OBJECTDIR}/_ext/6d6c8d0a/tpdec_drm.o: ../lib/libMpegTPDec/src/tpdec_drm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6d6c8d0a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6d6c8d0a/tpdec_drm.o ../lib/libMpegTPDec/src/tpdec_drm.cpp

${OBJECTDIR}/_ext/6d6c8d0a/tpdec_latm.o: ../lib/libMpegTPDec/src/tpdec_latm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6d6c8d0a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6d6c8d0a/tpdec_latm.o ../lib/libMpegTPDec/src/tpdec_latm.cpp

${OBJECTDIR}/_ext/6d6c8d0a/tpdec_lib.o: ../lib/libMpegTPDec/src/tpdec_lib.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6d6c8d0a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6d6c8d0a/tpdec_lib.o ../lib/libMpegTPDec/src/tpdec_lib.cpp

${OBJECTDIR}/_ext/b1ae65e2/tpenc_adif.o: ../lib/libMpegTPEnc/src/tpenc_adif.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/b1ae65e2
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b1ae65e2/tpenc_adif.o ../lib/libMpegTPEnc/src/tpenc_adif.cpp

${OBJECTDIR}/_ext/b1ae65e2/tpenc_adts.o: ../lib/libMpegTPEnc/src/tpenc_adts.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/b1ae65e2
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b1ae65e2/tpenc_adts.o ../lib/libMpegTPEnc/src/tpenc_adts.cpp

${OBJECTDIR}/_ext/b1ae65e2/tpenc_asc.o: ../lib/libMpegTPEnc/src/tpenc_asc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/b1ae65e2
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b1ae65e2/tpenc_asc.o ../lib/libMpegTPEnc/src/tpenc_asc.cpp

${OBJECTDIR}/_ext/b1ae65e2/tpenc_latm.o: ../lib/libMpegTPEnc/src/tpenc_latm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/b1ae65e2
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b1ae65e2/tpenc_latm.o ../lib/libMpegTPEnc/src/tpenc_latm.cpp

${OBJECTDIR}/_ext/b1ae65e2/tpenc_lib.o: ../lib/libMpegTPEnc/src/tpenc_lib.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/b1ae65e2
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b1ae65e2/tpenc_lib.o ../lib/libMpegTPEnc/src/tpenc_lib.cpp

${OBJECTDIR}/_ext/9f5aa0b8/limiter.o: ../lib/libPCMutils/src/limiter.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/9f5aa0b8
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/9f5aa0b8/limiter.o ../lib/libPCMutils/src/limiter.cpp

${OBJECTDIR}/_ext/9f5aa0b8/pcmutils_lib.o: ../lib/libPCMutils/src/pcmutils_lib.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/9f5aa0b8
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/9f5aa0b8/pcmutils_lib.o ../lib/libPCMutils/src/pcmutils_lib.cpp

${OBJECTDIR}/_ext/eac66cc0/env_calc.o: ../lib/libSBRdec/src/env_calc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/env_calc.o ../lib/libSBRdec/src/env_calc.cpp

${OBJECTDIR}/_ext/eac66cc0/env_dec.o: ../lib/libSBRdec/src/env_dec.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/env_dec.o ../lib/libSBRdec/src/env_dec.cpp

${OBJECTDIR}/_ext/eac66cc0/env_extr.o: ../lib/libSBRdec/src/env_extr.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/env_extr.o ../lib/libSBRdec/src/env_extr.cpp

${OBJECTDIR}/_ext/eac66cc0/huff_dec.o: ../lib/libSBRdec/src/huff_dec.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/huff_dec.o ../lib/libSBRdec/src/huff_dec.cpp

${OBJECTDIR}/_ext/eac66cc0/lpp_tran.o: ../lib/libSBRdec/src/lpp_tran.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/lpp_tran.o ../lib/libSBRdec/src/lpp_tran.cpp

${OBJECTDIR}/_ext/eac66cc0/psbitdec.o: ../lib/libSBRdec/src/psbitdec.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/psbitdec.o ../lib/libSBRdec/src/psbitdec.cpp

${OBJECTDIR}/_ext/eac66cc0/psdec.o: ../lib/libSBRdec/src/psdec.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/psdec.o ../lib/libSBRdec/src/psdec.cpp

${OBJECTDIR}/_ext/eac66cc0/psdec_hybrid.o: ../lib/libSBRdec/src/psdec_hybrid.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/psdec_hybrid.o ../lib/libSBRdec/src/psdec_hybrid.cpp

${OBJECTDIR}/_ext/eac66cc0/sbr_crc.o: ../lib/libSBRdec/src/sbr_crc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/sbr_crc.o ../lib/libSBRdec/src/sbr_crc.cpp

${OBJECTDIR}/_ext/eac66cc0/sbr_deb.o: ../lib/libSBRdec/src/sbr_deb.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/sbr_deb.o ../lib/libSBRdec/src/sbr_deb.cpp

${OBJECTDIR}/_ext/eac66cc0/sbr_dec.o: ../lib/libSBRdec/src/sbr_dec.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/sbr_dec.o ../lib/libSBRdec/src/sbr_dec.cpp

${OBJECTDIR}/_ext/eac66cc0/sbr_ram.o: ../lib/libSBRdec/src/sbr_ram.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/sbr_ram.o ../lib/libSBRdec/src/sbr_ram.cpp

${OBJECTDIR}/_ext/eac66cc0/sbr_rom.o: ../lib/libSBRdec/src/sbr_rom.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/sbr_rom.o ../lib/libSBRdec/src/sbr_rom.cpp

${OBJECTDIR}/_ext/eac66cc0/sbrdec_drc.o: ../lib/libSBRdec/src/sbrdec_drc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/sbrdec_drc.o ../lib/libSBRdec/src/sbrdec_drc.cpp

${OBJECTDIR}/_ext/eac66cc0/sbrdec_freq_sca.o: ../lib/libSBRdec/src/sbrdec_freq_sca.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/sbrdec_freq_sca.o ../lib/libSBRdec/src/sbrdec_freq_sca.cpp

${OBJECTDIR}/_ext/eac66cc0/sbrdecoder.o: ../lib/libSBRdec/src/sbrdecoder.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eac66cc0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eac66cc0/sbrdecoder.o ../lib/libSBRdec/src/sbrdecoder.cpp

${OBJECTDIR}/_ext/2f084598/bit_sbr.o: ../lib/libSBRenc/src/bit_sbr.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/bit_sbr.o ../lib/libSBRenc/src/bit_sbr.cpp

${OBJECTDIR}/_ext/2f084598/code_env.o: ../lib/libSBRenc/src/code_env.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/code_env.o ../lib/libSBRenc/src/code_env.cpp

${OBJECTDIR}/_ext/2f084598/env_bit.o: ../lib/libSBRenc/src/env_bit.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/env_bit.o ../lib/libSBRenc/src/env_bit.cpp

${OBJECTDIR}/_ext/2f084598/env_est.o: ../lib/libSBRenc/src/env_est.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/env_est.o ../lib/libSBRenc/src/env_est.cpp

${OBJECTDIR}/_ext/2f084598/fram_gen.o: ../lib/libSBRenc/src/fram_gen.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/fram_gen.o ../lib/libSBRenc/src/fram_gen.cpp

${OBJECTDIR}/_ext/2f084598/invf_est.o: ../lib/libSBRenc/src/invf_est.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/invf_est.o ../lib/libSBRenc/src/invf_est.cpp

${OBJECTDIR}/_ext/2f084598/mh_det.o: ../lib/libSBRenc/src/mh_det.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/mh_det.o ../lib/libSBRenc/src/mh_det.cpp

${OBJECTDIR}/_ext/2f084598/nf_est.o: ../lib/libSBRenc/src/nf_est.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/nf_est.o ../lib/libSBRenc/src/nf_est.cpp

${OBJECTDIR}/_ext/2f084598/ps_bitenc.o: ../lib/libSBRenc/src/ps_bitenc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/ps_bitenc.o ../lib/libSBRenc/src/ps_bitenc.cpp

${OBJECTDIR}/_ext/2f084598/ps_encode.o: ../lib/libSBRenc/src/ps_encode.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/ps_encode.o ../lib/libSBRenc/src/ps_encode.cpp

${OBJECTDIR}/_ext/2f084598/ps_main.o: ../lib/libSBRenc/src/ps_main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/ps_main.o ../lib/libSBRenc/src/ps_main.cpp

${OBJECTDIR}/_ext/2f084598/resampler.o: ../lib/libSBRenc/src/resampler.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/resampler.o ../lib/libSBRenc/src/resampler.cpp

${OBJECTDIR}/_ext/2f084598/sbr_encoder.o: ../lib/libSBRenc/src/sbr_encoder.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/sbr_encoder.o ../lib/libSBRenc/src/sbr_encoder.cpp

${OBJECTDIR}/_ext/2f084598/sbr_misc.o: ../lib/libSBRenc/src/sbr_misc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/sbr_misc.o ../lib/libSBRenc/src/sbr_misc.cpp

${OBJECTDIR}/_ext/2f084598/sbr_ram_.o: ../lib/libSBRenc/src/sbr_ram_.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/sbr_ram_.o ../lib/libSBRenc/src/sbr_ram_.cpp

${OBJECTDIR}/_ext/2f084598/sbr_rom_.o: ../lib/libSBRenc/src/sbr_rom_.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/sbr_rom_.o ../lib/libSBRenc/src/sbr_rom_.cpp

${OBJECTDIR}/_ext/2f084598/sbrenc_freq_sca.o: ../lib/libSBRenc/src/sbrenc_freq_sca.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/sbrenc_freq_sca.o ../lib/libSBRenc/src/sbrenc_freq_sca.cpp

${OBJECTDIR}/_ext/2f084598/ton_corr.o: ../lib/libSBRenc/src/ton_corr.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/ton_corr.o ../lib/libSBRenc/src/ton_corr.cpp

${OBJECTDIR}/_ext/2f084598/tran_det.o: ../lib/libSBRenc/src/tran_det.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/2f084598
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2f084598/tran_det.o ../lib/libSBRenc/src/tran_det.cpp

${OBJECTDIR}/_ext/51becb56/conv_string.o: ../lib/libSYS/src/conv_string.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/51becb56
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/51becb56/conv_string.o ../lib/libSYS/src/conv_string.cpp

${OBJECTDIR}/_ext/51becb56/genericStds.o: ../lib/libSYS/src/genericStds.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/51becb56
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/51becb56/genericStds.o ../lib/libSYS/src/genericStds.cpp

${OBJECTDIR}/_ext/51becb56/wav_file.o: ../lib/libSYS/src/wav_file.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/51becb56
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../lib/libAACdec/include -I../lib/libAACenc/include -I../lib/libFDK/include -I../lib/libMpegTPDec/include -I../lib/libMpegTPEnc/include -I../lib/libPCMutils/include -I../lib/libSBRdec/include -I../lib/libSBRenc/include -I../lib/libSYS/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/51becb56/wav_file.o ../lib/libSYS/src/wav_file.cpp

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
