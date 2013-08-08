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
RECASTNAV_DIR = $(KBE_ROOT)/kbe/src/lib/third_party/recastnavigation
KBE_INCLUDES += -I$(RECASTNAV_DIR)/Detour/Include
KBE_INCLUDES += -I$(RECASTNAV_DIR)/DetourCrowd/Include
KBE_INCLUDES += -I$(RECASTNAV_DIR)/DetourTileCache/Include
KBE_INCLUDES += -I$(RECASTNAV_DIR)/Recast/Include
endif

include $(KBE_ROOT)/kbe/src/build/common.mak