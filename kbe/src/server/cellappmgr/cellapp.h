// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_CELLAPPMGR_CELLAPP_H
#define KBE_CELLAPPMGR_CELLAPP_H

#include "spaces.h"
#include "common/common.h"
#include "helper/debug_helper.h"
#include "helper/watcher.h"

namespace KBEngine{ 

class Cellapp
{
public:
	Cellapp();
	virtual ~Cellapp();
	
	float load() const { return load_; }
	void load(float v) { load_ = v; }
	
	void destroy(){ isDestroyed_ = true; }
	bool isDestroyed() const { return isDestroyed_; }

	float initProgress() const{ return initProgress_; }
	void initProgress(float v){ initProgress_ = v; }

	ENTITY_ID numEntities() const { return numEntities_; }
	void numEntities(ENTITY_ID num) { numEntities_ = num; }
	void incNumEntities() { ++numEntities_; }

	uint32 flags() const { return flags_; }
	void flags(uint32 v) { flags_ = v; }

	void globalOrderID(COMPONENT_ORDER v) { globalOrderID_ = v; }
	COMPONENT_ORDER globalOrderID() const { return globalOrderID_; }

	void groupOrderID(COMPONENT_ORDER v) { groupOrderID_ = v; }
	COMPONENT_ORDER groupOrderID() const { return groupOrderID_; }
	
	ENTITY_ID numSpaces() const { return spaces_.size(); }
	Spaces& spaces() { return spaces_; }

protected:
	ENTITY_ID numEntities_;

	float load_;

	bool isDestroyed_;

	Watchers watchers_;
	
	Spaces spaces_;

	float initProgress_;

	uint32 flags_;

	COMPONENT_ORDER globalOrderID_;

	COMPONENT_ORDER groupOrderID_;
};

}

#endif // KBE_CELLAPPMGR_CELLAPP_H
