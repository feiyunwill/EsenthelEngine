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
	${OBJECTDIR}/_ext/a5beacab/mkg3states.o \
	${OBJECTDIR}/_ext/a5beacab/tif_aux.o \
	${OBJECTDIR}/_ext/a5beacab/tif_close.o \
	${OBJECTDIR}/_ext/a5beacab/tif_codec.o \
	${OBJECTDIR}/_ext/a5beacab/tif_color.o \
	${OBJECTDIR}/_ext/a5beacab/tif_compress.o \
	${OBJECTDIR}/_ext/a5beacab/tif_dir.o \
	${OBJECTDIR}/_ext/a5beacab/tif_dirinfo.o \
	${OBJECTDIR}/_ext/a5beacab/tif_dirread.o \
	${OBJECTDIR}/_ext/a5beacab/tif_dirwrite.o \
	${OBJECTDIR}/_ext/a5beacab/tif_dumpmode.o \
	${OBJECTDIR}/_ext/a5beacab/tif_error.o \
	${OBJECTDIR}/_ext/a5beacab/tif_extension.o \
	${OBJECTDIR}/_ext/a5beacab/tif_fax3.o \
	${OBJECTDIR}/_ext/a5beacab/tif_fax3sm.o \
	${OBJECTDIR}/_ext/a5beacab/tif_flush.o \
	${OBJECTDIR}/_ext/a5beacab/tif_getimage.o \
	${OBJECTDIR}/_ext/a5beacab/tif_jbig.o \
	${OBJECTDIR}/_ext/a5beacab/tif_jpeg.o \
	${OBJECTDIR}/_ext/a5beacab/tif_jpeg_12.o \
	${OBJECTDIR}/_ext/a5beacab/tif_luv.o \
	${OBJECTDIR}/_ext/a5beacab/tif_lzma.o \
	${OBJECTDIR}/_ext/a5beacab/tif_lzw.o \
	${OBJECTDIR}/_ext/a5beacab/tif_next.o \
	${OBJECTDIR}/_ext/a5beacab/tif_ojpeg.o \
	${OBJECTDIR}/_ext/a5beacab/tif_open.o \
	${OBJECTDIR}/_ext/a5beacab/tif_packbits.o \
	${OBJECTDIR}/_ext/a5beacab/tif_pixarlog.o \
	${OBJECTDIR}/_ext/a5beacab/tif_predict.o \
	${OBJECTDIR}/_ext/a5beacab/tif_print.o \
	${OBJECTDIR}/_ext/a5beacab/tif_read.o \
	${OBJECTDIR}/_ext/a5beacab/tif_strip.o \
	${OBJECTDIR}/_ext/a5beacab/tif_swab.o \
	${OBJECTDIR}/_ext/a5beacab/tif_thunder.o \
	${OBJECTDIR}/_ext/a5beacab/tif_tile.o \
	${OBJECTDIR}/_ext/a5beacab/tif_unix.o \
	${OBJECTDIR}/_ext/a5beacab/tif_version.o \
	${OBJECTDIR}/_ext/a5beacab/tif_warning.o \
	${OBJECTDIR}/_ext/a5beacab/tif_write.o \
	${OBJECTDIR}/_ext/a5beacab/tif_zip.o


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

${OBJECTDIR}/_ext/a5beacab/mkg3states.o: ../libtiff/mkg3states.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/mkg3states.o ../libtiff/mkg3states.c

${OBJECTDIR}/_ext/a5beacab/tif_aux.o: ../libtiff/tif_aux.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_aux.o ../libtiff/tif_aux.c

${OBJECTDIR}/_ext/a5beacab/tif_close.o: ../libtiff/tif_close.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_close.o ../libtiff/tif_close.c

${OBJECTDIR}/_ext/a5beacab/tif_codec.o: ../libtiff/tif_codec.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_codec.o ../libtiff/tif_codec.c

${OBJECTDIR}/_ext/a5beacab/tif_color.o: ../libtiff/tif_color.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_color.o ../libtiff/tif_color.c

${OBJECTDIR}/_ext/a5beacab/tif_compress.o: ../libtiff/tif_compress.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_compress.o ../libtiff/tif_compress.c

${OBJECTDIR}/_ext/a5beacab/tif_dir.o: ../libtiff/tif_dir.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_dir.o ../libtiff/tif_dir.c

${OBJECTDIR}/_ext/a5beacab/tif_dirinfo.o: ../libtiff/tif_dirinfo.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_dirinfo.o ../libtiff/tif_dirinfo.c

${OBJECTDIR}/_ext/a5beacab/tif_dirread.o: ../libtiff/tif_dirread.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_dirread.o ../libtiff/tif_dirread.c

${OBJECTDIR}/_ext/a5beacab/tif_dirwrite.o: ../libtiff/tif_dirwrite.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_dirwrite.o ../libtiff/tif_dirwrite.c

${OBJECTDIR}/_ext/a5beacab/tif_dumpmode.o: ../libtiff/tif_dumpmode.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_dumpmode.o ../libtiff/tif_dumpmode.c

${OBJECTDIR}/_ext/a5beacab/tif_error.o: ../libtiff/tif_error.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_error.o ../libtiff/tif_error.c

${OBJECTDIR}/_ext/a5beacab/tif_extension.o: ../libtiff/tif_extension.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_extension.o ../libtiff/tif_extension.c

${OBJECTDIR}/_ext/a5beacab/tif_fax3.o: ../libtiff/tif_fax3.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_fax3.o ../libtiff/tif_fax3.c

${OBJECTDIR}/_ext/a5beacab/tif_fax3sm.o: ../libtiff/tif_fax3sm.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_fax3sm.o ../libtiff/tif_fax3sm.c

${OBJECTDIR}/_ext/a5beacab/tif_flush.o: ../libtiff/tif_flush.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_flush.o ../libtiff/tif_flush.c

${OBJECTDIR}/_ext/a5beacab/tif_getimage.o: ../libtiff/tif_getimage.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_getimage.o ../libtiff/tif_getimage.c

${OBJECTDIR}/_ext/a5beacab/tif_jbig.o: ../libtiff/tif_jbig.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_jbig.o ../libtiff/tif_jbig.c

${OBJECTDIR}/_ext/a5beacab/tif_jpeg.o: ../libtiff/tif_jpeg.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_jpeg.o ../libtiff/tif_jpeg.c

${OBJECTDIR}/_ext/a5beacab/tif_jpeg_12.o: ../libtiff/tif_jpeg_12.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_jpeg_12.o ../libtiff/tif_jpeg_12.c

${OBJECTDIR}/_ext/a5beacab/tif_luv.o: ../libtiff/tif_luv.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_luv.o ../libtiff/tif_luv.c

${OBJECTDIR}/_ext/a5beacab/tif_lzma.o: ../libtiff/tif_lzma.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_lzma.o ../libtiff/tif_lzma.c

${OBJECTDIR}/_ext/a5beacab/tif_lzw.o: ../libtiff/tif_lzw.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_lzw.o ../libtiff/tif_lzw.c

${OBJECTDIR}/_ext/a5beacab/tif_next.o: ../libtiff/tif_next.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_next.o ../libtiff/tif_next.c

${OBJECTDIR}/_ext/a5beacab/tif_ojpeg.o: ../libtiff/tif_ojpeg.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_ojpeg.o ../libtiff/tif_ojpeg.c

${OBJECTDIR}/_ext/a5beacab/tif_open.o: ../libtiff/tif_open.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_open.o ../libtiff/tif_open.c

${OBJECTDIR}/_ext/a5beacab/tif_packbits.o: ../libtiff/tif_packbits.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_packbits.o ../libtiff/tif_packbits.c

${OBJECTDIR}/_ext/a5beacab/tif_pixarlog.o: ../libtiff/tif_pixarlog.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_pixarlog.o ../libtiff/tif_pixarlog.c

${OBJECTDIR}/_ext/a5beacab/tif_predict.o: ../libtiff/tif_predict.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_predict.o ../libtiff/tif_predict.c

${OBJECTDIR}/_ext/a5beacab/tif_print.o: ../libtiff/tif_print.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_print.o ../libtiff/tif_print.c

${OBJECTDIR}/_ext/a5beacab/tif_read.o: ../libtiff/tif_read.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_read.o ../libtiff/tif_read.c

${OBJECTDIR}/_ext/a5beacab/tif_strip.o: ../libtiff/tif_strip.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_strip.o ../libtiff/tif_strip.c

${OBJECTDIR}/_ext/a5beacab/tif_swab.o: ../libtiff/tif_swab.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_swab.o ../libtiff/tif_swab.c

${OBJECTDIR}/_ext/a5beacab/tif_thunder.o: ../libtiff/tif_thunder.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_thunder.o ../libtiff/tif_thunder.c

${OBJECTDIR}/_ext/a5beacab/tif_tile.o: ../libtiff/tif_tile.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_tile.o ../libtiff/tif_tile.c

${OBJECTDIR}/_ext/a5beacab/tif_unix.o: ../libtiff/tif_unix.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_unix.o ../libtiff/tif_unix.c

${OBJECTDIR}/_ext/a5beacab/tif_version.o: ../libtiff/tif_version.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_version.o ../libtiff/tif_version.c

${OBJECTDIR}/_ext/a5beacab/tif_warning.o: ../libtiff/tif_warning.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_warning.o ../libtiff/tif_warning.c

${OBJECTDIR}/_ext/a5beacab/tif_write.o: ../libtiff/tif_write.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_write.o ../libtiff/tif_write.c

${OBJECTDIR}/_ext/a5beacab/tif_zip.o: ../libtiff/tif_zip.c
	${MKDIR} -p ${OBJECTDIR}/_ext/a5beacab
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -std=c11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a5beacab/tif_zip.o ../libtiff/tif_zip.c

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
