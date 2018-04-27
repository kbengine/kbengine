// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CLIENT_ENTITY_ASPECT_H
#define KBE_CLIENT_ENTITY_ASPECT_H

#include "common/common.h"


namespace KBEngine{


class EntityAspect
{
public:
	EntityAspect(const EntityAspect& entityAspect);
	EntityAspect(ENTITY_ID aspectID);
	EntityAspect();
	virtual ~EntityAspect();
	
	void modelres(const std::string& modelres){ modelres_ = modelres; }
	const std::string& modelres(){ return modelres_; }

	ENTITY_ID aspectID() const{ return aspectID_; }

	float modelScale() const{ return modelScale_; }
protected:
	ENTITY_ID aspectID_;
	std::string modelres_;
	float modelScale_;	
};

}
#endif
