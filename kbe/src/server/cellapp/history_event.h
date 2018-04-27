// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
