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
	${OBJECTDIR}/_ext/511e25b6/analysis.o \
	${OBJECTDIR}/_ext/511e25b6/bitrate.o \
	${OBJECTDIR}/_ext/511e25b6/block.o \
	${OBJECTDIR}/_ext/511e25b6/codebook.o \
	${OBJECTDIR}/_ext/511e25b6/envelope.o \
	${OBJECTDIR}/_ext/511e25b6/floor0.o \
	${OBJECTDIR}/_ext/511e25b6/floor1.o \
	${OBJECTDIR}/_ext/511e25b6/info.o \
	${OBJECTDIR}/_ext/511e25b6/lookup.o \
	${OBJECTDIR}/_ext/511e25b6/lpc.o \
	${OBJECTDIR}/_ext/511e25b6/lsp.o \
	${OBJECTDIR}/_ext/511e25b6/mapping0.o \
	${OBJECTDIR}/_ext/511e25b6/mdct.o \
	${OBJECTDIR}/_ext/511e25b6/psy.o \
	${OBJECTDIR}/_ext/511e25b6/registry.o \
	${OBJECTDIR}/_ext/511e25b6/res0.o \
	${OBJECTDIR}/_ext/511e25b6/sharedbook.o \
	${OBJECTDIR}/_ext/511e25b6/smallft.o \
	${OBJECTDIR}/_ext/511e25b6/synthesis.o \
	${OBJECTDIR}/_ext/511e25b6/tone.o \
	${OBJECTDIR}/_ext/511e25b6/vorbisenc.o \
	${OBJECTDIR}/_ext/511e25b6/vorbisfile.o \
	${OBJECTDIR}/_ext/511e25b6/window.o


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

${OBJECTDIR}/_ext/511e25b6/analysis.o: ../lib/analysis.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/analysis.o ../lib/analysis.c

${OBJECTDIR}/_ext/511e25b6/bitrate.o: ../lib/bitrate.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/bitrate.o ../lib/bitrate.c

${OBJECTDIR}/_ext/511e25b6/block.o: ../lib/block.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/block.o ../lib/block.c

${OBJECTDIR}/_ext/511e25b6/codebook.o: ../lib/codebook.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/codebook.o ../lib/codebook.c

${OBJECTDIR}/_ext/511e25b6/envelope.o: ../lib/envelope.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/envelope.o ../lib/envelope.c

${OBJECTDIR}/_ext/511e25b6/floor0.o: ../lib/floor0.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/floor0.o ../lib/floor0.c

${OBJECTDIR}/_ext/511e25b6/floor1.o: ../lib/floor1.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/floor1.o ../lib/floor1.c

${OBJECTDIR}/_ext/511e25b6/info.o: ../lib/info.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/info.o ../lib/info.c

${OBJECTDIR}/_ext/511e25b6/lookup.o: ../lib/lookup.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/lookup.o ../lib/lookup.c

${OBJECTDIR}/_ext/511e25b6/lpc.o: ../lib/lpc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/lpc.o ../lib/lpc.c

${OBJECTDIR}/_ext/511e25b6/lsp.o: ../lib/lsp.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/lsp.o ../lib/lsp.c

${OBJECTDIR}/_ext/511e25b6/mapping0.o: ../lib/mapping0.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/mapping0.o ../lib/mapping0.c

${OBJECTDIR}/_ext/511e25b6/mdct.o: ../lib/mdct.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/mdct.o ../lib/mdct.c

${OBJECTDIR}/_ext/511e25b6/psy.o: ../lib/psy.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/psy.o ../lib/psy.c

${OBJECTDIR}/_ext/511e25b6/registry.o: ../lib/registry.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/registry.o ../lib/registry.c

${OBJECTDIR}/_ext/511e25b6/res0.o: ../lib/res0.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/res0.o ../lib/res0.c

${OBJECTDIR}/_ext/511e25b6/sharedbook.o: ../lib/sharedbook.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/sharedbook.o ../lib/sharedbook.c

${OBJECTDIR}/_ext/511e25b6/smallft.o: ../lib/smallft.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/smallft.o ../lib/smallft.c

${OBJECTDIR}/_ext/511e25b6/synthesis.o: ../lib/synthesis.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/synthesis.o ../lib/synthesis.c

${OBJECTDIR}/_ext/511e25b6/tone.o: ../lib/tone.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/tone.o ../lib/tone.c

${OBJECTDIR}/_ext/511e25b6/vorbisenc.o: ../lib/vorbisenc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/vorbisenc.o ../lib/vorbisenc.c

${OBJECTDIR}/_ext/511e25b6/vorbisfile.o: ../lib/vorbisfile.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/vorbisfile.o ../lib/vorbisfile.c

${OBJECTDIR}/_ext/511e25b6/window.o: ../lib/window.c
	${MKDIR} -p ${OBJECTDIR}/_ext/511e25b6
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../include -I../../Ogg/include -I../lib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e25b6/window.o ../lib/window.c

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
