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

#include "baseapp.h"
#include "base_messages_forward_handler.h"
#include "network/bundle.h"
#include "network/channel.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
BaseMessagesForwardHandler::BaseMessagesForwardHandler(Base* pBase):
Task(),
pBase_(pBase),
completed_(false),
startForward_(false)
{
	Baseapp::getSingleton().networkInterface().dispatcher().addTask(this);
}

//-------------------------------------------------------------------------------------
BaseMessagesForwardHandler::~BaseMessagesForwardHandler()
{
	DEBUG_MSG(fmt::format("BaseMessagesForwardHandler::~BaseMessagesForwardHandler(): size({})!\n", 
		bufferedSendToCellappMessages_.size()));

	if(!completed_)
		Baseapp::getSingleton().networkInterface().dispatcher().cancelTask(this);

	std::vector<Network::Bundle*>::iterator iter = bufferedSendToCellappMessages_.begin();
	for(; iter != bufferedSendToCellappMessages_.end(); ++iter)
	{
		Network::Bundle::ObjPool().reclaimObject((*iter));
	}

	bufferedSendToCellappMessages_.clear();
}

//-------------------------------------------------------------------------------------
void BaseMessagesForwardHandler::pushMessages(Network::Bundle* pBundle)
{
	bufferedSendToCellappMessages_.push_back(pBundle);
}

//-------------------------------------------------------------------------------------
void BaseMessagesForwardHandler::startForward()
{
	startForward_ = true;
	DEBUG_MSG(fmt::format("BaseMessagesForwardHandler::startForward(): size({})!\n", bufferedSendToCellappMessages_.size()));
}

//-------------------------------------------------------------------------------------
bool BaseMessagesForwardHandler::process()
{
	if(!startForward_)
		return true;

	if(bufferedSendToCellappMessages_.size() == 0)
	{
		completed_ = true;
		pBase_->onBufferedForwardToCellappMessagesOver();
		return false;
	}

	if(pBase_->cellMailbox() == NULL || pBase_->cellMailbox()->getChannel() == NULL)
		return true;

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

}
