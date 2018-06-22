// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_ARCHIVER_H
#define KBE_ARCHIVER_H

// common include
#include "helper/debug_helper.h"
#include "common/common.h"
// #define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine{

class Archiver
{
public:
	Archiver();
	~Archiver();
	
	void tick();
	void createArchiveTable();
	void archive(Entity& entity);

private:
	int						archiveIndex_;
	std::vector<ENTITY_ID> 	arEntityIDs_;
};


}

#endif // KBE_ARCHIVER_H
