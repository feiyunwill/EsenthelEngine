LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE       := recast
LOCAL_SRC_FILES    := ../../Detour/Source/DetourAlloc.cpp ../../Detour/Source/DetourCommon.cpp ../../Detour/Source/DetourNavMesh.cpp ../../Detour/Source/DetourNavMeshBuilder.cpp ../../Detour/Source/DetourNavMeshQuery.cpp ../../Detour/Source/DetourNode.cpp ../../DetourCrowd/Source/DetourCrowd.cpp ../../DetourCrowd/Source/DetourLocalBoundary.cpp ../../DetourCrowd/Source/DetourObstacleAvoidance.cpp ../../DetourCrowd/Source/DetourPathCorridor.cpp ../../DetourCrowd/Source/DetourPathQueue.cpp ../../DetourCrowd/Source/DetourProximityGrid.cpp ../../DetourTileCache/Source/DetourTileCache.cpp ../../DetourTileCache/Source/DetourTileCacheBuilder.cpp ../../Recast/Source/Recast.cpp ../../Recast/Source/RecastAlloc.cpp ../../Recast/Source/RecastArea.cpp ../../Recast/Source/RecastContour.cpp ../../Recast/Source/RecastFilter.cpp ../../Recast/Source/RecastLayers.cpp ../../Recast/Source/RecastMesh.cpp ../../Recast/Source/RecastMeshDetail.cpp ../../Recast/Source/RecastRasterization.cpp ../../Recast/Source/RecastRegion.cpp
LOCAL_CFLAGS       := -I../Recast/Include -I../DetourTileCache/Include -I../DetourCrowd/Include -I../Detour/Include -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPPFLAGS     := -I../Recast/Include -I../DetourTileCache/Include -I../DetourCrowd/Include -I../Detour/Include -O3 -fshort-wchar -ffast-math -fomit-frame-pointer
LOCAL_CPP_FEATURES := rtti # exceptions
LOCAL_ARM_NEON     := true # force NEON usage for all files

include $(BUILD_STATIC_LIBRARY)
