LIB =	recastnav

SRCS =					\
	DetourAlloc			\
	DetourCommon			\
	DetourNavMesh			\
	DetourNavMeshBuilder		\
	DetourNavMeshQuery		\
	DetourNode			\
	DetourCrowd			\
	DetourLocalBoundary		\
	DetourObstacleAvoidance		\
	DetourPathCorridor		\
	DetourPathQueue			\
	DetourProximityGrid		\
	DetourTileCache			\
	DetourTileCacheBuilder		\
	Recast				\
	RecastAlloc			\
	RecastArea			\
	RecastContour			\
	RecastFilter			\
	RecastLayers			\
	RecastMesh			\
	RecastMeshDetail		\
	RecastRasterization		\
	RecastRegion

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/third_party/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak