/******************************************************************************/
#include "stdafx.h"

#if PHYSX_DLL_ACTUAL

#include "../../../ThirdPartyLibs/begin.h"

#undef  PX_DEBUG_VEHICLE_ON
#define PX_DEBUG_VEHICLE_ON 0

#if WINDOWS
#undef _WINDOWS_
#undef _WIN32_WINNT
#include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/windows/PsWindowsMutex.cpp"
#include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/windows/PsWindowsPrintString.cpp"
#else
#include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/unix/PsUnixMutex.cpp"
#include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/unix/PsUnixPrintString.cpp"
#endif

#if 1
PxFoundation& PxGetFoundation() {return *Physx.foundation;}
namespace physx{
namespace shdfnd{
   PxAllocatorCallback& getAllocator() {return Physx.allocator;}
   Foundation& Foundation::getInstance() {return SCAST(Foundation, *Physx.foundation);}
   void Foundation::error(PxErrorCode::Enum c, const char* file, int line, const char* messageFmt, ...) {Exit(messageFmt);}
   void Foundation::incRefCount() {}
   void Foundation::decRefCount() {}
}}
#else
#include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/PsFoundation.cpp"
#endif
#include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/PsAllocator.cpp"
#include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/PsTempAllocator.cpp"
#include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/PsString.cpp"

#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtRigidBodyExt.cpp"

#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtJoint.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtD6Joint.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtD6JointSolverPrep.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtDistanceJoint.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtDistanceJointSolverPrep.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtFixedJoint.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtFixedJointSolverPrep.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtPrismaticJoint.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtPrismaticJointSolverPrep.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtRevoluteJoint.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtRevoluteJointSolverPrep.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtSphericalJoint.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtSphericalJointSolverPrep.cpp"

#define Edge PxEdge
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtClothFabricCooker.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtClothSimpleTetherCooker.cpp"
#define prefixSum         _prefixSum         // avoid name duplicates from other sources
#define gatherAdjacencies _gatherAdjacencies // avoid name duplicates from other sources
#define pushHeap          _pushHeap          // avoid name duplicates from other sources
#define  popHeap           _popHeap          // avoid name duplicates from other sources
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtClothGeodesicTetherCooker.cpp"
#undef Edge

#define resolveConstraintPtr _resolveConstraintPtr // avoid name duplicates from other sources
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleComponents.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleDrive.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleDrive4W.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleDriveNW.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleDriveTank.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleNoDrive.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleSDK.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleSuspWheelTire4.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleTireFriction.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleUpdate.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleWheels.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleSerialization.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PxVehicleMetaData.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/VehicleUtilControl.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/VehicleUtilSetup.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/VehicleUtilTelemetry.cpp"

#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PhysXMetaData/src/PxVehicleAutoGeneratedMetaDataObjects.cpp"
#include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PhysXMetaData/src/PxVehicleMetaDataObjects.cpp"

#include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/PsMathUtils.cpp"

#if USE_DEFAULT_CPU_DISPATCHER
   #include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtCpuWorkerThread.cpp"
   #include "../../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/ExtDefaultCpuDispatcher.cpp"
   #if WINDOWS
      #include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/windows/PsWindowsThread.cpp"
      #include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/windows/PsWindowsSync.cpp"
      #include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/windows/PsWindowsSList.cpp"
   #else
      #include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/unix/PsUnixThread.cpp"
      #include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/unix/PsUnixSync.cpp"
      #include "../../../ThirdPartyLibs/PhysX/PxShared/src/foundation/src/unix/PsWindowsSList.cpp"
   #endif
#endif

#include "../../../ThirdPartyLibs/end.h"
#endif
/******************************************************************************/
