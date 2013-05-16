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
#include "base.hpp"
#include "baseapp.hpp"
#include "restore_entity_handler.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
RestoreEntityHandler::RestoreEntityHandler(Mercury::NetworkInterface & networkInterface):
Task(),
networkInterface_(networkInterface),
entities_(),
inProcess_(false),
restoreSpaces_()
{
	// networkInterface_.mainDispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
RestoreEntityHandler::~RestoreEntityHandler()
{
	if(inProcess_)
		networkInterface_.mainDispatcher().cancelFrequentTask(this);

	 DEBUG_MSG("RestoreEntityHandler::~RestoreEntityHandler()\n");
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
		data.spaceID = pBase->getSpaceID();

		if(pBase->isCreatedSpace())
		{
			restoreSpaces_.push_back(data);
		}
		else
		{
			entities_.push_back(data);
		}
	}

	if(!inProcess_)
	{
		inProcess_ = true;
		networkInterface_.mainDispatcher().addFrequentTask(this);
	}
}

//-------------------------------------------------------------------------------------
bool RestoreEntityHandler::process()
{
	Components::COMPONENTS cts = Components::getSingleton().getComponents(CELLAPP_TYPE);
	Mercury::Channel* pChannel = NULL;

	if(cts.size() > 0)
	{
		Components::COMPONENTS::iterator ctiter = cts.begin();
		if((*ctiter).pChannel == NULL)
			return true;

		pChannel = (*ctiter).pChannel;
	}

	if(pChannel == NULL)
		return true;

	// 首先需要找到这个cell上的space
	KBE_ASSERT(restoreSpaces_.size() > 0);
	
	int count = 0;
	int spaceCellCount = 0;

	// 必须等待space恢复
	std::vector<RestoreData>::iterator restoreSpacesIter = restoreSpaces_.begin();
	for(; restoreSpacesIter != restoreSpaces_.end(); restoreSpacesIter++)
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
				if(pBase->getCellMailbox() == NULL)
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
			ERROR_MSG(boost::format("RestoreEntityHandler::process(): lose space(%1%).\n") % (*restoreSpacesIter).id);
		}
	}
	
	if(spaceCellCount != (int)restoreSpaces_.size())
		return true;

	// 恢复其他entity
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

			if(pBase->getCellMailbox() != NULL)
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
					for(; restoreSpacesIter != restoreSpaces_.end(); restoreSpacesIter++)
					{
						Base* pSpace = Baseapp::getSingleton().findEntity((*restoreSpacesIter).id);
						if(pBase->getSpaceID() == pSpace->getSpaceID())
						{
							cellMailbox = pSpace->getCellMailbox();
							break;
						}
					}
					
					pBase->restoreCell(cellMailbox);
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
		inProcess_ = false;
		Baseapp::getSingleton().onResoreEntitiesOver(this);
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------

}
