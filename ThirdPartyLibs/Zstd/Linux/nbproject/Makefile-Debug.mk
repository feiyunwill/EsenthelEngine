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
	${OBJECTDIR}/_ext/9c382c64/entropy_common.o \
	${OBJECTDIR}/_ext/9c382c64/error_private.o \
	${OBJECTDIR}/_ext/9c382c64/fse_decompress.o \
	${OBJECTDIR}/_ext/9c382c64/pool.o \
	${OBJECTDIR}/_ext/9c382c64/threading.o \
	${OBJECTDIR}/_ext/9c382c64/zstd_common.o \
	${OBJECTDIR}/_ext/6f0a339b/fse_compress.o \
	${OBJECTDIR}/_ext/6f0a339b/huf_compress.o \
	${OBJECTDIR}/_ext/6f0a339b/zstd_compress.o \
	${OBJECTDIR}/_ext/6f0a339b/zstd_double_fast.o \
	${OBJECTDIR}/_ext/6f0a339b/zstd_fast.o \
	${OBJECTDIR}/_ext/6f0a339b/zstd_lazy.o \
	${OBJECTDIR}/_ext/6f0a339b/zstd_ldm.o \
	${OBJECTDIR}/_ext/6f0a339b/zstd_opt.o \
	${OBJECTDIR}/_ext/6f0a339b/zstdmt_compress.o \
	${OBJECTDIR}/_ext/b34904dc/huf_decompress.o \
	${OBJECTDIR}/_ext/b34904dc/zstd_decompress.o \
	${OBJECTDIR}/_ext/b3c3a42c/cover.o \
	${OBJECTDIR}/_ext/b3c3a42c/divsufsort.o \
	${OBJECTDIR}/_ext/b3c3a42c/zdict.o


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

${OBJECTDIR}/_ext/9c382c64/entropy_common.o: ../lib/common/entropy_common.c
	${MKDIR} -p ${OBJECTDIR}/_ext/9c382c64
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/9c382c64/entropy_common.o ../lib/common/entropy_common.c

${OBJECTDIR}/_ext/9c382c64/error_private.o: ../lib/common/error_private.c
	${MKDIR} -p ${OBJECTDIR}/_ext/9c382c64
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/9c382c64/error_private.o ../lib/common/error_private.c

${OBJECTDIR}/_ext/9c382c64/fse_decompress.o: ../lib/common/fse_decompress.c
	${MKDIR} -p ${OBJECTDIR}/_ext/9c382c64
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/9c382c64/fse_decompress.o ../lib/common/fse_decompress.c

${OBJECTDIR}/_ext/9c382c64/pool.o: ../lib/common/pool.c
	${MKDIR} -p ${OBJECTDIR}/_ext/9c382c64
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/9c382c64/pool.o ../lib/common/pool.c

${OBJECTDIR}/_ext/9c382c64/threading.o: ../lib/common/threading.c
	${MKDIR} -p ${OBJECTDIR}/_ext/9c382c64
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/9c382c64/threading.o ../lib/common/threading.c

${OBJECTDIR}/_ext/9c382c64/zstd_common.o: ../lib/common/zstd_common.c
	${MKDIR} -p ${OBJECTDIR}/_ext/9c382c64
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/9c382c64/zstd_common.o ../lib/common/zstd_common.c

${OBJECTDIR}/_ext/6f0a339b/fse_compress.o: ../lib/compress/fse_compress.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6f0a339b
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6f0a339b/fse_compress.o ../lib/compress/fse_compress.c

${OBJECTDIR}/_ext/6f0a339b/huf_compress.o: ../lib/compress/huf_compress.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6f0a339b
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6f0a339b/huf_compress.o ../lib/compress/huf_compress.c

${OBJECTDIR}/_ext/6f0a339b/zstd_compress.o: ../lib/compress/zstd_compress.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6f0a339b
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6f0a339b/zstd_compress.o ../lib/compress/zstd_compress.c

${OBJECTDIR}/_ext/6f0a339b/zstd_double_fast.o: ../lib/compress/zstd_double_fast.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6f0a339b
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6f0a339b/zstd_double_fast.o ../lib/compress/zstd_double_fast.c

${OBJECTDIR}/_ext/6f0a339b/zstd_fast.o: ../lib/compress/zstd_fast.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6f0a339b
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6f0a339b/zstd_fast.o ../lib/compress/zstd_fast.c

${OBJECTDIR}/_ext/6f0a339b/zstd_lazy.o: ../lib/compress/zstd_lazy.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6f0a339b
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6f0a339b/zstd_lazy.o ../lib/compress/zstd_lazy.c

${OBJECTDIR}/_ext/6f0a339b/zstd_ldm.o: ../lib/compress/zstd_ldm.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6f0a339b
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6f0a339b/zstd_ldm.o ../lib/compress/zstd_ldm.c

${OBJECTDIR}/_ext/6f0a339b/zstd_opt.o: ../lib/compress/zstd_opt.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6f0a339b
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6f0a339b/zstd_opt.o ../lib/compress/zstd_opt.c

${OBJECTDIR}/_ext/6f0a339b/zstdmt_compress.o: ../lib/compress/zstdmt_compress.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6f0a339b
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6f0a339b/zstdmt_compress.o ../lib/compress/zstdmt_compress.c

${OBJECTDIR}/_ext/b34904dc/huf_decompress.o: ../lib/decompress/huf_decompress.c
	${MKDIR} -p ${OBJECTDIR}/_ext/b34904dc
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b34904dc/huf_decompress.o ../lib/decompress/huf_decompress.c

${OBJECTDIR}/_ext/b34904dc/zstd_decompress.o: ../lib/decompress/zstd_decompress.c
	${MKDIR} -p ${OBJECTDIR}/_ext/b34904dc
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b34904dc/zstd_decompress.o ../lib/decompress/zstd_decompress.c

${OBJECTDIR}/_ext/b3c3a42c/cover.o: ../lib/dictBuilder/cover.c
	${MKDIR} -p ${OBJECTDIR}/_ext/b3c3a42c
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b3c3a42c/cover.o ../lib/dictBuilder/cover.c

${OBJECTDIR}/_ext/b3c3a42c/divsufsort.o: ../lib/dictBuilder/divsufsort.c
	${MKDIR} -p ${OBJECTDIR}/_ext/b3c3a42c
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b3c3a42c/divsufsort.o ../lib/dictBuilder/divsufsort.c

${OBJECTDIR}/_ext/b3c3a42c/zdict.o: ../lib/dictBuilder/zdict.c
	${MKDIR} -p ${OBJECTDIR}/_ext/b3c3a42c
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b3c3a42c/zdict.o ../lib/dictBuilder/zdict.c

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
