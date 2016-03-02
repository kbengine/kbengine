/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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
#include "base.h"
#include "baseapp.h"
#include "restore_entity_handler.h"
#include "server/components.h"
#include "entitydef/entity_mailbox.h"
#include "entitydef/entitydef.h"

#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
RestoreEntityHandler::RestoreEntityHandler(COMPONENT_ID cellappID, Network::NetworkInterface & networkInterface):
Task(),
networkInterface_(networkInterface),
entities_(),
inProcess_(false),
restoreSpaces_(),
otherRestoredSpaces_(),
broadcastOtherBaseapps_(false),
tickReport_(0),
spaceIDs_(),
cellappID_(cellappID),
canRestore_(false)
{
	// networkInterface_.dispatcher().addTask(this);
}

//-------------------------------------------------------------------------------------
RestoreEntityHandler::~RestoreEntityHandler()
{
	if(inProcess_)
		networkInterface_.dispatcher().cancelTask(this);

	std::vector<RestoreData>::iterator restoreSpacesIter = otherRestoredSpaces_.begin();
	for(; restoreSpacesIter != otherRestoredSpaces_.end(); ++restoreSpacesIter)
	{
		if((*restoreSpacesIter).cell)
		{
			Py_DECREF((*restoreSpacesIter).cell);
			(*restoreSpacesIter).cell = NULL;
		}
	}

	DEBUG_MSG(fmt::format("RestoreEntityHandler::~RestoreEntityHandler({})\n", cellappID_));
}

//-------------------------------------------------------------------------------------
void RestoreEntityHandler::pushEntity(ENTITY_ID id)
{
	RestoreData data;
	data.id = id;
	data.creatingCell = false;
	data.processed = false;

	Base* pBase = Baseapp::getSingleton().findEntity(data.id);
	if(pBase && !pBase->isDestroyed())
	{
		data.spaceID = pBase->spaceID();

		if(pBase->isCreatedSpace())
		{
			restoreSpaces_.push_back(data);
		}
		else
		{
			entities_.push_back(data);
		}
	}

	std::vector<SPACE_ID>::const_iterator iter = std::find(spaceIDs_.begin(), spaceIDs_.end(), data.spaceID);
	if(iter == spaceIDs_.end())
	{
		spaceIDs_.push_back(data.spaceID);
	}

	if(!inProcess_)
	{
		inProcess_ = true;
		networkInterface_.dispatcher().addTask(this);
	}

	tickReport_ = timestamp();
}

//-------------------------------------------------------------------------------------
bool RestoreEntityHandler::process()
{
	Components::COMPONENTS& cts = Components::getSingleton().getComponents(CELLAPP_TYPE);
	Network::Channel* pChannel = NULL;

	if(cts.size() > 0)
	{
		Components::COMPONENTS::iterator ctiter = cts.begin();
		if((*ctiter).pChannel == NULL)
		{
			canRestore(false);
			return true;
		}

		pChannel = (*ctiter).pChannel;
	}

	if(pChannel == NULL)
	{
		canRestore(false);
		return true;
	}

	if(!canRestore())
	{
		if(timestamp() - tickReport_ > uint64( 3 * stampsPerSecond() ))
		{
			Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
			(*pBundle).newMessage(CellappInterface::requestRestore);
			(*pBundle) << cellappID();
			pChannel->send(pBundle);
		}

		return true;
	}

	int count = 0;

	// ������Ҫ�ҵ����cell�ϵ�space
	// KBE_ASSERT(restoreSpaces_.size() > 0);
	// ���spaceEntity�������baseapp�ϴ���������ȴ�
	// ��spaceEntity��cell��������֮���㲥�����е�baseapp�� ÿ��baseapp
	// ȥ�ж��Ƿ�����Ҫ�ָ���entity
	if(restoreSpaces_.size() > 0)
	{
		if(timestamp() - tickReport_ > uint64( 3 * stampsPerSecond() ))
		{
			tickReport_ = timestamp();
			INFO_MSG(fmt::format("RestoreEntityHandler::process({2}): wait for localSpace to get cell!, entitiesSize({0}), spaceSize={1}\n", 
				entities_.size(), restoreSpaces_.size(), cellappID_));
		}

		int spaceCellCount = 0;

		// ����ȴ�space�ָ�
		std::vector<RestoreData>::iterator restoreSpacesIter = restoreSpaces_.begin();
		for(; restoreSpacesIter != restoreSpaces_.end(); ++restoreSpacesIter)
		{
			Base* pBase = Baseapp::getSingleton().findEntity((*restoreSpacesIter).id);
			
			if(pBase)
			{
				if(++count > (int)g_kbeSrvConfig.getBaseApp().entityRestoreSize)
				{
					return true;
				}

				if((*restoreSpacesIter).creatingCell == false)
				{
					(*restoreSpacesIter).creatingCell = true;
					pBase->restoreCell(NULL);
				}
				else
				{
					if(pBase->cellMailbox() == NULL)
					{
						return true;
					}
					else
					{
						spaceCellCount++;
						if(!(*restoreSpacesIter).processed)
						{
							(*restoreSpacesIter).processed = true;
							pBase->onRestore();
						}
					}
				}
			}
			else
			{
				ERROR_MSG(fmt::format("RestoreEntityHandler::process({}): lose space({}).\n", cellappID_, (*restoreSpacesIter).id));
			}
		}
		
		if(spaceCellCount != (int)restoreSpaces_.size())
			return true;

		// ֪ͨ����baseapp�� space�ָ���cell
		if(!broadcastOtherBaseapps_)
		{
			broadcastOtherBaseapps_ = true;

			INFO_MSG(fmt::format("RestoreEntityHandler::process({}): begin broadcast-spaceGetCell to otherBaseapps...\n", cellappID_));

			std::vector<RestoreData>::iterator restoreSpacesIter = restoreSpaces_.begin();
			for(; restoreSpacesIter != restoreSpaces_.end(); ++restoreSpacesIter)
			{
				Base* pBase = Baseapp::getSingleton().findEntity((*restoreSpacesIter).id);
				bool destroyed = (pBase == NULL || pBase->isDestroyed());
				COMPONENT_ID baseappID = g_componentID;
				COMPONENT_ID cellappID = 0;
				SPACE_ID spaceID = (*restoreSpacesIter).spaceID;
				ENTITY_ID spaceEntityID = (*restoreSpacesIter).id;
				ENTITY_SCRIPT_UID utype = 0;

				if(!destroyed)
				{
					utype = pBase->pScriptModule()->getUType();
					cellappID = pBase->cellMailbox()->componentID();
				}

				spaceIDs_.erase(std::remove(spaceIDs_.begin(), spaceIDs_.end(), spaceID), spaceIDs_.end());

				Network::Channel* pChannel = NULL;
				Components::COMPONENTS& cts = Components::getSingleton().getComponents(BASEAPP_TYPE);
				Components::COMPONENTS::iterator comsiter = cts.begin();
				for(; comsiter != cts.end(); ++comsiter)
				{
					pChannel = (*comsiter).pChannel;
					
					if(pChannel)
					{
						INFO_MSG(fmt::format("RestoreEntityHandler::process({4}): broadcast baseapp[{0}, {1}], spaceID[{2}], utype[{3}]...\n", 
							(*comsiter).cid, pChannel->c_str(), spaceID, utype, cellappID_));

						Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
						(*pBundle).newMessage(BaseappInterface::onRestoreSpaceCellFromOtherBaseapp);
						(*pBundle) << baseappID << cellappID << spaceID << spaceEntityID << utype << destroyed;
						pChannel->send(pBundle);
					}
				}
			}
		}
	}

	if(spaceIDs_.size() > 0)
	{
		if(timestamp() - tickReport_ > uint64( 3 * stampsPerSecond() ))
		{
			tickReport_ = timestamp();
			INFO_MSG(fmt::format("RestoreEntityHandler::process({}): wait for otherBaseappSpaces to get cell!, entitiesSize({}), spaceSize={}\n", 
				cellappID_, entities_.size(), spaceIDs_.size()));
		}

		return true;
	}

	// �ָ�����entity
	std::vector<RestoreData>::iterator iter = entities_.begin();
	for(; iter != entities_.end(); )
	{
		RestoreData& data = (*iter);
		Base* pBase = Baseapp::getSingleton().findEntity(data.id);

		if(pBase)
		{
			if(++count > g_kbeSrvConfig.getBaseApp().entityRestoreSize)
			{
				return true;
			}

			if(pBase->cellMailbox() != NULL)
			{
				if(!data.processed)
				{
					data.processed = true;
					pBase->onRestore();
				}

				iter = entities_.erase(iter);
			}
			else
			{
				if(!data.creatingCell)
				{
					data.creatingCell = true;
					
					EntityMailboxAbstract* cellMailbox = NULL;
					std::vector<RestoreData>::iterator restoreSpacesIter = restoreSpaces_.begin();
					for(; restoreSpacesIter != restoreSpaces_.end(); ++restoreSpacesIter)
					{
						Base* pSpace = Baseapp::getSingleton().findEntity((*restoreSpacesIter).id);
						if(pSpace && pBase->spaceID() == pSpace->spaceID())
						{
							cellMailbox = pSpace->cellMailbox();
							break;
						}
					}
					
					if(cellMailbox == NULL)
					{
						restoreSpacesIter = otherRestoredSpaces_.begin();
						for(; restoreSpacesIter != otherRestoredSpaces_.end(); ++restoreSpacesIter)
						{
							if(pBase->spaceID() == (*restoreSpacesIter).spaceID && (*restoreSpacesIter).cell)
							{
								cellMailbox = (*restoreSpacesIter).cell;
								break;
							}
						}
					}

					if(cellMailbox)
					{
						pBase->restoreCell(cellMailbox);
					}
					else
					{
						ENTITY_ID delID = pBase->id();

						pBase->destroy();
						WARNING_MSG(fmt::format("RestoreEntityHandler::process({}): not fount spaceCell, killed base({})!", 
							cellappID_, delID));

						if(Baseapp::getSingleton().findEntity(delID) == NULL)
							iter = entities_.erase(iter);

						continue;
					}
				}

				iter++;
			}
		}
		else
		{
			iter = entities_.erase(iter);
		}
	}

	if(entities_.size() == 0)
	{
		std::vector<RestoreData>::iterator restoreSpacesIter = otherRestoredSpaces_.begin();
		for(; restoreSpacesIter != otherRestoredSpaces_.end(); ++restoreSpacesIter)
		{
			if((*restoreSpacesIter).cell)
			{
				Py_DECREF((*restoreSpacesIter).cell);
				(*restoreSpacesIter).cell = NULL;
			}
		}
		
		otherRestoredSpaces_.clear();

		inProcess_ = false;
		Baseapp::getSingleton().onRestoreEntitiesOver(this);
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void RestoreEntityHandler::onRestoreSpaceCellFromOtherBaseapp(COMPONENT_ID baseappID, COMPONENT_ID cellappID,
															 SPACE_ID spaceID, ENTITY_ID spaceEntityID, ENTITY_SCRIPT_UID utype, bool destroyed)
{
	std::vector<SPACE_ID>::size_type oldsize = spaceIDs_.size();
	spaceIDs_.erase(std::remove(spaceIDs_.begin(), spaceIDs_.end(), spaceID), spaceIDs_.end());
	if(oldsize == spaceIDs_.size())
		return;

	RestoreData data;
	data.id = spaceEntityID;
	data.creatingCell = false;
	data.processed = true;
	data.spaceID = spaceID;
	data.cell = NULL;

	if(!destroyed)
	{
		ScriptDefModule* sm = EntityDef::findScriptModule(utype);
		if(sm == NULL)
		{
			ERROR_MSG(fmt::format("RestoreEntityHandler::onRestoreSpaceCellFromOtherBaseapp({}): not found utype {}!\n", 
				cellappID_, utype));
		}
		else
		{
			if(!destroyed)
				data.cell = new EntityMailbox(sm, NULL, cellappID, spaceEntityID, MAILBOX_TYPE_CELL);
		}
	}

	otherRestoredSpaces_.push_back(data);
}


//-------------------------------------------------------------------------------------

}
