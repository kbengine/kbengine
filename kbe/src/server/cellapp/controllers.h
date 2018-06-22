// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CONTROLLERS_H
#define KBE_CONTROLLERS_H

#include "controller.h"	

namespace KBEngine{

class Controllers
{
public:
	Controllers(ENTITY_ID entityID);
	~Controllers();

	bool add(KBEShared_ptr<Controller> pController);
	bool remove(KBEShared_ptr<Controller> pController);
	bool remove(uint32 id);
	
	void clear();

	uint32 freeID() { return ++lastid_; }

	typedef std::map<uint32, KBEShared_ptr< Controller > > CONTROLLERS_MAP;
	CONTROLLERS_MAP& objects() { return objects_; }

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

private:
	CONTROLLERS_MAP objects_;

	uint32 lastid_;

	ENTITY_ID entityID_;
};

}
#endif // KBE_CONTROLLERS_H
