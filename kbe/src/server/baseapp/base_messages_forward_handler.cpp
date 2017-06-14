/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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

#include "proxy.h"
#include "baseapp.h"
#include "base_messages_forward_handler.h"
#include "network/bundle.h"
#include "network/channel.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
BaseMessagesForwardCellappHandler::BaseMessagesForwardCellappHandler(Base* pBase):
Task(),
pBase_(pBase),
completed_(false),
startForward_(false),
createTime_(0)
{
	DEBUG_MSG(fmt::format("BaseMessagesForwardCellappHandler::BaseMessagesForwardCellappHandler() : entityID({})!\n", 
		(pBase_ ? pBase_->id() : 0)));
	
	Baseapp::getSingleton().networkInterface().dispatcher().addTask(this);

	createTime_ = timestamp();
}

//-------------------------------------------------------------------------------------
BaseMessagesForwardCellappHandler::~BaseMessagesForwardCellappHandler()
{
	DEBUG_MSG(fmt::format("BaseMessagesForwardCellappHandler::~BaseMessagesForwardCellappHandler(): size({}), entityID({})!\n", 
		bufferedSendToCellappMessages_.size(), (pBase_ ? pBase_->id() : 0)));

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
void BaseMessagesForwardCellappHandler::pushMessages(Network::Bundle* pBundle)
{
	bufferedSendToCellappMessages_.push_back(pBundle);
	
	size_t msgsize = bufferedSendToCellappMessages_.size();

	if(msgsize > 4096 && msgsize <= 8192)
	{
		WARNING_MSG(fmt::format("BaseMessagesForwardCellappHandler::pushMessages(): size({}) > 4096! entityID={}\n", 
			msgsize, (pBase_ ? pBase_->id() : 0)));
	}
	else if(msgsize > 8192)
	{
		ERROR_MSG(fmt::format("BaseMessagesForwardCellappHandler::pushMessages(): size({}) > 8192! entityID={}\n", 
			msgsize, (pBase_ ? pBase_->id() : 0)));
		
		startForward();
	}
}

//-------------------------------------------------------------------------------------
void BaseMessagesForwardCellappHandler::startForward()
{
	startForward_ = true;

	DEBUG_MSG(fmt::format("BaseMessagesForwardCellappHandler::startForward(): size({}), entityID({})!\n", 
		bufferedSendToCellappMessages_.size(), (pBase_ ? pBase_->id() : 0)));
}

//-------------------------------------------------------------------------------------
bool BaseMessagesForwardCellappHandler::process()
{
	if (!startForward_)
	{
		if (timestamp() - createTime_ >= uint64(5 * stampsPerSecond()))
		{
			ERROR_MSG(fmt::format("BaseMessagesForwardCellappHandler::process(): Wait for a timeout({}s)! size={}, entityID={}\n",
				((timestamp() - createTime_) / stampsPerSecond()), bufferedSendToCellappMessages_.size(), (pBase_ ? pBase_->id() : 0)));

			startForward_ = true;
		}

		return true;
	}

	if(bufferedSendToCellappMessages_.size() == 0)
	{
		completed_ = true;
		pBase_->onBufferedForwardToCellappMessagesOver();
		return false;
	}

	if(pBase_->cellMailbox() == NULL || pBase_->cellMailbox()->getChannel() == NULL)
	{
		WARNING_MSG(fmt::format("BaseMessagesForwardCellappHandler::process(): no cell! size={}, entityID={}\n", 
			bufferedSendToCellappMessages_.size(), (pBase_ ? pBase_->id() : 0)));
		
		completed_ = true;
		pBase_->onBufferedForwardToCellappMessagesOver();
		return false;
	}

	int remainPacketSize = PACKET_MAX_SIZE_TCP * 10;

	std::vector<Network::Bundle*>::iterator iter = bufferedSendToCellappMessages_.begin();
	for(; iter != bufferedSendToCellappMessages_.end(); )
	{
		Network::Bundle* pBundle = (*iter); 
		remainPacketSize -= pBundle->packetsLength();
		iter = bufferedSendToCellappMessages_.erase(iter);
		pBase_->sendToCellapp(pBundle);

		if(remainPacketSize <= 0)
			return true;
	}

	return true;
}

//-------------------------------------------------------------------------------------
BaseMessagesForwardClientHandler::BaseMessagesForwardClientHandler(Base* pBase, COMPONENT_ID cellappID):
Task(),
pBase_(pBase),
completed_(false),
startForward_(false),
cellappID_(cellappID),
createTime_(0)
{
	DEBUG_MSG(fmt::format("BaseMessagesForwardClientHandler::BaseMessagesForwardClientHandler() : cellappID({}), entityID({})!\n", 
		cellappID_, (pBase_ ? pBase_->id() : 0)));
	
	Baseapp::getSingleton().networkInterface().dispatcher().addTask(this);

	createTime_ = timestamp();
}

//-------------------------------------------------------------------------------------
BaseMessagesForwardClientHandler::~BaseMessagesForwardClientHandler()
{
	DEBUG_MSG(fmt::format("BaseMessagesForwardClientHandler::~BaseMessagesForwardClientHandler(): size({}), cellappID({}), entityID({})!\n", 
		bufferedSendToClientMessages_.size(), cellappID_, (pBase_ ? pBase_->id() : 0)));

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
			msgsize, cellappID_, (pBase_ ? pBase_->id() : 0)));
	}
	else if(msgsize > 10240)
	{
		ERROR_MSG(fmt::format("BaseMessagesForwardClientHandler::pushMessages(): size({}) > 10240! cellappID={}, entityID={}\n", 
			msgsize, cellappID_, (pBase_ ? pBase_->id() : 0)));
	}
}

//-------------------------------------------------------------------------------------
void BaseMessagesForwardClientHandler::startForward()
{
	startForward_ = true;

	DEBUG_MSG(fmt::format("BaseMessagesForwardClientHandler::startForward(): size({}), cellappID({}), entityID({})!\n", 
		bufferedSendToClientMessages_.size(), cellappID_, (pBase_ ? pBase_->id() : 0)));
}

//-------------------------------------------------------------------------------------
bool BaseMessagesForwardClientHandler::process()
{
	if (!startForward_)
	{
		if (timestamp() - createTime_ >= uint64(300 * stampsPerSecond()))
		{
			ERROR_MSG(fmt::format("BaseMessagesForwardClientHandler::process(): Wait for a timeout({}s)! size={}, entityID={}\n",
				((timestamp() - createTime_) / stampsPerSecond()), bufferedSendToClientMessages_.size(), (pBase_ ? pBase_->id() : 0)));

			startForward_ = true;
		}

		return true;
	}

	if(bufferedSendToClientMessages_.size() == 0)
	{
		completed_ = true;
		pBase_->onBufferedForwardToClientMessagesOver();
		return false;
	}

	if(pBase_->clientMailbox() == NULL || pBase_->clientMailbox()->getChannel() == NULL)
	{
		WARNING_MSG(fmt::format("BaseMessagesForwardClientHandler::process(): no client! size={}, entityID={}\n", 
			bufferedSendToClientMessages_.size(), (pBase_ ? pBase_->id() : 0)));
		
		completed_ = true;
		pBase_->onBufferedForwardToClientMessagesOver();
		return false;
	}

	int remainPacketSize = PACKET_MAX_SIZE_TCP * 10;

	std::vector<Network::Bundle*>::iterator iter = bufferedSendToClientMessages_.begin();
	for(; iter != bufferedSendToClientMessages_.end(); )
	{
		Network::Bundle* pBundle = (*iter); 
		remainPacketSize -= pBundle->packetsLength();
		iter = bufferedSendToClientMessages_.erase(iter);
		static_cast<Proxy*>(pBase_)->sendToClient(pBundle);

		if(remainPacketSize <= 0)
			return true;
	}

	return true;
}

//-------------------------------------------------------------------------------------

}
