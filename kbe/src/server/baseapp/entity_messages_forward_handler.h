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

#ifndef KBE_BASE_ENTITY_MESSAGES_FORWARD_HANDLER_H
#define KBE_BASE_ENTITY_MESSAGES_FORWARD_HANDLER_H

#include "helper/debug_helper.h"
#include "common/common.h"

namespace KBEngine{

class Entity;
class EntityMessagesForwardCellappHandler : public Task
{
public:
	EntityMessagesForwardCellappHandler(Entity* pEntity);
	~EntityMessagesForwardCellappHandler();
	
	bool process();

	void pushMessages(Network::Bundle* pBundle);

	void startForward();
	void stopForward(){ startForward_ = false; }

	bool isStop() const{ return !startForward_; }

private:
	Entity* pEntity_;
	bool completed_;
	std::vector<Network::Bundle*> bufferedSendToCellappMessages_;
	bool startForward_;
	uint64 createTime_;
};

class BaseMessagesForwardClientHandler : public Task
{
public:
	BaseMessagesForwardClientHandler(Entity* pEntity, COMPONENT_ID cellappID);
	~BaseMessagesForwardClientHandler();
	
	bool process();

	void pushMessages(Network::Bundle* pBundle);

	void startForward();
	void stopForward(){ startForward_ = false; }

	bool isStop() const{ return !startForward_; }
	
	COMPONENT_ID cellappID() const {
		return cellappID_;
	}

	void cellappID(COMPONENT_ID cid) {
		cellappID_ = cid;
	}

private:
	Entity* pEntity_;
	bool completed_;
	std::vector<Network::Bundle*> bufferedSendToClientMessages_;
	bool startForward_;
	COMPONENT_ID cellappID_;
	uint64 createTime_;
};

}

#endif // KBE_BASE_ENTITY_MESSAGES_FORWARD_HANDLER_H
