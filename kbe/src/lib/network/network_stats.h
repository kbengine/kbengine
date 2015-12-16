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

#ifndef KBE_NETWORK_STATS_H
#define KBE_NETWORK_STATS_H

#include "network/interfaces.h"
#include "common/common.h"
#include "common/singleton.h"

namespace KBEngine { 
namespace Network
{

class MessageHandler;

/*
	记录network流量等信息
*/
class NetworkStats : public Singleton<NetworkStats>
{
public:
	enum S_OP{
		SEND,
		RECV
	};

	struct Stats
	{
		Stats()
		{
			name = "";
			send_count = 0;
			send_size = 0;
			recv_size = 0;
			recv_count = 0;
		}

		std::string name;
		uint32 send_size;
		uint32 send_count;
		uint32 recv_size;
		uint32 recv_count;
	};

	typedef KBEUnordered_map<std::string, Stats> STATS;

	NetworkStats();
	~NetworkStats();

	void trackMessage(S_OP op, const MessageHandler& msgHandler, uint32 size);

	NetworkStats::STATS& stats(){ return stats_; }

	void addHandler(NetworkStatsHandler* pHandler);
	void removeHandler(NetworkStatsHandler* pHandler);

private:
	STATS stats_;

	std::vector<NetworkStatsHandler*> handlers_;
};

}
}
#endif // KBE_NETWORK_STATS_H
