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
CC=clang
CCC=clang++
CXX=clang++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
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
EE_OBJ_FILES

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m64 -fshort-wchar -fpermissive -ffast-math -ffriend-injection -Wno-int-to-pointer-cast -Wno-invalid-offsetof -Wno-comment -Wno-parentheses -Wno-switch -Wno-null-dereference -Wno-empty-body -Wno-address-of-temporary -include stdafx.h
CXXFLAGS=-m64 -fshort-wchar -fpermissive -ffast-math -ffriend-injection -Wno-int-to-pointer-cast -Wno-invalid-offsetof -Wno-comment -Wno-parentheses -Wno-switch -Wno-null-dereference -Wno-empty-body -Wno-address-of-temporary -include stdafx.h

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=EE_LIB_PATH EXTERNAL_LIBS -Wl,-rpath,Bin -lpthread -ldl -lX11 -lXi -lXinerama -lXrandr -lrt -lXmu -lGL -lopenal -lz -lodbc -ludev -lXcursor -lXxf86vm

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk EE_APP_NAME

EE_APP_NAME: EE_LIB_PATH EXTERNAL_LIBS

EE_APP_NAME: ${OBJECTFILES}
	clang++ -o EE_APP_NAME ${OBJECTFILES} ${LDLIBSOPTIONS} -static-libstdc++ -s

EE_CPP_FILES

stdafx.h.pch: stdafx.h
	${MKDIR} -p ..
	@echo Performing Custom Build Step
	clang++ -x c++-header stdafx.h -o stdafx.h.pch $(CXXFLAGS) -O3 -DDEBUG=0 -I. EE_HEADER_PATH -std=c++14

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} stdafx.h.pch
	${RM} EE_APP_NAME

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
