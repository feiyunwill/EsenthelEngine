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
	${OBJECTDIR}/_ext/d0be1876/btAxisSweep3.o \
	${OBJECTDIR}/_ext/d0be1876/btBroadphaseProxy.o \
	${OBJECTDIR}/_ext/d0be1876/btCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/d0be1876/btDbvt.o \
	${OBJECTDIR}/_ext/d0be1876/btDbvtBroadphase.o \
	${OBJECTDIR}/_ext/d0be1876/btDispatcher.o \
	${OBJECTDIR}/_ext/d0be1876/btOverlappingPairCache.o \
	${OBJECTDIR}/_ext/d0be1876/btQuantizedBvh.o \
	${OBJECTDIR}/_ext/d0be1876/btSimpleBroadphase.o \
	${OBJECTDIR}/_ext/3c6b2689/SphereTriangleDetector.o \
	${OBJECTDIR}/_ext/3c6b2689/btActivatingCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btBox2dBox2dCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btBoxBoxCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btBoxBoxDetector.o \
	${OBJECTDIR}/_ext/3c6b2689/btCollisionDispatcher.o \
	${OBJECTDIR}/_ext/3c6b2689/btCollisionDispatcherMt.o \
	${OBJECTDIR}/_ext/3c6b2689/btCollisionObject.o \
	${OBJECTDIR}/_ext/3c6b2689/btCollisionWorld.o \
	${OBJECTDIR}/_ext/3c6b2689/btCollisionWorldImporter.o \
	${OBJECTDIR}/_ext/3c6b2689/btCompoundCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btCompoundCompoundCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btConvex2dConvex2dAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btConvexConcaveCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btConvexConvexAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btConvexPlaneCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btDefaultCollisionConfiguration.o \
	${OBJECTDIR}/_ext/3c6b2689/btEmptyCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btGhostObject.o \
	${OBJECTDIR}/_ext/3c6b2689/btHashedSimplePairCache.o \
	${OBJECTDIR}/_ext/3c6b2689/btInternalEdgeUtility.o \
	${OBJECTDIR}/_ext/3c6b2689/btManifoldResult.o \
	${OBJECTDIR}/_ext/3c6b2689/btSimulationIslandManager.o \
	${OBJECTDIR}/_ext/3c6b2689/btSphereBoxCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btSphereSphereCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btSphereTriangleCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/3c6b2689/btUnionFind.o \
	${OBJECTDIR}/_ext/8575bec1/btBox2dShape.o \
	${OBJECTDIR}/_ext/8575bec1/btBoxShape.o \
	${OBJECTDIR}/_ext/8575bec1/btBvhTriangleMeshShape.o \
	${OBJECTDIR}/_ext/8575bec1/btCapsuleShape.o \
	${OBJECTDIR}/_ext/8575bec1/btCollisionShape.o \
	${OBJECTDIR}/_ext/8575bec1/btCompoundShape.o \
	${OBJECTDIR}/_ext/8575bec1/btConcaveShape.o \
	${OBJECTDIR}/_ext/8575bec1/btConeShape.o \
	${OBJECTDIR}/_ext/8575bec1/btConvex2dShape.o \
	${OBJECTDIR}/_ext/8575bec1/btConvexHullShape.o \
	${OBJECTDIR}/_ext/8575bec1/btConvexInternalShape.o \
	${OBJECTDIR}/_ext/8575bec1/btConvexPointCloudShape.o \
	${OBJECTDIR}/_ext/8575bec1/btConvexPolyhedron.o \
	${OBJECTDIR}/_ext/8575bec1/btConvexShape.o \
	${OBJECTDIR}/_ext/8575bec1/btConvexTriangleMeshShape.o \
	${OBJECTDIR}/_ext/8575bec1/btCylinderShape.o \
	${OBJECTDIR}/_ext/8575bec1/btEmptyShape.o \
	${OBJECTDIR}/_ext/8575bec1/btHeightfieldTerrainShape.o \
	${OBJECTDIR}/_ext/8575bec1/btMinkowskiSumShape.o \
	${OBJECTDIR}/_ext/8575bec1/btMultiSphereShape.o \
	${OBJECTDIR}/_ext/8575bec1/btMultimaterialTriangleMeshShape.o \
	${OBJECTDIR}/_ext/8575bec1/btOptimizedBvh.o \
	${OBJECTDIR}/_ext/8575bec1/btPolyhedralConvexShape.o \
	${OBJECTDIR}/_ext/8575bec1/btScaledBvhTriangleMeshShape.o \
	${OBJECTDIR}/_ext/8575bec1/btShapeHull.o \
	${OBJECTDIR}/_ext/8575bec1/btSphereShape.o \
	${OBJECTDIR}/_ext/8575bec1/btStaticPlaneShape.o \
	${OBJECTDIR}/_ext/8575bec1/btStridingMeshInterface.o \
	${OBJECTDIR}/_ext/8575bec1/btTetrahedronShape.o \
	${OBJECTDIR}/_ext/8575bec1/btTriangleBuffer.o \
	${OBJECTDIR}/_ext/8575bec1/btTriangleCallback.o \
	${OBJECTDIR}/_ext/8575bec1/btTriangleIndexVertexArray.o \
	${OBJECTDIR}/_ext/8575bec1/btTriangleIndexVertexMaterialArray.o \
	${OBJECTDIR}/_ext/8575bec1/btTriangleMesh.o \
	${OBJECTDIR}/_ext/8575bec1/btTriangleMeshShape.o \
	${OBJECTDIR}/_ext/8575bec1/btUniformScalingShape.o \
	${OBJECTDIR}/_ext/1983f56a/btContactProcessing.o \
	${OBJECTDIR}/_ext/1983f56a/btGImpactBvh.o \
	${OBJECTDIR}/_ext/1983f56a/btGImpactCollisionAlgorithm.o \
	${OBJECTDIR}/_ext/1983f56a/btGImpactQuantizedBvh.o \
	${OBJECTDIR}/_ext/1983f56a/btGImpactShape.o \
	${OBJECTDIR}/_ext/1983f56a/btGenericPoolAllocator.o \
	${OBJECTDIR}/_ext/1983f56a/btTriangleShapeEx.o \
	${OBJECTDIR}/_ext/1983f56a/gim_box_set.o \
	${OBJECTDIR}/_ext/1983f56a/gim_contact.o \
	${OBJECTDIR}/_ext/1983f56a/gim_memory.o \
	${OBJECTDIR}/_ext/1983f56a/gim_tri_collision.o \
	${OBJECTDIR}/_ext/6e925355/btContinuousConvexCollision.o \
	${OBJECTDIR}/_ext/6e925355/btConvexCast.o \
	${OBJECTDIR}/_ext/6e925355/btGjkConvexCast.o \
	${OBJECTDIR}/_ext/6e925355/btGjkEpa2.o \
	${OBJECTDIR}/_ext/6e925355/btGjkEpaPenetrationDepthSolver.o \
	${OBJECTDIR}/_ext/6e925355/btGjkPairDetector.o \
	${OBJECTDIR}/_ext/6e925355/btMinkowskiPenetrationDepthSolver.o \
	${OBJECTDIR}/_ext/6e925355/btPersistentManifold.o \
	${OBJECTDIR}/_ext/6e925355/btPolyhedralContactClipping.o \
	${OBJECTDIR}/_ext/6e925355/btRaycastCallback.o \
	${OBJECTDIR}/_ext/6e925355/btSubSimplexConvexCast.o \
	${OBJECTDIR}/_ext/6e925355/btVoronoiSimplexSolver.o \
	${OBJECTDIR}/_ext/80780714/btKinematicCharacterController.o \
	${OBJECTDIR}/_ext/12134e05/btConeTwistConstraint.o \
	${OBJECTDIR}/_ext/12134e05/btContactConstraint.o \
	${OBJECTDIR}/_ext/12134e05/btFixedConstraint.o \
	${OBJECTDIR}/_ext/12134e05/btGearConstraint.o \
	${OBJECTDIR}/_ext/12134e05/btGeneric6DofConstraint.o \
	${OBJECTDIR}/_ext/12134e05/btGeneric6DofSpring2Constraint.o \
	${OBJECTDIR}/_ext/12134e05/btGeneric6DofSpringConstraint.o \
	${OBJECTDIR}/_ext/12134e05/btHinge2Constraint.o \
	${OBJECTDIR}/_ext/12134e05/btHingeConstraint.o \
	${OBJECTDIR}/_ext/12134e05/btNNCGConstraintSolver.o \
	${OBJECTDIR}/_ext/12134e05/btPoint2PointConstraint.o \
	${OBJECTDIR}/_ext/12134e05/btSequentialImpulseConstraintSolver.o \
	${OBJECTDIR}/_ext/12134e05/btSliderConstraint.o \
	${OBJECTDIR}/_ext/12134e05/btSolve2LinearConstraint.o \
	${OBJECTDIR}/_ext/12134e05/btTypedConstraint.o \
	${OBJECTDIR}/_ext/12134e05/btUniversalConstraint.o \
	${OBJECTDIR}/_ext/5f6bb2c9/btDiscreteDynamicsWorld.o \
	${OBJECTDIR}/_ext/5f6bb2c9/btDiscreteDynamicsWorldMt.o \
	${OBJECTDIR}/_ext/5f6bb2c9/btRigidBody.o \
	${OBJECTDIR}/_ext/5f6bb2c9/btSimpleDynamicsWorld.o \
	${OBJECTDIR}/_ext/5f6bb2c9/btSimulationIslandManagerMt.o \
	${OBJECTDIR}/_ext/cf0245d7/btMultiBody.o \
	${OBJECTDIR}/_ext/cf0245d7/btMultiBodyConstraint.o \
	${OBJECTDIR}/_ext/cf0245d7/btMultiBodyConstraintSolver.o \
	${OBJECTDIR}/_ext/cf0245d7/btMultiBodyDynamicsWorld.o \
	${OBJECTDIR}/_ext/cf0245d7/btMultiBodyFixedConstraint.o \
	${OBJECTDIR}/_ext/cf0245d7/btMultiBodyGearConstraint.o \
	${OBJECTDIR}/_ext/cf0245d7/btMultiBodyJointLimitConstraint.o \
	${OBJECTDIR}/_ext/cf0245d7/btMultiBodyJointMotor.o \
	${OBJECTDIR}/_ext/cf0245d7/btMultiBodyPoint2Point.o \
	${OBJECTDIR}/_ext/cf0245d7/btMultiBodySliderConstraint.o \
	${OBJECTDIR}/_ext/5f05fc1f/btDantzigLCP.o \
	${OBJECTDIR}/_ext/5f05fc1f/btLemkeAlgorithm.o \
	${OBJECTDIR}/_ext/5f05fc1f/btMLCPSolver.o \
	${OBJECTDIR}/_ext/6f87d9d7/btRaycastVehicle.o \
	${OBJECTDIR}/_ext/6f87d9d7/btWheelInfo.o \
	${OBJECTDIR}/_ext/1408271/btAlignedAllocator.o \
	${OBJECTDIR}/_ext/1408271/btConvexHull.o \
	${OBJECTDIR}/_ext/1408271/btConvexHullComputer.o \
	${OBJECTDIR}/_ext/1408271/btGeometryUtil.o \
	${OBJECTDIR}/_ext/1408271/btPolarDecomposition.o \
	${OBJECTDIR}/_ext/1408271/btQuickprof.o \
	${OBJECTDIR}/_ext/1408271/btSerializer.o \
	${OBJECTDIR}/_ext/1408271/btSerializer64.o \
	${OBJECTDIR}/_ext/1408271/btThreads.o \
	${OBJECTDIR}/_ext/1408271/btVector3.o


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

${OBJECTDIR}/_ext/d0be1876/btAxisSweep3.o: ../lib/src/BulletCollision/BroadphaseCollision/btAxisSweep3.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d0be1876
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d0be1876/btAxisSweep3.o ../lib/src/BulletCollision/BroadphaseCollision/btAxisSweep3.cpp

${OBJECTDIR}/_ext/d0be1876/btBroadphaseProxy.o: ../lib/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d0be1876
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d0be1876/btBroadphaseProxy.o ../lib/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp

${OBJECTDIR}/_ext/d0be1876/btCollisionAlgorithm.o: ../lib/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d0be1876
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d0be1876/btCollisionAlgorithm.o ../lib/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/d0be1876/btDbvt.o: ../lib/src/BulletCollision/BroadphaseCollision/btDbvt.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d0be1876
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d0be1876/btDbvt.o ../lib/src/BulletCollision/BroadphaseCollision/btDbvt.cpp

${OBJECTDIR}/_ext/d0be1876/btDbvtBroadphase.o: ../lib/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d0be1876
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d0be1876/btDbvtBroadphase.o ../lib/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp

${OBJECTDIR}/_ext/d0be1876/btDispatcher.o: ../lib/src/BulletCollision/BroadphaseCollision/btDispatcher.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d0be1876
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d0be1876/btDispatcher.o ../lib/src/BulletCollision/BroadphaseCollision/btDispatcher.cpp

${OBJECTDIR}/_ext/d0be1876/btOverlappingPairCache.o: ../lib/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d0be1876
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d0be1876/btOverlappingPairCache.o ../lib/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp

${OBJECTDIR}/_ext/d0be1876/btQuantizedBvh.o: ../lib/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d0be1876
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d0be1876/btQuantizedBvh.o ../lib/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp

${OBJECTDIR}/_ext/d0be1876/btSimpleBroadphase.o: ../lib/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d0be1876
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d0be1876/btSimpleBroadphase.o ../lib/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp

${OBJECTDIR}/_ext/3c6b2689/SphereTriangleDetector.o: ../lib/src/BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/SphereTriangleDetector.o ../lib/src/BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp

${OBJECTDIR}/_ext/3c6b2689/btActivatingCollisionAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btActivatingCollisionAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btBox2dBox2dCollisionAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btBox2dBox2dCollisionAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btBoxBoxCollisionAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btBoxBoxCollisionAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btBoxBoxDetector.o: ../lib/src/BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btBoxBoxDetector.o ../lib/src/BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp

${OBJECTDIR}/_ext/3c6b2689/btCollisionDispatcher.o: ../lib/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btCollisionDispatcher.o ../lib/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp

${OBJECTDIR}/_ext/3c6b2689/btCollisionDispatcherMt.o: ../lib/src/BulletCollision/CollisionDispatch/btCollisionDispatcherMt.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btCollisionDispatcherMt.o ../lib/src/BulletCollision/CollisionDispatch/btCollisionDispatcherMt.cpp

${OBJECTDIR}/_ext/3c6b2689/btCollisionObject.o: ../lib/src/BulletCollision/CollisionDispatch/btCollisionObject.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btCollisionObject.o ../lib/src/BulletCollision/CollisionDispatch/btCollisionObject.cpp

${OBJECTDIR}/_ext/3c6b2689/btCollisionWorld.o: ../lib/src/BulletCollision/CollisionDispatch/btCollisionWorld.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btCollisionWorld.o ../lib/src/BulletCollision/CollisionDispatch/btCollisionWorld.cpp

${OBJECTDIR}/_ext/3c6b2689/btCollisionWorldImporter.o: ../lib/src/BulletCollision/CollisionDispatch/btCollisionWorldImporter.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btCollisionWorldImporter.o ../lib/src/BulletCollision/CollisionDispatch/btCollisionWorldImporter.cpp

${OBJECTDIR}/_ext/3c6b2689/btCompoundCollisionAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btCompoundCollisionAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btCompoundCompoundCollisionAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btCompoundCompoundCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btCompoundCompoundCollisionAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btCompoundCompoundCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btConvex2dConvex2dAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btConvex2dConvex2dAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btConvexConcaveCollisionAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btConvexConcaveCollisionAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btConvexConvexAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btConvexConvexAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btConvexPlaneCollisionAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btConvexPlaneCollisionAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btDefaultCollisionConfiguration.o: ../lib/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btDefaultCollisionConfiguration.o ../lib/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp

${OBJECTDIR}/_ext/3c6b2689/btEmptyCollisionAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btEmptyCollisionAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btGhostObject.o: ../lib/src/BulletCollision/CollisionDispatch/btGhostObject.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btGhostObject.o ../lib/src/BulletCollision/CollisionDispatch/btGhostObject.cpp

${OBJECTDIR}/_ext/3c6b2689/btHashedSimplePairCache.o: ../lib/src/BulletCollision/CollisionDispatch/btHashedSimplePairCache.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btHashedSimplePairCache.o ../lib/src/BulletCollision/CollisionDispatch/btHashedSimplePairCache.cpp

${OBJECTDIR}/_ext/3c6b2689/btInternalEdgeUtility.o: ../lib/src/BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btInternalEdgeUtility.o ../lib/src/BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp

${OBJECTDIR}/_ext/3c6b2689/btManifoldResult.o: ../lib/src/BulletCollision/CollisionDispatch/btManifoldResult.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btManifoldResult.o ../lib/src/BulletCollision/CollisionDispatch/btManifoldResult.cpp

${OBJECTDIR}/_ext/3c6b2689/btSimulationIslandManager.o: ../lib/src/BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btSimulationIslandManager.o ../lib/src/BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp

${OBJECTDIR}/_ext/3c6b2689/btSphereBoxCollisionAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btSphereBoxCollisionAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btSphereSphereCollisionAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btSphereSphereCollisionAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btSphereTriangleCollisionAlgorithm.o: ../lib/src/BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btSphereTriangleCollisionAlgorithm.o ../lib/src/BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/3c6b2689/btUnionFind.o: ../lib/src/BulletCollision/CollisionDispatch/btUnionFind.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/3c6b2689
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/3c6b2689/btUnionFind.o ../lib/src/BulletCollision/CollisionDispatch/btUnionFind.cpp

${OBJECTDIR}/_ext/8575bec1/btBox2dShape.o: ../lib/src/BulletCollision/CollisionShapes/btBox2dShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btBox2dShape.o ../lib/src/BulletCollision/CollisionShapes/btBox2dShape.cpp

${OBJECTDIR}/_ext/8575bec1/btBoxShape.o: ../lib/src/BulletCollision/CollisionShapes/btBoxShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btBoxShape.o ../lib/src/BulletCollision/CollisionShapes/btBoxShape.cpp

${OBJECTDIR}/_ext/8575bec1/btBvhTriangleMeshShape.o: ../lib/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btBvhTriangleMeshShape.o ../lib/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp

${OBJECTDIR}/_ext/8575bec1/btCapsuleShape.o: ../lib/src/BulletCollision/CollisionShapes/btCapsuleShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btCapsuleShape.o ../lib/src/BulletCollision/CollisionShapes/btCapsuleShape.cpp

${OBJECTDIR}/_ext/8575bec1/btCollisionShape.o: ../lib/src/BulletCollision/CollisionShapes/btCollisionShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btCollisionShape.o ../lib/src/BulletCollision/CollisionShapes/btCollisionShape.cpp

${OBJECTDIR}/_ext/8575bec1/btCompoundShape.o: ../lib/src/BulletCollision/CollisionShapes/btCompoundShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btCompoundShape.o ../lib/src/BulletCollision/CollisionShapes/btCompoundShape.cpp

${OBJECTDIR}/_ext/8575bec1/btConcaveShape.o: ../lib/src/BulletCollision/CollisionShapes/btConcaveShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btConcaveShape.o ../lib/src/BulletCollision/CollisionShapes/btConcaveShape.cpp

${OBJECTDIR}/_ext/8575bec1/btConeShape.o: ../lib/src/BulletCollision/CollisionShapes/btConeShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btConeShape.o ../lib/src/BulletCollision/CollisionShapes/btConeShape.cpp

${OBJECTDIR}/_ext/8575bec1/btConvex2dShape.o: ../lib/src/BulletCollision/CollisionShapes/btConvex2dShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btConvex2dShape.o ../lib/src/BulletCollision/CollisionShapes/btConvex2dShape.cpp

${OBJECTDIR}/_ext/8575bec1/btConvexHullShape.o: ../lib/src/BulletCollision/CollisionShapes/btConvexHullShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btConvexHullShape.o ../lib/src/BulletCollision/CollisionShapes/btConvexHullShape.cpp

${OBJECTDIR}/_ext/8575bec1/btConvexInternalShape.o: ../lib/src/BulletCollision/CollisionShapes/btConvexInternalShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btConvexInternalShape.o ../lib/src/BulletCollision/CollisionShapes/btConvexInternalShape.cpp

${OBJECTDIR}/_ext/8575bec1/btConvexPointCloudShape.o: ../lib/src/BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btConvexPointCloudShape.o ../lib/src/BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp

${OBJECTDIR}/_ext/8575bec1/btConvexPolyhedron.o: ../lib/src/BulletCollision/CollisionShapes/btConvexPolyhedron.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btConvexPolyhedron.o ../lib/src/BulletCollision/CollisionShapes/btConvexPolyhedron.cpp

${OBJECTDIR}/_ext/8575bec1/btConvexShape.o: ../lib/src/BulletCollision/CollisionShapes/btConvexShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btConvexShape.o ../lib/src/BulletCollision/CollisionShapes/btConvexShape.cpp

${OBJECTDIR}/_ext/8575bec1/btConvexTriangleMeshShape.o: ../lib/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btConvexTriangleMeshShape.o ../lib/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp

${OBJECTDIR}/_ext/8575bec1/btCylinderShape.o: ../lib/src/BulletCollision/CollisionShapes/btCylinderShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btCylinderShape.o ../lib/src/BulletCollision/CollisionShapes/btCylinderShape.cpp

${OBJECTDIR}/_ext/8575bec1/btEmptyShape.o: ../lib/src/BulletCollision/CollisionShapes/btEmptyShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btEmptyShape.o ../lib/src/BulletCollision/CollisionShapes/btEmptyShape.cpp

${OBJECTDIR}/_ext/8575bec1/btHeightfieldTerrainShape.o: ../lib/src/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btHeightfieldTerrainShape.o ../lib/src/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp

${OBJECTDIR}/_ext/8575bec1/btMinkowskiSumShape.o: ../lib/src/BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btMinkowskiSumShape.o ../lib/src/BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp

${OBJECTDIR}/_ext/8575bec1/btMultiSphereShape.o: ../lib/src/BulletCollision/CollisionShapes/btMultiSphereShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btMultiSphereShape.o ../lib/src/BulletCollision/CollisionShapes/btMultiSphereShape.cpp

${OBJECTDIR}/_ext/8575bec1/btMultimaterialTriangleMeshShape.o: ../lib/src/BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btMultimaterialTriangleMeshShape.o ../lib/src/BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp

${OBJECTDIR}/_ext/8575bec1/btOptimizedBvh.o: ../lib/src/BulletCollision/CollisionShapes/btOptimizedBvh.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btOptimizedBvh.o ../lib/src/BulletCollision/CollisionShapes/btOptimizedBvh.cpp

${OBJECTDIR}/_ext/8575bec1/btPolyhedralConvexShape.o: ../lib/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btPolyhedralConvexShape.o ../lib/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp

${OBJECTDIR}/_ext/8575bec1/btScaledBvhTriangleMeshShape.o: ../lib/src/BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btScaledBvhTriangleMeshShape.o ../lib/src/BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp

${OBJECTDIR}/_ext/8575bec1/btShapeHull.o: ../lib/src/BulletCollision/CollisionShapes/btShapeHull.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btShapeHull.o ../lib/src/BulletCollision/CollisionShapes/btShapeHull.cpp

${OBJECTDIR}/_ext/8575bec1/btSphereShape.o: ../lib/src/BulletCollision/CollisionShapes/btSphereShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btSphereShape.o ../lib/src/BulletCollision/CollisionShapes/btSphereShape.cpp

${OBJECTDIR}/_ext/8575bec1/btStaticPlaneShape.o: ../lib/src/BulletCollision/CollisionShapes/btStaticPlaneShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btStaticPlaneShape.o ../lib/src/BulletCollision/CollisionShapes/btStaticPlaneShape.cpp

${OBJECTDIR}/_ext/8575bec1/btStridingMeshInterface.o: ../lib/src/BulletCollision/CollisionShapes/btStridingMeshInterface.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btStridingMeshInterface.o ../lib/src/BulletCollision/CollisionShapes/btStridingMeshInterface.cpp

${OBJECTDIR}/_ext/8575bec1/btTetrahedronShape.o: ../lib/src/BulletCollision/CollisionShapes/btTetrahedronShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btTetrahedronShape.o ../lib/src/BulletCollision/CollisionShapes/btTetrahedronShape.cpp

${OBJECTDIR}/_ext/8575bec1/btTriangleBuffer.o: ../lib/src/BulletCollision/CollisionShapes/btTriangleBuffer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btTriangleBuffer.o ../lib/src/BulletCollision/CollisionShapes/btTriangleBuffer.cpp

${OBJECTDIR}/_ext/8575bec1/btTriangleCallback.o: ../lib/src/BulletCollision/CollisionShapes/btTriangleCallback.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btTriangleCallback.o ../lib/src/BulletCollision/CollisionShapes/btTriangleCallback.cpp

${OBJECTDIR}/_ext/8575bec1/btTriangleIndexVertexArray.o: ../lib/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btTriangleIndexVertexArray.o ../lib/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp

${OBJECTDIR}/_ext/8575bec1/btTriangleIndexVertexMaterialArray.o: ../lib/src/BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btTriangleIndexVertexMaterialArray.o ../lib/src/BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp

${OBJECTDIR}/_ext/8575bec1/btTriangleMesh.o: ../lib/src/BulletCollision/CollisionShapes/btTriangleMesh.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btTriangleMesh.o ../lib/src/BulletCollision/CollisionShapes/btTriangleMesh.cpp

${OBJECTDIR}/_ext/8575bec1/btTriangleMeshShape.o: ../lib/src/BulletCollision/CollisionShapes/btTriangleMeshShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btTriangleMeshShape.o ../lib/src/BulletCollision/CollisionShapes/btTriangleMeshShape.cpp

${OBJECTDIR}/_ext/8575bec1/btUniformScalingShape.o: ../lib/src/BulletCollision/CollisionShapes/btUniformScalingShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8575bec1
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8575bec1/btUniformScalingShape.o ../lib/src/BulletCollision/CollisionShapes/btUniformScalingShape.cpp

${OBJECTDIR}/_ext/1983f56a/btContactProcessing.o: ../lib/src/BulletCollision/Gimpact/btContactProcessing.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1983f56a
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1983f56a/btContactProcessing.o ../lib/src/BulletCollision/Gimpact/btContactProcessing.cpp

${OBJECTDIR}/_ext/1983f56a/btGImpactBvh.o: ../lib/src/BulletCollision/Gimpact/btGImpactBvh.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1983f56a
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1983f56a/btGImpactBvh.o ../lib/src/BulletCollision/Gimpact/btGImpactBvh.cpp

${OBJECTDIR}/_ext/1983f56a/btGImpactCollisionAlgorithm.o: ../lib/src/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1983f56a
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1983f56a/btGImpactCollisionAlgorithm.o ../lib/src/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp

${OBJECTDIR}/_ext/1983f56a/btGImpactQuantizedBvh.o: ../lib/src/BulletCollision/Gimpact/btGImpactQuantizedBvh.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1983f56a
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1983f56a/btGImpactQuantizedBvh.o ../lib/src/BulletCollision/Gimpact/btGImpactQuantizedBvh.cpp

${OBJECTDIR}/_ext/1983f56a/btGImpactShape.o: ../lib/src/BulletCollision/Gimpact/btGImpactShape.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1983f56a
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1983f56a/btGImpactShape.o ../lib/src/BulletCollision/Gimpact/btGImpactShape.cpp

${OBJECTDIR}/_ext/1983f56a/btGenericPoolAllocator.o: ../lib/src/BulletCollision/Gimpact/btGenericPoolAllocator.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1983f56a
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1983f56a/btGenericPoolAllocator.o ../lib/src/BulletCollision/Gimpact/btGenericPoolAllocator.cpp

${OBJECTDIR}/_ext/1983f56a/btTriangleShapeEx.o: ../lib/src/BulletCollision/Gimpact/btTriangleShapeEx.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1983f56a
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1983f56a/btTriangleShapeEx.o ../lib/src/BulletCollision/Gimpact/btTriangleShapeEx.cpp

${OBJECTDIR}/_ext/1983f56a/gim_box_set.o: ../lib/src/BulletCollision/Gimpact/gim_box_set.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1983f56a
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1983f56a/gim_box_set.o ../lib/src/BulletCollision/Gimpact/gim_box_set.cpp

${OBJECTDIR}/_ext/1983f56a/gim_contact.o: ../lib/src/BulletCollision/Gimpact/gim_contact.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1983f56a
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1983f56a/gim_contact.o ../lib/src/BulletCollision/Gimpact/gim_contact.cpp

${OBJECTDIR}/_ext/1983f56a/gim_memory.o: ../lib/src/BulletCollision/Gimpact/gim_memory.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1983f56a
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1983f56a/gim_memory.o ../lib/src/BulletCollision/Gimpact/gim_memory.cpp

${OBJECTDIR}/_ext/1983f56a/gim_tri_collision.o: ../lib/src/BulletCollision/Gimpact/gim_tri_collision.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1983f56a
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1983f56a/gim_tri_collision.o ../lib/src/BulletCollision/Gimpact/gim_tri_collision.cpp

${OBJECTDIR}/_ext/6e925355/btContinuousConvexCollision.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btContinuousConvexCollision.o ../lib/src/BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp

${OBJECTDIR}/_ext/6e925355/btConvexCast.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btConvexCast.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btConvexCast.o ../lib/src/BulletCollision/NarrowPhaseCollision/btConvexCast.cpp

${OBJECTDIR}/_ext/6e925355/btGjkConvexCast.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btGjkConvexCast.o ../lib/src/BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp

${OBJECTDIR}/_ext/6e925355/btGjkEpa2.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btGjkEpa2.o ../lib/src/BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp

${OBJECTDIR}/_ext/6e925355/btGjkEpaPenetrationDepthSolver.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btGjkEpaPenetrationDepthSolver.o ../lib/src/BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp

${OBJECTDIR}/_ext/6e925355/btGjkPairDetector.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btGjkPairDetector.o ../lib/src/BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp

${OBJECTDIR}/_ext/6e925355/btMinkowskiPenetrationDepthSolver.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btMinkowskiPenetrationDepthSolver.o ../lib/src/BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp

${OBJECTDIR}/_ext/6e925355/btPersistentManifold.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btPersistentManifold.o ../lib/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp

${OBJECTDIR}/_ext/6e925355/btPolyhedralContactClipping.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btPolyhedralContactClipping.o ../lib/src/BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.cpp

${OBJECTDIR}/_ext/6e925355/btRaycastCallback.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btRaycastCallback.o ../lib/src/BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp

${OBJECTDIR}/_ext/6e925355/btSubSimplexConvexCast.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btSubSimplexConvexCast.o ../lib/src/BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp

${OBJECTDIR}/_ext/6e925355/btVoronoiSimplexSolver.o: ../lib/src/BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6e925355
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6e925355/btVoronoiSimplexSolver.o ../lib/src/BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp

${OBJECTDIR}/_ext/80780714/btKinematicCharacterController.o: ../lib/src/BulletDynamics/Character/btKinematicCharacterController.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/80780714
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/80780714/btKinematicCharacterController.o ../lib/src/BulletDynamics/Character/btKinematicCharacterController.cpp

${OBJECTDIR}/_ext/12134e05/btConeTwistConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btConeTwistConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp

${OBJECTDIR}/_ext/12134e05/btContactConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btContactConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btContactConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btContactConstraint.cpp

${OBJECTDIR}/_ext/12134e05/btFixedConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btFixedConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btFixedConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btFixedConstraint.cpp

${OBJECTDIR}/_ext/12134e05/btGearConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btGearConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btGearConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btGearConstraint.cpp

${OBJECTDIR}/_ext/12134e05/btGeneric6DofConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btGeneric6DofConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp

${OBJECTDIR}/_ext/12134e05/btGeneric6DofSpring2Constraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpring2Constraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btGeneric6DofSpring2Constraint.o ../lib/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpring2Constraint.cpp

${OBJECTDIR}/_ext/12134e05/btGeneric6DofSpringConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btGeneric6DofSpringConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp

${OBJECTDIR}/_ext/12134e05/btHinge2Constraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btHinge2Constraint.o ../lib/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp

${OBJECTDIR}/_ext/12134e05/btHingeConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btHingeConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btHingeConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btHingeConstraint.cpp

${OBJECTDIR}/_ext/12134e05/btNNCGConstraintSolver.o: ../lib/src/BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btNNCGConstraintSolver.o ../lib/src/BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.cpp

${OBJECTDIR}/_ext/12134e05/btPoint2PointConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btPoint2PointConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp

${OBJECTDIR}/_ext/12134e05/btSequentialImpulseConstraintSolver.o: ../lib/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btSequentialImpulseConstraintSolver.o ../lib/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp

${OBJECTDIR}/_ext/12134e05/btSliderConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btSliderConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btSliderConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btSliderConstraint.cpp

${OBJECTDIR}/_ext/12134e05/btSolve2LinearConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btSolve2LinearConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp

${OBJECTDIR}/_ext/12134e05/btTypedConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btTypedConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btTypedConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btTypedConstraint.cpp

${OBJECTDIR}/_ext/12134e05/btUniversalConstraint.o: ../lib/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/12134e05
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/12134e05/btUniversalConstraint.o ../lib/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp

${OBJECTDIR}/_ext/5f6bb2c9/btDiscreteDynamicsWorld.o: ../lib/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5f6bb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5f6bb2c9/btDiscreteDynamicsWorld.o ../lib/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp

${OBJECTDIR}/_ext/5f6bb2c9/btDiscreteDynamicsWorldMt.o: ../lib/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5f6bb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5f6bb2c9/btDiscreteDynamicsWorldMt.o ../lib/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.cpp

${OBJECTDIR}/_ext/5f6bb2c9/btRigidBody.o: ../lib/src/BulletDynamics/Dynamics/btRigidBody.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5f6bb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5f6bb2c9/btRigidBody.o ../lib/src/BulletDynamics/Dynamics/btRigidBody.cpp

${OBJECTDIR}/_ext/5f6bb2c9/btSimpleDynamicsWorld.o: ../lib/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5f6bb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5f6bb2c9/btSimpleDynamicsWorld.o ../lib/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp

${OBJECTDIR}/_ext/5f6bb2c9/btSimulationIslandManagerMt.o: ../lib/src/BulletDynamics/Dynamics/btSimulationIslandManagerMt.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5f6bb2c9
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5f6bb2c9/btSimulationIslandManagerMt.o ../lib/src/BulletDynamics/Dynamics/btSimulationIslandManagerMt.cpp

${OBJECTDIR}/_ext/cf0245d7/btMultiBody.o: ../lib/src/BulletDynamics/Featherstone/btMultiBody.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/cf0245d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cf0245d7/btMultiBody.o ../lib/src/BulletDynamics/Featherstone/btMultiBody.cpp

${OBJECTDIR}/_ext/cf0245d7/btMultiBodyConstraint.o: ../lib/src/BulletDynamics/Featherstone/btMultiBodyConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/cf0245d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cf0245d7/btMultiBodyConstraint.o ../lib/src/BulletDynamics/Featherstone/btMultiBodyConstraint.cpp

${OBJECTDIR}/_ext/cf0245d7/btMultiBodyConstraintSolver.o: ../lib/src/BulletDynamics/Featherstone/btMultiBodyConstraintSolver.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/cf0245d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cf0245d7/btMultiBodyConstraintSolver.o ../lib/src/BulletDynamics/Featherstone/btMultiBodyConstraintSolver.cpp

${OBJECTDIR}/_ext/cf0245d7/btMultiBodyDynamicsWorld.o: ../lib/src/BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/cf0245d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cf0245d7/btMultiBodyDynamicsWorld.o ../lib/src/BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.cpp

${OBJECTDIR}/_ext/cf0245d7/btMultiBodyFixedConstraint.o: ../lib/src/BulletDynamics/Featherstone/btMultiBodyFixedConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/cf0245d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cf0245d7/btMultiBodyFixedConstraint.o ../lib/src/BulletDynamics/Featherstone/btMultiBodyFixedConstraint.cpp

${OBJECTDIR}/_ext/cf0245d7/btMultiBodyGearConstraint.o: ../lib/src/BulletDynamics/Featherstone/btMultiBodyGearConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/cf0245d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cf0245d7/btMultiBodyGearConstraint.o ../lib/src/BulletDynamics/Featherstone/btMultiBodyGearConstraint.cpp

${OBJECTDIR}/_ext/cf0245d7/btMultiBodyJointLimitConstraint.o: ../lib/src/BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/cf0245d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cf0245d7/btMultiBodyJointLimitConstraint.o ../lib/src/BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.cpp

${OBJECTDIR}/_ext/cf0245d7/btMultiBodyJointMotor.o: ../lib/src/BulletDynamics/Featherstone/btMultiBodyJointMotor.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/cf0245d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cf0245d7/btMultiBodyJointMotor.o ../lib/src/BulletDynamics/Featherstone/btMultiBodyJointMotor.cpp

${OBJECTDIR}/_ext/cf0245d7/btMultiBodyPoint2Point.o: ../lib/src/BulletDynamics/Featherstone/btMultiBodyPoint2Point.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/cf0245d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cf0245d7/btMultiBodyPoint2Point.o ../lib/src/BulletDynamics/Featherstone/btMultiBodyPoint2Point.cpp

${OBJECTDIR}/_ext/cf0245d7/btMultiBodySliderConstraint.o: ../lib/src/BulletDynamics/Featherstone/btMultiBodySliderConstraint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/cf0245d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cf0245d7/btMultiBodySliderConstraint.o ../lib/src/BulletDynamics/Featherstone/btMultiBodySliderConstraint.cpp

${OBJECTDIR}/_ext/5f05fc1f/btDantzigLCP.o: ../lib/src/BulletDynamics/MLCPSolvers/btDantzigLCP.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5f05fc1f
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5f05fc1f/btDantzigLCP.o ../lib/src/BulletDynamics/MLCPSolvers/btDantzigLCP.cpp

${OBJECTDIR}/_ext/5f05fc1f/btLemkeAlgorithm.o: ../lib/src/BulletDynamics/MLCPSolvers/btLemkeAlgorithm.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5f05fc1f
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5f05fc1f/btLemkeAlgorithm.o ../lib/src/BulletDynamics/MLCPSolvers/btLemkeAlgorithm.cpp

${OBJECTDIR}/_ext/5f05fc1f/btMLCPSolver.o: ../lib/src/BulletDynamics/MLCPSolvers/btMLCPSolver.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5f05fc1f
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5f05fc1f/btMLCPSolver.o ../lib/src/BulletDynamics/MLCPSolvers/btMLCPSolver.cpp

${OBJECTDIR}/_ext/6f87d9d7/btRaycastVehicle.o: ../lib/src/BulletDynamics/Vehicle/btRaycastVehicle.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6f87d9d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6f87d9d7/btRaycastVehicle.o ../lib/src/BulletDynamics/Vehicle/btRaycastVehicle.cpp

${OBJECTDIR}/_ext/6f87d9d7/btWheelInfo.o: ../lib/src/BulletDynamics/Vehicle/btWheelInfo.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/6f87d9d7
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/6f87d9d7/btWheelInfo.o ../lib/src/BulletDynamics/Vehicle/btWheelInfo.cpp

${OBJECTDIR}/_ext/1408271/btAlignedAllocator.o: ../lib/src/LinearMath/btAlignedAllocator.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1408271
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1408271/btAlignedAllocator.o ../lib/src/LinearMath/btAlignedAllocator.cpp

${OBJECTDIR}/_ext/1408271/btConvexHull.o: ../lib/src/LinearMath/btConvexHull.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1408271
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1408271/btConvexHull.o ../lib/src/LinearMath/btConvexHull.cpp

${OBJECTDIR}/_ext/1408271/btConvexHullComputer.o: ../lib/src/LinearMath/btConvexHullComputer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1408271
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1408271/btConvexHullComputer.o ../lib/src/LinearMath/btConvexHullComputer.cpp

${OBJECTDIR}/_ext/1408271/btGeometryUtil.o: ../lib/src/LinearMath/btGeometryUtil.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1408271
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1408271/btGeometryUtil.o ../lib/src/LinearMath/btGeometryUtil.cpp

${OBJECTDIR}/_ext/1408271/btPolarDecomposition.o: ../lib/src/LinearMath/btPolarDecomposition.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1408271
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1408271/btPolarDecomposition.o ../lib/src/LinearMath/btPolarDecomposition.cpp

${OBJECTDIR}/_ext/1408271/btQuickprof.o: ../lib/src/LinearMath/btQuickprof.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1408271
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1408271/btQuickprof.o ../lib/src/LinearMath/btQuickprof.cpp

${OBJECTDIR}/_ext/1408271/btSerializer.o: ../lib/src/LinearMath/btSerializer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1408271
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1408271/btSerializer.o ../lib/src/LinearMath/btSerializer.cpp

${OBJECTDIR}/_ext/1408271/btSerializer64.o: ../lib/src/LinearMath/btSerializer64.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1408271
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1408271/btSerializer64.o ../lib/src/LinearMath/btSerializer64.cpp

${OBJECTDIR}/_ext/1408271/btThreads.o: ../lib/src/LinearMath/btThreads.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1408271
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1408271/btThreads.o ../lib/src/LinearMath/btThreads.cpp

${OBJECTDIR}/_ext/1408271/btVector3.o: ../lib/src/LinearMath/btVector3.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1408271
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1408271/btVector3.o ../lib/src/LinearMath/btVector3.cpp

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
