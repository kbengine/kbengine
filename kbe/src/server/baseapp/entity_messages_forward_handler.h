// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
