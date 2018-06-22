// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_RESTORE_ENTITY_HANDLER_H
#define KBE_RESTORE_ENTITY_HANDLER_H

#include "helper/debug_helper.h"
#include "common/common.h"

namespace KBEngine{

class Entity;
class EntityCall;

class RestoreEntityHandler : public Task
{
	struct RestoreData
	{
		ENTITY_ID id;
		bool creatingCell;
		bool processed;
		SPACE_ID spaceID;
		EntityCall* cell;
	};

public:
	RestoreEntityHandler(COMPONENT_ID cellappID, Network::NetworkInterface & networkInterface);
	~RestoreEntityHandler();
	
	bool process();
	
	void pushEntity(ENTITY_ID id);

	/** 
		某个baseapp上的space恢复了cell， 判断当前baseapp是否有相关entity需要恢复cell
	*/
	void onRestoreSpaceCellFromOtherBaseapp(COMPONENT_ID baseappID, COMPONENT_ID cellappID, 
		SPACE_ID spaceID, ENTITY_ID spaceEntityID, ENTITY_SCRIPT_UID utype, bool destroyed);

	/** 
		是否允许进行恢复
	*/
	bool canRestore() const{ return canRestore_; }
	void canRestore(bool v){ canRestore_ = v; }

	COMPONENT_ID cellappID() const{ return cellappID_; }
private:
	Network::NetworkInterface & networkInterface_;
	std::vector<RestoreData> entities_;
	bool inProcess_;

	std::vector<RestoreData> restoreSpaces_;
	std::vector<RestoreData> otherRestoredSpaces_;

	// space创建好cell后广播给其他baseapp
	bool broadcastOtherBaseapps_;

	uint64 tickReport_;

	std::vector<SPACE_ID> spaceIDs_;

	COMPONENT_ID cellappID_;

	bool canRestore_;
};


}

#endif // KBE_RESTORE_ENTITY_HANDLER_H
