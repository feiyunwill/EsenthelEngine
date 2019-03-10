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
	${OBJECTDIR}/_ext/a4bb875c/filter_sse2_intrinsics.o \
	${OBJECTDIR}/_ext/a4bb875c/intel_init.o \
	${OBJECTDIR}/_ext/511e4115/png.o \
	${OBJECTDIR}/_ext/511e4115/pngerror.o \
	${OBJECTDIR}/_ext/511e4115/pngget.o \
	${OBJECTDIR}/_ext/511e4115/pngmem.o \
	${OBJECTDIR}/_ext/511e4115/pngpread.o \
	${OBJECTDIR}/_ext/511e4115/pngread.o \
	${OBJECTDIR}/_ext/511e4115/pngrio.o \
	${OBJECTDIR}/_ext/511e4115/pngrtran.o \
	${OBJECTDIR}/_ext/511e4115/pngrutil.o \
	${OBJECTDIR}/_ext/511e4115/pngset.o \
	${OBJECTDIR}/_ext/511e4115/pngtrans.o \
	${OBJECTDIR}/_ext/511e4115/pngwio.o \
	${OBJECTDIR}/_ext/511e4115/pngwrite.o \
	${OBJECTDIR}/_ext/511e4115/pngwtran.o \
	${OBJECTDIR}/_ext/511e4115/pngwutil.o


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

${OBJECTDIR}/_ext/a4bb875c/filter_sse2_intrinsics.o: ../src/intel/filter_sse2_intrinsics.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a4bb875c
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a4bb875c/filter_sse2_intrinsics.o ../src/intel/filter_sse2_intrinsics.c

${OBJECTDIR}/_ext/a4bb875c/intel_init.o: ../src/intel/intel_init.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a4bb875c
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a4bb875c/intel_init.o ../src/intel/intel_init.c

${OBJECTDIR}/_ext/511e4115/png.o: ../src/png.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/png.o ../src/png.c

${OBJECTDIR}/_ext/511e4115/pngerror.o: ../src/pngerror.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngerror.o ../src/pngerror.c

${OBJECTDIR}/_ext/511e4115/pngget.o: ../src/pngget.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngget.o ../src/pngget.c

${OBJECTDIR}/_ext/511e4115/pngmem.o: ../src/pngmem.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngmem.o ../src/pngmem.c

${OBJECTDIR}/_ext/511e4115/pngpread.o: ../src/pngpread.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngpread.o ../src/pngpread.c

${OBJECTDIR}/_ext/511e4115/pngread.o: ../src/pngread.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngread.o ../src/pngread.c

${OBJECTDIR}/_ext/511e4115/pngrio.o: ../src/pngrio.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngrio.o ../src/pngrio.c

${OBJECTDIR}/_ext/511e4115/pngrtran.o: ../src/pngrtran.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngrtran.o ../src/pngrtran.c

${OBJECTDIR}/_ext/511e4115/pngrutil.o: ../src/pngrutil.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngrutil.o ../src/pngrutil.c

${OBJECTDIR}/_ext/511e4115/pngset.o: ../src/pngset.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngset.o ../src/pngset.c

${OBJECTDIR}/_ext/511e4115/pngtrans.o: ../src/pngtrans.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngtrans.o ../src/pngtrans.c

${OBJECTDIR}/_ext/511e4115/pngwio.o: ../src/pngwio.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngwio.o ../src/pngwio.c

${OBJECTDIR}/_ext/511e4115/pngwrite.o: ../src/pngwrite.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngwrite.o ../src/pngwrite.c

${OBJECTDIR}/_ext/511e4115/pngwtran.o: ../src/pngwtran.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngwtran.o ../src/pngwtran.c

${OBJECTDIR}/_ext/511e4115/pngwutil.o: ../src/pngwutil.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DPNG_INTEL_SSE -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pngwutil.o ../src/pngwutil.c

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
