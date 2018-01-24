/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#include "cellapp.h"
#include "forward_message_over_handler.h"
#include "entitydef/entities.h"
#include "common/memorystream.h"
#include "entity.h"
#include "space.h"
#include "spaces.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityInNewSpaceFromBaseapp::
FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityInNewSpaceFromBaseapp(Entity* e, SPACE_ID spaceID, PyObject* params) :
_e(e),
_spaceID(spaceID),
_params(params)
{
}

//-------------------------------------------------------------------------------------
FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityInNewSpaceFromBaseapp::~FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityInNewSpaceFromBaseapp()
{
	if (_params)
		Py_XDECREF(_params);
}

//-------------------------------------------------------------------------------------
void FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityInNewSpaceFromBaseapp::process()
{
	KBE_ASSERT(_e != NULL);
	
	Space* space = Spaces::findSpace(_spaceID);
	
	if(space == NULL || !space->isGood())
	{
		ERROR_MSG(fmt::format("FMH_Baseapp_onEntityGetCell::process: not found space({}), {} {}.\n",
			_spaceID, _e->scriptName(), _e->id()));

		return;
	}

	_e->spaceID(space->id());
	_e->initializeEntity(_params);
	Py_XDECREF(_params);
	_params = NULL;

	// Ìí¼Óµ½space
	space->addEntityToNode(_e);

	if (_e->clientEntityCall())
	{
		_e->onGetWitness();
	}
	else
	{
		space->onEnterWorld(_e);
	}
}

//-------------------------------------------------------------------------------------
FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityFromBaseapp::
	FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityFromBaseapp(
			std::string& entityType, ENTITY_ID createToEntityID, ENTITY_ID entityID, MemoryStream* pCellData, 
			 bool hasClient, bool inRescore, COMPONENT_ID componentID, SPACE_ID spaceID):
_entityType(entityType),
_createToEntityID(createToEntityID),
_entityID(entityID),
_pCellData(pCellData),
_hasClient(hasClient),
_componentID(componentID),
_spaceID(spaceID),
_inRescore(inRescore)
{
}

//-------------------------------------------------------------------------------------
FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityFromBaseapp::~FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityFromBaseapp()
{
	SAFE_RELEASE(_pCellData);
}

//-------------------------------------------------------------------------------------
void FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityFromBaseapp::process()
{
	Cellapp::getSingleton()._onCreateCellEntityFromBaseapp(_entityType, _createToEntityID, _entityID, 
		_pCellData, _hasClient, _inRescore, _componentID, _spaceID);

	MemoryStream::reclaimPoolObject(_pCellData);
	_pCellData = NULL;
}

//-------------------------------------------------------------------------------------
}
