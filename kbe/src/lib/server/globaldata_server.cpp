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
#include "globaldata_server.hpp"
#include "components.hpp"
#include "network/channel.hpp"

#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"

namespace KBEngine{ 
		
//-------------------------------------------------------------------------------------
GlobalDataServer::GlobalDataServer(DATA_TYPE dataType):
dataType_(dataType)
{
}

//-------------------------------------------------------------------------------------
GlobalDataServer::~GlobalDataServer()
{
}

//-------------------------------------------------------------------------------------
bool GlobalDataServer::write(Mercury::Channel* pChannel, COMPONENT_TYPE componentType, 
	const std::string& key, const std::string& value)
{
	// 广播所做的改变
	broadcastDataChange(pChannel, componentType, key, value);

	DATA_MAP_KEY iter = dict_.find(key);
	if(iter != dict_.end()){
		iter->second = value.c_str();
		return true;
	}
	
	dict_[key] = value;
	return true;	
}

//-------------------------------------------------------------------------------------
bool GlobalDataServer::del(Mercury::Channel* pChannel, COMPONENT_TYPE componentType, const std::string& key)
{
	if(!dict_.erase(key)){
		ERROR_MSG("GlobalDataServer::del: not found the key:[%s]\n", key.c_str());
		return false;
	}

	broadcastDataChange(pChannel, componentType, key, "", true);
	return true;	
}

//-------------------------------------------------------------------------------------
void GlobalDataServer::broadcastDataChange(Mercury::Channel* pChannel, COMPONENT_TYPE componentType, 
										const std::string& key, const std::string& value, bool isDelete)
{
	std::vector<COMPONENT_TYPE>::iterator iter = concernComponentTypes_.begin();
	for(; iter != concernComponentTypes_.end(); iter++)
	{
		COMPONENT_TYPE ct = (*iter);
		Components::COMPONENTS& channels = Components::getSingleton().getComponents(ct);
		Components::COMPONENTS::iterator iter1 = channels.begin();
		
		for(; iter1 != channels.end(); iter1++)
		{
			Mercury::Channel* lpChannel = iter1->pChannel;
			KBE_ASSERT(lpChannel != NULL);

			if(pChannel == lpChannel)
				continue;

			Mercury::Bundle bundle;

			switch(dataType_)
			{
			case GLOBAL_DATA:
				if(componentType == CELLAPP_TYPE)
				{
					bundle.newMessage(CellappInterface::onBroadcastGlobalDataChange);
				}
				else if(componentType == BASEAPP_TYPE)
				{
					bundle.newMessage(BaseappInterface::onBroadcastGlobalDataChange);
				}
				else
				{
					KBE_ASSERT(false && "componentType is error!\n");
				}
				break;
			case GLOBAL_BASES:
				bundle.newMessage(BaseappInterface::onBroadcastGlobalBasesChange);
				break;
			case CELLAPP_DATA:
				bundle.newMessage(CellappInterface::onBroadcastCellAppDataChange);
				break;
			default:
				KBE_ASSERT(false && "dataType is error!\n");
				break;
			};

			
			bundle << isDelete;
			bundle << key;
			if(!isDelete)
				bundle << value;

			bundle.send(*pChannel->endpoint());
		}
	}
}

//-------------------------------------------------------------------------------------
void GlobalDataServer::onGlobalDataClientLogon(Mercury::Channel* client, COMPONENT_TYPE componentType)
{
	bool isDelete = false;

	DATA_MAP_KEY iter = dict_.begin();
	for(; iter != dict_.end(); iter++)
	{
		Mercury::Bundle bundle;
		
		switch(dataType_)
		{
		case GLOBAL_DATA:
			if(componentType == CELLAPP_TYPE)
			{
				bundle.newMessage(CellappInterface::onBroadcastGlobalDataChange);
			}
			else if(componentType == BASEAPP_TYPE)
			{
				bundle.newMessage(BaseappInterface::onBroadcastGlobalDataChange);
			}
			else
			{
				KBE_ASSERT(false && "componentType is error!\n");
			}
			break;
		case GLOBAL_BASES:
			bundle.newMessage(BaseappInterface::onBroadcastGlobalDataChange);
			break;
		case CELLAPP_DATA:
			bundle.newMessage(CellappInterface::onBroadcastGlobalDataChange);
			break;
		default:
			KBE_ASSERT(false && "dataType is error!\n");
			break;
		};

		bundle << isDelete;
		bundle << iter->first.c_str();
		bundle << iter->second.c_str();
		bundle.send(*client->endpoint());
	}
}

//-------------------------------------------------------------------------------------
}
