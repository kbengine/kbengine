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

#ifndef KBE_SPACE_VIEWER_H
#define KBE_SPACE_VIEWER_H

#include "common/common.h"
#include "common/tasks.h"
#include "common/timer.h"
#include "helper/debug_helper.h"
#include "helper/eventhistory_stats.h"
#include "network/interfaces.h"

namespace KBEngine { 
namespace Network
{
class NetworkInterface;
class Address;
class MessageHandler;
}

class MemoryStream;

class SpaceViewer
{
public:
	struct ViewEntity
	{
		ViewEntity()
		{
			entityID = 0;
			updateVersion = 0;
		}

		ENTITY_ID entityID;
		Position3D position;
		Direction3D direction;

		// 更新序列号， 所有实体都更新完毕则序列号+1， 在某些时候量比较大的情况每次迭代一部分实体更新
		int updateVersion;
	};

public:
	SpaceViewer();
	virtual ~SpaceViewer();
	
	virtual void timeout();
	virtual void sendStream(MemoryStream* s, int type);

	void updateViewer(const Network::Address& addr, SPACE_ID spaceID, CELL_ID cellID);

	const Network::Address& addr() const {
		return addr_;
	}

protected:
	// 改变了查看space的cell
	void onChangedSpaceOrCell();
	void resetViewer();

	void updateClient();
	void initClient();

	Network::Address addr_;

	// 当前所查看的space和cell
	SPACE_ID spaceID_;
	CELL_ID cellID_;

	std::map< ENTITY_ID, ViewEntity > viewedEntities;

	int updateType_;

	// 更新序列号， 所有实体都更新完毕则序列号+1， 在某些时候量比较大的情况每次迭代一部分实体更新
	int lastUpdateVersion_;
};

class SpaceViewers : public TimerHandler
{
public:
	SpaceViewers();
	virtual ~SpaceViewers();

	void clear() {
		spaceViews_.clear();
	}

	bool addTimer();
	void finalise();

	void updateSpaceViewer(const Network::Address& addr, SPACE_ID spaceID, CELL_ID cellID, bool del);

protected:
	virtual void handleTimeout(TimerHandle handle, void * arg);
	TimerHandle reportLimitTimerHandle_;

	std::map< Network::Address, SpaceViewer> spaceViews_;
};

}

#endif // KBE_SPACE_VIEWER_H
