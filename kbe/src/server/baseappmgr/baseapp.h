// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_BASEAPPMGR_BASEAPP_H
#define KBE_BASEAPPMGR_BASEAPP_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "helper/watcher.h"

namespace KBEngine{ 

class Baseapp
{
public:
	Baseapp();
	virtual ~Baseapp();
	
	ENTITY_ID numEntitys() const { return numEntitys_; }
	void numEntitys(ENTITY_ID num) { numEntitys_ = num; }
	
	ENTITY_ID numProxices() const { return numProxices_; }
	void numProxices(ENTITY_ID num) { numProxices_ = num; }
	void incNumProxices() { ++numProxices_; }

	float load() const { return load_; }
	void load(float v) { load_ = v; }
	
	void destroy(){ isDestroyed_ = true; }
	bool isDestroyed() const { return isDestroyed_; }

	float initProgress() const{ return initProgress_; }
	void initProgress(float v){ initProgress_ = v; }

	ENTITY_ID numEntities() const { return numEntitys_ + numProxices_; }
	void incNumEntities() { ++numEntitys_; }

	uint32 flags() const { return flags_; }
	void flags(uint32 v) { flags_ = v; }
	
protected:
	ENTITY_ID numEntitys_;
	ENTITY_ID numProxices_;
	float load_;

	bool isDestroyed_;

	Watchers watchers_;

	float initProgress_;
	
	uint32 flags_;
};

}

#endif // KBE_BASEAPPMGR_BASEAPP_H
