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
	${OBJECTDIR}/_ext/98fc376a/lzham_assert.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_checksum.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_huffman_codes.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_lzbase.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_lzcomp.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_lzcomp_internal.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_lzcomp_state.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_lzdecomp.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_lzdecompbase.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_match_accel.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_mem.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_platform.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_prefix_coding.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_pthreads_threading.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_symbol_codec.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_timer.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_vector.o \
	${OBJECTDIR}/_ext/98fc376a/lzham_win32_threading.o


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

${OBJECTDIR}/_ext/98fc376a/lzham_assert.o: ../Source/lzham_assert.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_assert.o ../Source/lzham_assert.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_checksum.o: ../Source/lzham_checksum.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_checksum.o ../Source/lzham_checksum.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_huffman_codes.o: ../Source/lzham_huffman_codes.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_huffman_codes.o ../Source/lzham_huffman_codes.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_lzbase.o: ../Source/lzham_lzbase.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_lzbase.o ../Source/lzham_lzbase.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_lzcomp.o: ../Source/lzham_lzcomp.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_lzcomp.o ../Source/lzham_lzcomp.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_lzcomp_internal.o: ../Source/lzham_lzcomp_internal.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_lzcomp_internal.o ../Source/lzham_lzcomp_internal.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_lzcomp_state.o: ../Source/lzham_lzcomp_state.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_lzcomp_state.o ../Source/lzham_lzcomp_state.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_lzdecomp.o: ../Source/lzham_lzdecomp.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_lzdecomp.o ../Source/lzham_lzdecomp.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_lzdecompbase.o: ../Source/lzham_lzdecompbase.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_lzdecompbase.o ../Source/lzham_lzdecompbase.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_match_accel.o: ../Source/lzham_match_accel.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_match_accel.o ../Source/lzham_match_accel.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_mem.o: ../Source/lzham_mem.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_mem.o ../Source/lzham_mem.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_platform.o: ../Source/lzham_platform.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_platform.o ../Source/lzham_platform.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_prefix_coding.o: ../Source/lzham_prefix_coding.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_prefix_coding.o ../Source/lzham_prefix_coding.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_pthreads_threading.o: ../Source/lzham_pthreads_threading.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_pthreads_threading.o ../Source/lzham_pthreads_threading.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_symbol_codec.o: ../Source/lzham_symbol_codec.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_symbol_codec.o ../Source/lzham_symbol_codec.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_timer.o: ../Source/lzham_timer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_timer.o ../Source/lzham_timer.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_vector.o: ../Source/lzham_vector.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_vector.o ../Source/lzham_vector.cpp

${OBJECTDIR}/_ext/98fc376a/lzham_win32_threading.o: ../Source/lzham_win32_threading.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/lzham_win32_threading.o ../Source/lzham_win32_threading.cpp

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
