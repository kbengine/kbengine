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

#include "baseapp.hpp"
#include "base_messages_forward_handler.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
BaseMessagesForwardHandler::BaseMessagesForwardHandler(Base* pBase):
Task(),
pBase_(pBase),
completed_(false),
startForward_(false)
{
	Baseapp::getSingleton().networkInterface().mainDispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
BaseMessagesForwardHandler::~BaseMessagesForwardHandler()
{
	if(!completed_)
		Baseapp::getSingleton().networkInterface().mainDispatcher().cancelFrequentTask(this);

	std::vector<Mercury::Bundle*>::iterator iter = bufferedSendToCellappMessages_.begin();
	for(; iter != bufferedSendToCellappMessages_.end(); iter++)
	{
		Mercury::Bundle::ObjPool().reclaimObject((*iter));
	}

	bufferedSendToCellappMessages_.clear();
}

//-------------------------------------------------------------------------------------
void BaseMessagesForwardHandler::pushMessages(Mercury::Bundle* pBundle)
{
	bufferedSendToCellappMessages_.push_back(pBundle);
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

	int remainPacketSize = PACKET_MAX_SIZE_TCP;

	std::vector<Mercury::Bundle*>::iterator iter = bufferedSendToCellappMessages_.begin();
	for(; iter != bufferedSendToCellappMessages_.end(); )
	{
		pBase_->sendToCellapp((*iter));

		remainPacketSize -= (*iter)->packetsLength();

		iter = bufferedSendToCellappMessages_.erase(iter);

		if(remainPacketSize <= 0)
			return true;
	}

	return true;
}

//-------------------------------------------------------------------------------------

}
