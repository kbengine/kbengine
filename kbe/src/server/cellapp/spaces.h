// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SPACEMANAGER_H
#define KBE_SPACEMANAGER_H

#include "helper/debug_helper.h"
#include "common/common.h"
#include "common/singleton.h"
#include "updatable.h"
#include "space.h"

namespace KBEngine{

class Spaces
{
public:
	Spaces();
	~Spaces();
	
	static void finalise();

	typedef std::map<SPACE_ID, KBEShared_ptr<Space> > SPACES;

	/** 
		创建一个新的space 
	*/
	static Space* createNewSpace(SPACE_ID spaceID, const std::string& scriptModuleName);
	
	/**
		销毁一个space
	*/
	static bool destroySpace(SPACE_ID spaceID, ENTITY_ID entityID);

	/** 
		寻找一个指定space 
	*/
	static Space* findSpace(SPACE_ID spaceID);
	
	/** 
		更新所有的space 
	*/
	static void update();

	static size_t size(){ return spaces_.size(); }

protected:
	static SPACES spaces_;
};

}
#endif
