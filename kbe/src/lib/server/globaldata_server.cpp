// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#include "globaldata_server.h"
#include "components.h"
#include "network/channel.h"

#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/baseapp/baseapp_interface.h"

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
bool GlobalDataServer::write(Network::Channel* pChannel, COMPONENT_TYPE componentType, 
	const std::string& key, const std::string& value)
{
	// 广播所做的改变
	broadcastDataChanged(pChannel, componentType, key, value);

	DATA_MAP_KEY iter = dict_.find(key);
	if(iter != dict_.end()){
		iter->second = value.c_str();
		return true;
	}
	
	dict_[key] = value;
	return true;	
}

//-------------------------------------------------------------------------------------
bool GlobalDataServer::del(Network::Channel* pChannel, COMPONENT_TYPE componentType, const std::string& key)
{
	if(!dict_.erase(key)){
		ERROR_MSG(fmt::format("GlobalDataServer::del: not found the key:[{}]\n", key.c_str()));
		return false;
	}

	broadcastDataChanged(pChannel, componentType, key, "", true);
	return true;	
}

//-------------------------------------------------------------------------------------
void GlobalDataServer::broadcastDataChanged(Network::Channel* pChannel, COMPONENT_TYPE componentType, 
										const std::string& key, const std::string& value, bool isDelete)
{
	INFO_MSG(fmt::format("GlobalDataServer::broadcastDataChanged: writer({0}, addr={4}), keySize={1}, valSize={2}, isDelete={3}\n",
		COMPONENT_NAME_EX(componentType), key.size(), value.size(), (int)isDelete, pChannel->c_str()));

	std::vector<COMPONENT_TYPE>::iterator iter = concernComponentTypes_.begin();
	for(; iter != concernComponentTypes_.end(); ++iter)
	{
		COMPONENT_TYPE ct = (*iter);
		Components::COMPONENTS& channels = Components::getSingleton().getComponents(ct);
		Components::COMPONENTS::iterator iter1 = channels.begin();
		
		for(; iter1 != channels.end(); ++iter1)
		{
			Network::Channel* lpChannel = iter1->pChannel;
			KBE_ASSERT(lpChannel != NULL);

			if(pChannel == lpChannel)
				continue;

			if(dataType_ == BASEAPP_DATA && iter1->componentType != BASEAPP_TYPE)
				continue;
				
			if(dataType_ == CELLAPP_DATA && iter1->componentType != CELLAPP_TYPE)
				continue;

			Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

			switch(dataType_)
			{
			case GLOBAL_DATA:
				if(iter1->componentType == CELLAPP_TYPE)
				{
					(*pBundle).newMessage(CellappInterface::onBroadcastGlobalDataChanged);
				}
				else if(iter1->componentType == BASEAPP_TYPE)
				{
					(*pBundle).newMessage(BaseappInterface::onBroadcastGlobalDataChanged);
				}
				else
				{
					KBE_ASSERT(false && "componentType error!\n");
				}
				break;
			case BASEAPP_DATA:
				(*pBundle).newMessage(BaseappInterface::onBroadcastBaseAppDataChanged);
				break;
			case CELLAPP_DATA:
				(*pBundle).newMessage(CellappInterface::onBroadcastCellAppDataChanged);
				break;
			default:
				KBE_ASSERT(false && "dataType error!\n");
				break;
			};

			
			(*pBundle) << isDelete;
			ArraySize slen = key.size();
			(*pBundle) << slen;
			(*pBundle).assign(key.data(), slen);

			if(!isDelete)
			{
				slen = value.size();
				(*pBundle) << slen;
				(*pBundle).assign(value.data(), slen);
			}

			lpChannel->send(pBundle);
		}
	}
}

//-------------------------------------------------------------------------------------
void GlobalDataServer::onGlobalDataClientLogon(Network::Channel* client, COMPONENT_TYPE componentType)
{
	bool isDelete = false;

	DATA_MAP_KEY iter = dict_.begin();
	for(; iter != dict_.end(); ++iter)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		
		switch(dataType_)
		{
		case GLOBAL_DATA:
			if(componentType == CELLAPP_TYPE)
			{
				(*pBundle).newMessage(CellappInterface::onBroadcastGlobalDataChanged);
			}
			else if(componentType == BASEAPP_TYPE)
			{
				(*pBundle).newMessage(BaseappInterface::onBroadcastGlobalDataChanged);
			}
			else
			{
				KBE_ASSERT(false && "componentType error!\n");
			}
			break;
		case BASEAPP_DATA:
			if(componentType != BASEAPP_TYPE)
			{
				Network::Bundle::reclaimPoolObject(pBundle);
				continue;
			}

			(*pBundle).newMessage(BaseappInterface::onBroadcastBaseAppDataChanged);
			break;
		case CELLAPP_DATA:
			if(componentType != CELLAPP_TYPE)
			{
				Network::Bundle::reclaimPoolObject(pBundle);
				continue;
			}

			(*pBundle).newMessage(CellappInterface::onBroadcastCellAppDataChanged);
			break;
		default:
			KBE_ASSERT(false && "dataType error!\n");
			break;
		};

		(*pBundle) << isDelete;

		ArraySize slen = iter->first.size();
		(*pBundle) << slen;
		(*pBundle).assign(iter->first.data(), slen);

		slen = iter->second.size();
		(*pBundle) << slen;
		(*pBundle).assign(iter->second.data(), slen);

		client->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------
}
