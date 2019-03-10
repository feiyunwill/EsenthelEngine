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
	${OBJECTDIR}/_ext/511e25b6/analyze.o \
	${OBJECTDIR}/_ext/511e25b6/apiwrapper.o \
	${OBJECTDIR}/_ext/511e25b6/bitpack.o \
	${OBJECTDIR}/_ext/511e25b6/decapiwrapper.o \
	${OBJECTDIR}/_ext/511e25b6/decinfo.o \
	${OBJECTDIR}/_ext/511e25b6/decode.o \
	${OBJECTDIR}/_ext/511e25b6/dequant.o \
	${OBJECTDIR}/_ext/511e25b6/encapiwrapper.o \
	${OBJECTDIR}/_ext/511e25b6/encfrag.o \
	${OBJECTDIR}/_ext/511e25b6/encinfo.o \
	${OBJECTDIR}/_ext/511e25b6/encode.o \
	${OBJECTDIR}/_ext/511e25b6/enquant.o \
	${OBJECTDIR}/_ext/511e25b6/fdct.o \
	${OBJECTDIR}/_ext/511e25b6/fragment.o \
	${OBJECTDIR}/_ext/511e25b6/huffdec.o \
	${OBJECTDIR}/_ext/511e25b6/huffenc.o \
	${OBJECTDIR}/_ext/511e25b6/idct.o \
	${OBJECTDIR}/_ext/511e25b6/info.o \
	${OBJECTDIR}/_ext/511e25b6/internal.o \
	${OBJECTDIR}/_ext/511e25b6/mathops.o \
	${OBJECTDIR}/_ext/511e25b6/mcenc.o \
	${OBJECTDIR}/_ext/511e25b6/quant.o \
	${OBJECTDIR}/_ext/511e25b6/rate.o \
	${OBJECTDIR}/_ext/511e25b6/state.o \
	${OBJECTDIR}/_ext/511e25b6/tokenize.o \
	${OBJECTDIR}/_ext/a59f819d/mmxencfrag.o \
	${OBJECTDIR}/_ext/a59f819d/mmxfdct.o \
	${OBJECTDIR}/_ext/a59f819d/mmxfrag.o \
	${OBJECTDIR}/_ext/a59f819d/mmxidct.o \
	${OBJECTDIR}/_ext/a59f819d/mmxstate.o \
	${OBJECTDIR}/_ext/a59f819d/sse2encfrag.o \
	${OBJECTDIR}/_ext/a59f819d/sse2fdct.o \
	${OBJECTDIR}/_ext/a59f819d/sse2idct.o \
	${OBJECTDIR}/_ext/a59f819d/x86cpu.o \
	${OBJECTDIR}/_ext/a59f819d/x86enc.o \
	${OBJECTDIR}/_ext/a59f819d/x86enquant.o \
	${OBJECTDIR}/_ext/a59f819d/x86state.o


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

${OBJECTDIR}/_ext/511e25b6/analyze.o: ../lib/analyze.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/analyze.o ../lib/analyze.c

${OBJECTDIR}/_ext/511e25b6/apiwrapper.o: ../lib/apiwrapper.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/apiwrapper.o ../lib/apiwrapper.c

${OBJECTDIR}/_ext/511e25b6/bitpack.o: ../lib/bitpack.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/bitpack.o ../lib/bitpack.c

${OBJECTDIR}/_ext/511e25b6/decapiwrapper.o: ../lib/decapiwrapper.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/decapiwrapper.o ../lib/decapiwrapper.c

${OBJECTDIR}/_ext/511e25b6/decinfo.o: ../lib/decinfo.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/decinfo.o ../lib/decinfo.c

${OBJECTDIR}/_ext/511e25b6/decode.o: ../lib/decode.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/decode.o ../lib/decode.c

${OBJECTDIR}/_ext/511e25b6/dequant.o: ../lib/dequant.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/dequant.o ../lib/dequant.c

${OBJECTDIR}/_ext/511e25b6/encapiwrapper.o: ../lib/encapiwrapper.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/encapiwrapper.o ../lib/encapiwrapper.c

${OBJECTDIR}/_ext/511e25b6/encfrag.o: ../lib/encfrag.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/encfrag.o ../lib/encfrag.c

${OBJECTDIR}/_ext/511e25b6/encinfo.o: ../lib/encinfo.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/encinfo.o ../lib/encinfo.c

${OBJECTDIR}/_ext/511e25b6/encode.o: ../lib/encode.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/encode.o ../lib/encode.c

${OBJECTDIR}/_ext/511e25b6/enquant.o: ../lib/enquant.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/enquant.o ../lib/enquant.c

${OBJECTDIR}/_ext/511e25b6/fdct.o: ../lib/fdct.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/fdct.o ../lib/fdct.c

${OBJECTDIR}/_ext/511e25b6/fragment.o: ../lib/fragment.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/fragment.o ../lib/fragment.c

${OBJECTDIR}/_ext/511e25b6/huffdec.o: ../lib/huffdec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/huffdec.o ../lib/huffdec.c

${OBJECTDIR}/_ext/511e25b6/huffenc.o: ../lib/huffenc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/huffenc.o ../lib/huffenc.c

${OBJECTDIR}/_ext/511e25b6/idct.o: ../lib/idct.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/idct.o ../lib/idct.c

${OBJECTDIR}/_ext/511e25b6/info.o: ../lib/info.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/info.o ../lib/info.c

${OBJECTDIR}/_ext/511e25b6/internal.o: ../lib/internal.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/internal.o ../lib/internal.c

${OBJECTDIR}/_ext/511e25b6/mathops.o: ../lib/mathops.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/mathops.o ../lib/mathops.c

${OBJECTDIR}/_ext/511e25b6/mcenc.o: ../lib/mcenc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/mcenc.o ../lib/mcenc.c

${OBJECTDIR}/_ext/511e25b6/quant.o: ../lib/quant.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/quant.o ../lib/quant.c

${OBJECTDIR}/_ext/511e25b6/rate.o: ../lib/rate.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/rate.o ../lib/rate.c

${OBJECTDIR}/_ext/511e25b6/state.o: ../lib/state.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/state.o ../lib/state.c

${OBJECTDIR}/_ext/511e25b6/tokenize.o: ../lib/tokenize.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/tokenize.o ../lib/tokenize.c

${OBJECTDIR}/_ext/a59f819d/mmxencfrag.o: ../lib/x86/mmxencfrag.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/mmxencfrag.o ../lib/x86/mmxencfrag.c

${OBJECTDIR}/_ext/a59f819d/mmxfdct.o: ../lib/x86/mmxfdct.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/mmxfdct.o ../lib/x86/mmxfdct.c

${OBJECTDIR}/_ext/a59f819d/mmxfrag.o: ../lib/x86/mmxfrag.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/mmxfrag.o ../lib/x86/mmxfrag.c

${OBJECTDIR}/_ext/a59f819d/mmxidct.o: ../lib/x86/mmxidct.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/mmxidct.o ../lib/x86/mmxidct.c

${OBJECTDIR}/_ext/a59f819d/mmxstate.o: ../lib/x86/mmxstate.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/mmxstate.o ../lib/x86/mmxstate.c

${OBJECTDIR}/_ext/a59f819d/sse2encfrag.o: ../lib/x86/sse2encfrag.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/sse2encfrag.o ../lib/x86/sse2encfrag.c

${OBJECTDIR}/_ext/a59f819d/sse2fdct.o: ../lib/x86/sse2fdct.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/sse2fdct.o ../lib/x86/sse2fdct.c

${OBJECTDIR}/_ext/a59f819d/sse2idct.o: ../lib/x86/sse2idct.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/sse2idct.o ../lib/x86/sse2idct.c

${OBJECTDIR}/_ext/a59f819d/x86cpu.o: ../lib/x86/x86cpu.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/x86cpu.o ../lib/x86/x86cpu.c

${OBJECTDIR}/_ext/a59f819d/x86enc.o: ../lib/x86/x86enc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/x86enc.o ../lib/x86/x86enc.c

${OBJECTDIR}/_ext/a59f819d/x86enquant.o: ../lib/x86/x86enquant.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/x86enquant.o ../lib/x86/x86enquant.c

${OBJECTDIR}/_ext/a59f819d/x86state.o: ../lib/x86/x86state.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a59f819d
	${RM} "$@.d"
	$(COMPILE.c) -g -DOC_X86_64_ASM -DOC_X86_ASM -I../include -I../../Ogg/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a59f819d/x86state.o ../lib/x86/x86state.c

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
