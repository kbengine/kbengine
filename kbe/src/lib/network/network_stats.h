// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
