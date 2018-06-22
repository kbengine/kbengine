// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "proxy.h"
#include "baseapp.h"
#include "entity_messages_forward_handler.h"
#include "network/bundle.h"
#include "network/channel.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
EntityMessagesForwardCellappHandler::EntityMessagesForwardCellappHandler(Entity* pEntity):
Task(),
pEntity_(pEntity),
completed_(false),
startForward_(false),
createTime_(0)
{
	DEBUG_MSG(fmt::format("EntityMessagesForwardCellappHandler::EntityMessagesForwardCellappHandler() : entityID({})!\n", 
		(pEntity_ ? pEntity_->id() : 0)));
	
	Baseapp::getSingleton().networkInterface().dispatcher().addTask(this);

	createTime_ = timestamp();
}

//-------------------------------------------------------------------------------------
EntityMessagesForwardCellappHandler::~EntityMessagesForwardCellappHandler()
{
	DEBUG_MSG(fmt::format("EntityMessagesForwardCellappHandler::~EntityMessagesForwardCellappHandler(): size({}), entityID({})!\n", 
		bufferedSendToCellappMessages_.size(), (pEntity_ ? pEntity_->id() : 0)));

	if(!completed_)
		Baseapp::getSingleton().networkInterface().dispatcher().cancelTask(this);

	std::vector<Network::Bundle*>::iterator iter = bufferedSendToCellappMessages_.begin();
	for(; iter != bufferedSendToCellappMessages_.end(); ++iter)
	{
		Network::Bundle::reclaimPoolObject((*iter));
	}

	bufferedSendToCellappMessages_.clear();
}

//-------------------------------------------------------------------------------------
void EntityMessagesForwardCellappHandler::pushMessages(Network::Bundle* pBundle)
{
	bufferedSendToCellappMessages_.push_back(pBundle);
	
	size_t msgsize = bufferedSendToCellappMessages_.size();

	if(msgsize > 4096 && msgsize <= 8192)
	{
		WARNING_MSG(fmt::format("EntityMessagesForwardCellappHandler::pushMessages(): size({}) > 4096! entityID={}\n", 
			msgsize, (pEntity_ ? pEntity_->id() : 0)));
	}
	else if(msgsize > 8192)
	{
		ERROR_MSG(fmt::format("EntityMessagesForwardCellappHandler::pushMessages(): size({}) > 8192! entityID={}\n", 
			msgsize, (pEntity_ ? pEntity_->id() : 0)));
		
		startForward();
	}
}

//-------------------------------------------------------------------------------------
void EntityMessagesForwardCellappHandler::startForward()
{
	startForward_ = true;

	DEBUG_MSG(fmt::format("EntityMessagesForwardCellappHandler::startForward(): size({}), entityID({})!\n", 
		bufferedSendToCellappMessages_.size(), (pEntity_ ? pEntity_->id() : 0)));
}

//-------------------------------------------------------------------------------------
bool EntityMessagesForwardCellappHandler::process()
{
	if (!startForward_)
	{
		if (timestamp() - createTime_ >= uint64(5 * stampsPerSecond()))
		{
			ERROR_MSG(fmt::format("EntityMessagesForwardCellappHandler::process(): Wait for a timeout({}s)! size={}, entityID={}\n",
				((timestamp() - createTime_) / stampsPerSecond()), bufferedSendToCellappMessages_.size(), (pEntity_ ? pEntity_->id() : 0)));

			startForward_ = true;
		}

		return true;
	}

	if(bufferedSendToCellappMessages_.size() == 0)
	{
		completed_ = true;
		pEntity_->onBufferedForwardToCellappMessagesOver();
		return false;
	}

	if(pEntity_->cellEntityCall() == NULL || pEntity_->cellEntityCall()->getChannel() == NULL)
	{
		WARNING_MSG(fmt::format("EntityMessagesForwardCellappHandler::process(): no cell! size={}, entityID={}\n", 
			bufferedSendToCellappMessages_.size(), (pEntity_ ? pEntity_->id() : 0)));
		
		completed_ = true;
		pEntity_->onBufferedForwardToCellappMessagesOver();
		return false;
	}

	int remainPacketSize = PACKET_MAX_SIZE_TCP * 10;

	std::vector<Network::Bundle*>::iterator iter = bufferedSendToCellappMessages_.begin();
	for(; iter != bufferedSendToCellappMessages_.end(); )
	{
		Network::Bundle* pBundle = (*iter); 
		remainPacketSize -= pBundle->packetsLength();
		iter = bufferedSendToCellappMessages_.erase(iter);
		pEntity_->sendToCellapp(pBundle);

		if(remainPacketSize <= 0)
			return true;
	}

	return true;
}

//-------------------------------------------------------------------------------------
BaseMessagesForwardClientHandler::BaseMessagesForwardClientHandler(Entity* pEntity, COMPONENT_ID cellappID):
Task(),
pEntity_(pEntity),
completed_(false),
startForward_(false),
cellappID_(cellappID),
createTime_(0)
{
	DEBUG_MSG(fmt::format("BaseMessagesForwardClientHandler::BaseMessagesForwardClientHandler() : cellappID({}), entityID({})!\n", 
		cellappID_, (pEntity_ ? pEntity_->id() : 0)));
	
	Baseapp::getSingleton().networkInterface().dispatcher().addTask(this);

	createTime_ = timestamp();
}

//-------------------------------------------------------------------------------------
BaseMessagesForwardClientHandler::~BaseMessagesForwardClientHandler()
{
	DEBUG_MSG(fmt::format("BaseMessagesForwardClientHandler::~BaseMessagesForwardClientHandler(): size({}), cellappID({}), entityID({})!\n", 
		bufferedSendToClientMessages_.size(), cellappID_, (pEntity_ ? pEntity_->id() : 0)));

	if(!completed_)
		Baseapp::getSingleton().networkInterface().dispatcher().cancelTask(this);

	std::vector<Network::Bundle*>::iterator iter = bufferedSendToClientMessages_.begin();
	for(; iter != bufferedSendToClientMessages_.end(); ++iter)
	{
		Network::Bundle::reclaimPoolObject((*iter));
	}

	bufferedSendToClientMessages_.clear();
}

//-------------------------------------------------------------------------------------
void BaseMessagesForwardClientHandler::pushMessages(Network::Bundle* pBundle)
{
	bufferedSendToClientMessages_.push_back(pBundle);
	
	size_t msgsize = bufferedSendToClientMessages_.size();
	
	if(msgsize > 4096 && msgsize <= 10240)
	{
		WARNING_MSG(fmt::format("BaseMessagesForwardClientHandler::pushMessages(): size({}) > 4096! cellappID={}, entityID={}\n", 
			msgsize, cellappID_, (pEntity_ ? pEntity_->id() : 0)));
	}
	else if(msgsize > 10240)
	{
		ERROR_MSG(fmt::format("BaseMessagesForwardClientHandler::pushMessages(): size({}) > 10240! cellappID={}, entityID={}\n", 
			msgsize, cellappID_, (pEntity_ ? pEntity_->id() : 0)));
	}
}

//-------------------------------------------------------------------------------------
void BaseMessagesForwardClientHandler::startForward()
{
	startForward_ = true;

	DEBUG_MSG(fmt::format("BaseMessagesForwardClientHandler::startForward(): size({}), cellappID({}), entityID({})!\n", 
		bufferedSendToClientMessages_.size(), cellappID_, (pEntity_ ? pEntity_->id() : 0)));
}

//-------------------------------------------------------------------------------------
bool BaseMessagesForwardClientHandler::process()
{
	if (!startForward_)
	{
		if (timestamp() - createTime_ >= uint64(300 * stampsPerSecond()))
		{
			ERROR_MSG(fmt::format("BaseMessagesForwardClientHandler::process(): Wait for a timeout({}s)! size={}, entityID={}\n",
				((timestamp() - createTime_) / stampsPerSecond()), bufferedSendToClientMessages_.size(), (pEntity_ ? pEntity_->id() : 0)));

			startForward_ = true;
		}

		return true;
	}

	if(bufferedSendToClientMessages_.size() == 0)
	{
		completed_ = true;
		pEntity_->onBufferedForwardToClientMessagesOver();
		return false;
	}

	if(pEntity_->clientEntityCall() == NULL || pEntity_->clientEntityCall()->getChannel() == NULL)
	{
		WARNING_MSG(fmt::format("BaseMessagesForwardClientHandler::process(): no client! size={}, entityID={}\n", 
			bufferedSendToClientMessages_.size(), (pEntity_ ? pEntity_->id() : 0)));
		
		completed_ = true;
		pEntity_->onBufferedForwardToClientMessagesOver();
		return false;
	}

	int remainPacketSize = PACKET_MAX_SIZE_TCP * 10;

	std::vector<Network::Bundle*>::iterator iter = bufferedSendToClientMessages_.begin();
	for(; iter != bufferedSendToClientMessages_.end(); )
	{
		Network::Bundle* pBundle = (*iter); 
		remainPacketSize -= pBundle->packetsLength();
		iter = bufferedSendToClientMessages_.erase(iter);
		static_cast<Proxy*>(pEntity_)->sendToClient(pBundle);

		if(remainPacketSize <= 0)
			return true;
	}

	return true;
}

//-------------------------------------------------------------------------------------

}
