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

#ifndef KBE_HISTORY_EVENT_H
#define KBE_HISTORY_EVENT_H

#include "helper/debug_helper.h"
#include "common/common.h"	


namespace KBEngine{
namespace Network{
	class MessageHandler;
}

typedef uint32 HistoryEventID;

/**
	描述一个历史事件
*/
class HistoryEvent
{
public:
	HistoryEvent(HistoryEventID id, const Network::MessageHandler& msgHandler, uint32 msglen);
	virtual ~HistoryEvent();

	HistoryEventID id() const{ return id_; }
	uint32 msglen() const { return msglen_; }

	void addMsgLen(uint32 v){ msglen_ += v; }

protected:
	HistoryEventID id_;
	uint32 msglen_;
	const Network::MessageHandler& msgHandler_;
};

/**
	管理所有的历史事件
*/
class EventHistory
{
public:
	EventHistory();
	virtual ~EventHistory();

	bool add(HistoryEvent* phe);
	bool remove(HistoryEvent* phe);
protected:
	std::map<HistoryEventID, HistoryEvent*> events_;
};

}
#endif
