LIB =	navigation

SRCS =						\
	DetourAlloc				\
	DetourAssert			\
	DetourCommon			\
	DetourNavMesh			\
	DetourNavMeshBuilder	\
	DetourNavMeshQuery		\
	DetourNode				\
	DetourCrowd				\
	DetourLocalBoundary		\
	DetourObstacleAvoidance	\
	DetourPathCorridor		\
	DetourPathQueue			\
	DetourProximityGrid		\
	DetourTileCache			\
	DetourTileCacheBuilder	\
	navigation				\
	navigation_handle		\
	navigation_tile_handle	\
	navigation_mesh_handle	\
	Recast					\
	RecastAlloc				\
	RecastArea				\
	RecastAssert			\
	RecastContour			\
	RecastFilter			\
	RecastLayers			\
	RecastMesh				\
	RecastMeshDetail		\
	RecastRasterization		\
	RecastRegion

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak