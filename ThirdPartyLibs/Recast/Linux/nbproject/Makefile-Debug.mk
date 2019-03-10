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
	${OBJECTDIR}/_ext/3c7eea82/DetourAlloc.o \
	${OBJECTDIR}/_ext/3c7eea82/DetourCommon.o \
	${OBJECTDIR}/_ext/3c7eea82/DetourNavMesh.o \
	${OBJECTDIR}/_ext/3c7eea82/DetourNavMeshBuilder.o \
	${OBJECTDIR}/_ext/3c7eea82/DetourNavMeshQuery.o \
	${OBJECTDIR}/_ext/3c7eea82/DetourNode.o \
	${OBJECTDIR}/_ext/52bc25a5/DetourCrowd.o \
	${OBJECTDIR}/_ext/52bc25a5/DetourLocalBoundary.o \
	${OBJECTDIR}/_ext/52bc25a5/DetourObstacleAvoidance.o \
	${OBJECTDIR}/_ext/52bc25a5/DetourPathCorridor.o \
	${OBJECTDIR}/_ext/52bc25a5/DetourPathQueue.o \
	${OBJECTDIR}/_ext/52bc25a5/DetourProximityGrid.o \
	${OBJECTDIR}/_ext/eafe92fe/DetourTileCache.o \
	${OBJECTDIR}/_ext/eafe92fe/DetourTileCacheBuilder.o \
	${OBJECTDIR}/_ext/afaeb2c9/Recast.o \
	${OBJECTDIR}/_ext/afaeb2c9/RecastAlloc.o \
	${OBJECTDIR}/_ext/afaeb2c9/RecastArea.o \
	${OBJECTDIR}/_ext/afaeb2c9/RecastContour.o \
	${OBJECTDIR}/_ext/afaeb2c9/RecastFilter.o \
	${OBJECTDIR}/_ext/afaeb2c9/RecastLayers.o \
	${OBJECTDIR}/_ext/afaeb2c9/RecastMesh.o \
	${OBJECTDIR}/_ext/afaeb2c9/RecastMeshDetail.o \
	${OBJECTDIR}/_ext/afaeb2c9/RecastRasterization.o \
	${OBJECTDIR}/_ext/afaeb2c9/RecastRegion.o


# C Compiler Flags
CFLAGS=

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

${OBJECTDIR}/_ext/3c7eea82/DetourAlloc.o: ../Detour/Source/DetourAlloc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c7eea82
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c7eea82/DetourAlloc.o ../Detour/Source/DetourAlloc.cpp

${OBJECTDIR}/_ext/3c7eea82/DetourCommon.o: ../Detour/Source/DetourCommon.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c7eea82
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c7eea82/DetourCommon.o ../Detour/Source/DetourCommon.cpp

${OBJECTDIR}/_ext/3c7eea82/DetourNavMesh.o: ../Detour/Source/DetourNavMesh.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c7eea82
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c7eea82/DetourNavMesh.o ../Detour/Source/DetourNavMesh.cpp

${OBJECTDIR}/_ext/3c7eea82/DetourNavMeshBuilder.o: ../Detour/Source/DetourNavMeshBuilder.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c7eea82
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c7eea82/DetourNavMeshBuilder.o ../Detour/Source/DetourNavMeshBuilder.cpp

${OBJECTDIR}/_ext/3c7eea82/DetourNavMeshQuery.o: ../Detour/Source/DetourNavMeshQuery.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c7eea82
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c7eea82/DetourNavMeshQuery.o ../Detour/Source/DetourNavMeshQuery.cpp

${OBJECTDIR}/_ext/3c7eea82/DetourNode.o: ../Detour/Source/DetourNode.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c7eea82
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c7eea82/DetourNode.o ../Detour/Source/DetourNode.cpp

${OBJECTDIR}/_ext/52bc25a5/DetourCrowd.o: ../DetourCrowd/Source/DetourCrowd.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/52bc25a5
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/52bc25a5/DetourCrowd.o ../DetourCrowd/Source/DetourCrowd.cpp

${OBJECTDIR}/_ext/52bc25a5/DetourLocalBoundary.o: ../DetourCrowd/Source/DetourLocalBoundary.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/52bc25a5
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/52bc25a5/DetourLocalBoundary.o ../DetourCrowd/Source/DetourLocalBoundary.cpp

${OBJECTDIR}/_ext/52bc25a5/DetourObstacleAvoidance.o: ../DetourCrowd/Source/DetourObstacleAvoidance.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/52bc25a5
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/52bc25a5/DetourObstacleAvoidance.o ../DetourCrowd/Source/DetourObstacleAvoidance.cpp

${OBJECTDIR}/_ext/52bc25a5/DetourPathCorridor.o: ../DetourCrowd/Source/DetourPathCorridor.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/52bc25a5
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/52bc25a5/DetourPathCorridor.o ../DetourCrowd/Source/DetourPathCorridor.cpp

${OBJECTDIR}/_ext/52bc25a5/DetourPathQueue.o: ../DetourCrowd/Source/DetourPathQueue.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/52bc25a5
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/52bc25a5/DetourPathQueue.o ../DetourCrowd/Source/DetourPathQueue.cpp

${OBJECTDIR}/_ext/52bc25a5/DetourProximityGrid.o: ../DetourCrowd/Source/DetourProximityGrid.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/52bc25a5
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/52bc25a5/DetourProximityGrid.o ../DetourCrowd/Source/DetourProximityGrid.cpp

${OBJECTDIR}/_ext/eafe92fe/DetourTileCache.o: ../DetourTileCache/Source/DetourTileCache.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eafe92fe
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eafe92fe/DetourTileCache.o ../DetourTileCache/Source/DetourTileCache.cpp

${OBJECTDIR}/_ext/eafe92fe/DetourTileCacheBuilder.o: ../DetourTileCache/Source/DetourTileCacheBuilder.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/eafe92fe
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/eafe92fe/DetourTileCacheBuilder.o ../DetourTileCache/Source/DetourTileCacheBuilder.cpp

${OBJECTDIR}/_ext/afaeb2c9/Recast.o: ../Recast/Source/Recast.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/afaeb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/afaeb2c9/Recast.o ../Recast/Source/Recast.cpp

${OBJECTDIR}/_ext/afaeb2c9/RecastAlloc.o: ../Recast/Source/RecastAlloc.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/afaeb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/afaeb2c9/RecastAlloc.o ../Recast/Source/RecastAlloc.cpp

${OBJECTDIR}/_ext/afaeb2c9/RecastArea.o: ../Recast/Source/RecastArea.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/afaeb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/afaeb2c9/RecastArea.o ../Recast/Source/RecastArea.cpp

${OBJECTDIR}/_ext/afaeb2c9/RecastContour.o: ../Recast/Source/RecastContour.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/afaeb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/afaeb2c9/RecastContour.o ../Recast/Source/RecastContour.cpp

${OBJECTDIR}/_ext/afaeb2c9/RecastFilter.o: ../Recast/Source/RecastFilter.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/afaeb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/afaeb2c9/RecastFilter.o ../Recast/Source/RecastFilter.cpp

${OBJECTDIR}/_ext/afaeb2c9/RecastLayers.o: ../Recast/Source/RecastLayers.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/afaeb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/afaeb2c9/RecastLayers.o ../Recast/Source/RecastLayers.cpp

${OBJECTDIR}/_ext/afaeb2c9/RecastMesh.o: ../Recast/Source/RecastMesh.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/afaeb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/afaeb2c9/RecastMesh.o ../Recast/Source/RecastMesh.cpp

${OBJECTDIR}/_ext/afaeb2c9/RecastMeshDetail.o: ../Recast/Source/RecastMeshDetail.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/afaeb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/afaeb2c9/RecastMeshDetail.o ../Recast/Source/RecastMeshDetail.cpp

${OBJECTDIR}/_ext/afaeb2c9/RecastRasterization.o: ../Recast/Source/RecastRasterization.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/afaeb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/afaeb2c9/RecastRasterization.o ../Recast/Source/RecastRasterization.cpp

${OBJECTDIR}/_ext/afaeb2c9/RecastRegion.o: ../Recast/Source/RecastRegion.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/afaeb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../DetourCrowd/Include -I../DetourTileCache/Include -I../Recast/Include -I../Detour/Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/afaeb2c9/RecastRegion.o ../Recast/Source/RecastRegion.cpp

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
