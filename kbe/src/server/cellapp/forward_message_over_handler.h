/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

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

class FMH_Baseapp_onEntityGetCellFrom_onCreateInNewSpaceFromBaseapp : public ForwardMessageOverHandler
{
public:
	FMH_Baseapp_onEntityGetCellFrom_onCreateInNewSpaceFromBaseapp(Entity* e, SPACE_ID spaceID, PyObject* params);
	~FMH_Baseapp_onEntityGetCellFrom_onCreateInNewSpaceFromBaseapp();

	virtual void process();

private:
	Entity* _e;
	SPACE_ID _spaceID;
	PyObject* params_;
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

