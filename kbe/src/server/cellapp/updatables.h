// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_UPDATABLES_H
#define KBE_UPDATABLES_H

// common include
#include "helper/debug_helper.h"
#include "common/common.h"
#include "updatable.h"	
// #define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32	
#else
// linux include
#endif

namespace KBEngine{

class Updatables
{
public:
	Updatables();
	~Updatables();

	void clear();

	bool add(Updatable* updatable);
	bool remove(Updatable* updatable);

	void update();

private:
	std::vector< std::map<uint32, Updatable*> > objects_;
};

}
#endif
