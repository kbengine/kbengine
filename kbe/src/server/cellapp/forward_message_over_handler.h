// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_FORWARD_MESSAGE_HANDLER_H
#define KBE_FORWARD_MESSAGE_HANDLER_H

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

class FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityInNewSpaceFromBaseapp : public ForwardMessageOverHandler
{
public:
	FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityInNewSpaceFromBaseapp(Entity* e, SPACE_ID spaceID, PyObject* params);
	~FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityInNewSpaceFromBaseapp();

	virtual void process();

private:
	Entity* _e;
	SPACE_ID _spaceID;
	PyObject* _params;
};

class FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityFromBaseapp : public ForwardMessageOverHandler
{
public:
	FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityFromBaseapp(std::string& entityType, ENTITY_ID createToEntityID, 
		ENTITY_ID entityID, MemoryStream* pCellData, bool hasClient, bool inRescore, COMPONENT_ID componentID, SPACE_ID spaceID);
	~FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityFromBaseapp();

	virtual void process();

private:
	std::string _entityType;
	ENTITY_ID _createToEntityID, _entityID;
	MemoryStream* _pCellData;
	bool _hasClient;
	COMPONENT_ID _componentID;
	SPACE_ID _spaceID;
	bool _inRescore;
};

}

#endif // KBE_FORWARD_MESSAGE_HANDLER_H

