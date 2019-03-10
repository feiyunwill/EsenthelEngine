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
	${OBJECTDIR}/_ext/c43c7845/7zStream.o \
	${OBJECTDIR}/_ext/c43c7845/LzFind.o \
	${OBJECTDIR}/_ext/c43c7845/LzmaDec.o \
	${OBJECTDIR}/_ext/c43c7845/LzmaEnc.o


# C Compiler Flags
CFLAGS=-m64

# CC Compiler Flags
CCFLAGS=-m64
CXXFLAGS=-m64

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

${OBJECTDIR}/_ext/c43c7845/7zStream.o: ../lzma/C/7zStream.c
	${MKDIR} -p ${OBJECTDIR}/_ext/c43c7845
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -D_7ZIP_ST -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/c43c7845/7zStream.o ../lzma/C/7zStream.c

${OBJECTDIR}/_ext/c43c7845/LzFind.o: ../lzma/C/LzFind.c
	${MKDIR} -p ${OBJECTDIR}/_ext/c43c7845
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -D_7ZIP_ST -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/c43c7845/LzFind.o ../lzma/C/LzFind.c

${OBJECTDIR}/_ext/c43c7845/LzmaDec.o: ../lzma/C/LzmaDec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/c43c7845
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -D_7ZIP_ST -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/c43c7845/LzmaDec.o ../lzma/C/LzmaDec.c

${OBJECTDIR}/_ext/c43c7845/LzmaEnc.o: ../lzma/C/LzmaEnc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/c43c7845
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -D_7ZIP_ST -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/c43c7845/LzmaEnc.o ../lzma/C/LzmaEnc.c

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
