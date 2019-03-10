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
	${OBJECTDIR}/_ext/6c8b93fe/autofit.o \
	${OBJECTDIR}/_ext/7723f055/ftbase.o \
	${OBJECTDIR}/_ext/7723f055/ftbbox.o \
	${OBJECTDIR}/_ext/7723f055/ftbitmap.o \
	${OBJECTDIR}/_ext/7723f055/ftdebug.o \
	${OBJECTDIR}/_ext/7723f055/ftfntfmt.o \
	${OBJECTDIR}/_ext/7723f055/ftfstype.o \
	${OBJECTDIR}/_ext/7723f055/ftgasp.o \
	${OBJECTDIR}/_ext/7723f055/ftglyph.o \
	${OBJECTDIR}/_ext/7723f055/ftgxval.o \
	${OBJECTDIR}/_ext/7723f055/ftinit.o \
	${OBJECTDIR}/_ext/7723f055/ftlcdfil.o \
	${OBJECTDIR}/_ext/7723f055/ftmm.o \
	${OBJECTDIR}/_ext/7723f055/ftotval.o \
	${OBJECTDIR}/_ext/7723f055/ftpatent.o \
	${OBJECTDIR}/_ext/7723f055/ftpfr.o \
	${OBJECTDIR}/_ext/7723f055/ftstroke.o \
	${OBJECTDIR}/_ext/7723f055/ftsynth.o \
	${OBJECTDIR}/_ext/7723f055/ftsystem.o \
	${OBJECTDIR}/_ext/7723f055/fttype1.o \
	${OBJECTDIR}/_ext/7723f055/ftwinfnt.o \
	${OBJECTDIR}/_ext/3da65260/bdf.o \
	${OBJECTDIR}/_ext/6d67f67e/ftccache.o \
	${OBJECTDIR}/_ext/3da6565f/cff.o \
	${OBJECTDIR}/_ext/3da656ba/type1cid.o \
	${OBJECTDIR}/_ext/772692de/ftgzip.o \
	${OBJECTDIR}/_ext/3da67aa5/ftlzw.o \
	${OBJECTDIR}/_ext/3da686cf/pcf.o \
	${OBJECTDIR}/_ext/3da68738/pfr.o \
	${OBJECTDIR}/_ext/6e2750dd/psaux.o \
	${OBJECTDIR}/_ext/bc812cbb/pshinter.o \
	${OBJECTDIR}/_ext/82448b21/psmodule.o \
	${OBJECTDIR}/_ext/5936fe01/raster.o \
	${OBJECTDIR}/_ext/772bbcdd/sfnt.o \
	${OBJECTDIR}/_ext/5b930e32/smooth.o \
	${OBJECTDIR}/_ext/3d19154c/truetype.o \
	${OBJECTDIR}/_ext/6e629f33/type1.o \
	${OBJECTDIR}/_ext/5df147bc/type42.o \
	${OBJECTDIR}/_ext/8cbe432c/winfnt.o


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

${OBJECTDIR}/_ext/6c8b93fe/autofit.o: ../lib/src/autofit/autofit.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6c8b93fe
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6c8b93fe/autofit.o ../lib/src/autofit/autofit.c

${OBJECTDIR}/_ext/7723f055/ftbase.o: ../lib/src/base/ftbase.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftbase.o ../lib/src/base/ftbase.c

${OBJECTDIR}/_ext/7723f055/ftbbox.o: ../lib/src/base/ftbbox.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftbbox.o ../lib/src/base/ftbbox.c

${OBJECTDIR}/_ext/7723f055/ftbitmap.o: ../lib/src/base/ftbitmap.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftbitmap.o ../lib/src/base/ftbitmap.c

${OBJECTDIR}/_ext/7723f055/ftdebug.o: ../lib/src/base/ftdebug.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftdebug.o ../lib/src/base/ftdebug.c

${OBJECTDIR}/_ext/7723f055/ftfntfmt.o: ../lib/src/base/ftfntfmt.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftfntfmt.o ../lib/src/base/ftfntfmt.c

${OBJECTDIR}/_ext/7723f055/ftfstype.o: ../lib/src/base/ftfstype.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftfstype.o ../lib/src/base/ftfstype.c

${OBJECTDIR}/_ext/7723f055/ftgasp.o: ../lib/src/base/ftgasp.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftgasp.o ../lib/src/base/ftgasp.c

${OBJECTDIR}/_ext/7723f055/ftglyph.o: ../lib/src/base/ftglyph.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftglyph.o ../lib/src/base/ftglyph.c

${OBJECTDIR}/_ext/7723f055/ftgxval.o: ../lib/src/base/ftgxval.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftgxval.o ../lib/src/base/ftgxval.c

${OBJECTDIR}/_ext/7723f055/ftinit.o: ../lib/src/base/ftinit.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftinit.o ../lib/src/base/ftinit.c

${OBJECTDIR}/_ext/7723f055/ftlcdfil.o: ../lib/src/base/ftlcdfil.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftlcdfil.o ../lib/src/base/ftlcdfil.c

${OBJECTDIR}/_ext/7723f055/ftmm.o: ../lib/src/base/ftmm.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftmm.o ../lib/src/base/ftmm.c

${OBJECTDIR}/_ext/7723f055/ftotval.o: ../lib/src/base/ftotval.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftotval.o ../lib/src/base/ftotval.c

${OBJECTDIR}/_ext/7723f055/ftpatent.o: ../lib/src/base/ftpatent.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftpatent.o ../lib/src/base/ftpatent.c

${OBJECTDIR}/_ext/7723f055/ftpfr.o: ../lib/src/base/ftpfr.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftpfr.o ../lib/src/base/ftpfr.c

${OBJECTDIR}/_ext/7723f055/ftstroke.o: ../lib/src/base/ftstroke.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftstroke.o ../lib/src/base/ftstroke.c

${OBJECTDIR}/_ext/7723f055/ftsynth.o: ../lib/src/base/ftsynth.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftsynth.o ../lib/src/base/ftsynth.c

${OBJECTDIR}/_ext/7723f055/ftsystem.o: ../lib/src/base/ftsystem.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftsystem.o ../lib/src/base/ftsystem.c

${OBJECTDIR}/_ext/7723f055/fttype1.o: ../lib/src/base/fttype1.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/fttype1.o ../lib/src/base/fttype1.c

${OBJECTDIR}/_ext/7723f055/ftwinfnt.o: ../lib/src/base/ftwinfnt.c
	${MKDIR} -p ${OBJECTDIR}/_ext/7723f055
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/7723f055/ftwinfnt.o ../lib/src/base/ftwinfnt.c

${OBJECTDIR}/_ext/3da65260/bdf.o: ../lib/src/bdf/bdf.c
	${MKDIR} -p ${OBJECTDIR}/_ext/3da65260
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3da65260/bdf.o ../lib/src/bdf/bdf.c

${OBJECTDIR}/_ext/6d67f67e/ftccache.o: ../lib/src/cache/ftccache.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6d67f67e
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6d67f67e/ftccache.o ../lib/src/cache/ftccache.c

${OBJECTDIR}/_ext/3da6565f/cff.o: ../lib/src/cff/cff.c
	${MKDIR} -p ${OBJECTDIR}/_ext/3da6565f
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3da6565f/cff.o ../lib/src/cff/cff.c

${OBJECTDIR}/_ext/3da656ba/type1cid.o: ../lib/src/cid/type1cid.c
	${MKDIR} -p ${OBJECTDIR}/_ext/3da656ba
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3da656ba/type1cid.o ../lib/src/cid/type1cid.c

${OBJECTDIR}/_ext/772692de/ftgzip.o: ../lib/src/gzip/ftgzip.c
	${MKDIR} -p ${OBJECTDIR}/_ext/772692de
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/772692de/ftgzip.o ../lib/src/gzip/ftgzip.c

${OBJECTDIR}/_ext/3da67aa5/ftlzw.o: ../lib/src/lzw/ftlzw.c
	${MKDIR} -p ${OBJECTDIR}/_ext/3da67aa5
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3da67aa5/ftlzw.o ../lib/src/lzw/ftlzw.c

${OBJECTDIR}/_ext/3da686cf/pcf.o: ../lib/src/pcf/pcf.c
	${MKDIR} -p ${OBJECTDIR}/_ext/3da686cf
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3da686cf/pcf.o ../lib/src/pcf/pcf.c

${OBJECTDIR}/_ext/3da68738/pfr.o: ../lib/src/pfr/pfr.c
	${MKDIR} -p ${OBJECTDIR}/_ext/3da68738
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3da68738/pfr.o ../lib/src/pfr/pfr.c

${OBJECTDIR}/_ext/6e2750dd/psaux.o: ../lib/src/psaux/psaux.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6e2750dd
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e2750dd/psaux.o ../lib/src/psaux/psaux.c

${OBJECTDIR}/_ext/bc812cbb/pshinter.o: ../lib/src/pshinter/pshinter.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc812cbb
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc812cbb/pshinter.o ../lib/src/pshinter/pshinter.c

${OBJECTDIR}/_ext/82448b21/psmodule.o: ../lib/src/psnames/psmodule.c
	${MKDIR} -p ${OBJECTDIR}/_ext/82448b21
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/82448b21/psmodule.o ../lib/src/psnames/psmodule.c

${OBJECTDIR}/_ext/5936fe01/raster.o: ../lib/src/raster/raster.c
	${MKDIR} -p ${OBJECTDIR}/_ext/5936fe01
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5936fe01/raster.o ../lib/src/raster/raster.c

${OBJECTDIR}/_ext/772bbcdd/sfnt.o: ../lib/src/sfnt/sfnt.c
	${MKDIR} -p ${OBJECTDIR}/_ext/772bbcdd
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/772bbcdd/sfnt.o ../lib/src/sfnt/sfnt.c

${OBJECTDIR}/_ext/5b930e32/smooth.o: ../lib/src/smooth/smooth.c
	${MKDIR} -p ${OBJECTDIR}/_ext/5b930e32
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5b930e32/smooth.o ../lib/src/smooth/smooth.c

${OBJECTDIR}/_ext/3d19154c/truetype.o: ../lib/src/truetype/truetype.c
	${MKDIR} -p ${OBJECTDIR}/_ext/3d19154c
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3d19154c/truetype.o ../lib/src/truetype/truetype.c

${OBJECTDIR}/_ext/6e629f33/type1.o: ../lib/src/type1/type1.c
	${MKDIR} -p ${OBJECTDIR}/_ext/6e629f33
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e629f33/type1.o ../lib/src/type1/type1.c

${OBJECTDIR}/_ext/5df147bc/type42.o: ../lib/src/type42/type42.c
	${MKDIR} -p ${OBJECTDIR}/_ext/5df147bc
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5df147bc/type42.o ../lib/src/type42/type42.c

${OBJECTDIR}/_ext/8cbe432c/winfnt.o: ../lib/src/winfonts/winfnt.c
	${MKDIR} -p ${OBJECTDIR}/_ext/8cbe432c
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -DFT2_BUILD_LIBRARY -DNDEBUG -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8cbe432c/winfnt.o ../lib/src/winfonts/winfnt.c

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
