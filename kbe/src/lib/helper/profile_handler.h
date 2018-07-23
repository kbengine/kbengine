// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_PROFILE_HANDLER_H
#define KBE_PROFILE_HANDLER_H

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

class ProfileHandler : public TimerHandler
{
public:
	ProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Network::Address& addr);
	virtual ~ProfileHandler();
	
	virtual void timeout() = 0;
	virtual void sendStream(MemoryStream* s) = 0;

	static KBEUnordered_map<std::string, KBEShared_ptr< ProfileHandler > > profiles;

protected:
	virtual void handleTimeout(TimerHandle handle, void * arg);

	Network::NetworkInterface& networkInterface_;

	TimerHandle reportLimitTimerHandle_;
	
	std::string name_;
	
	Network::Address addr_;

	uint32 timinglen_;
};

class CProfileHandler : public Task, 
						public ProfileHandler
{
public:
	CProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Network::Address& addr);
	virtual ~CProfileHandler();
	
	void timeout();
	void sendStream(MemoryStream* s);
	bool process();

private:
	struct ProfileVal
	{
		// 名称
		std::string		name;

		// startd后的时间.
		TimeStamp		lastTime;
		TimeStamp		diff_lastTime;

		// count_次的总时间
		TimeStamp		sumTime;
		TimeStamp		diff_sumTime;

		// 记录最后一次内部时间片
		TimeStamp		lastIntTime;
		TimeStamp		diff_lastIntTime;

		// count_次内部总时间
		TimeStamp		sumIntTime;
		TimeStamp		diff_sumIntTime;

		uint32			count;
		uint32			diff_count;
	};

	// 此ProfileVal只在计时器开始时记录default.profiles的初始值
	// 在结束时取出差值得到结果
	typedef KBEUnordered_map<std::string,  ProfileVal> PROFILEVALS;
	PROFILEVALS profileVals_;
};

class EventProfileHandler : public ProfileHandler
{
public:
	EventProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Network::Address& addr);
	virtual ~EventProfileHandler();
	
	void timeout();
	void sendStream(MemoryStream* s);

	void onTriggerEvent(const EventHistoryStats& eventHistory, const EventHistoryStats::Stats& stats, 
		uint32 size);

	static void triggerEvent(const EventHistoryStats& eventHistory, const EventHistoryStats::Stats& stats, 
		uint32 size);

private:
	struct ProfileVal
	{
		ProfileVal()
		{
			name = "";
			size = 0;
			count = 0;
		}

		// 名称
		std::string		name;

		uint32			size;
		uint32			count;
	};

	// 此ProfileVal只在计时器开始时记录default.profiles的初始值
	// 在结束时取出差值得到结果
	typedef KBEUnordered_map<std::string,  ProfileVal> PROFILEVALS;

	typedef KBEUnordered_map< std::string,  PROFILEVALS > PROFILEVALMAP;
	PROFILEVALMAP profileMaps_;
	
	static std::vector<EventProfileHandler*> eventProfileHandlers_;
	int removeHandle_;
};

class NetworkProfileHandler : public ProfileHandler, public Network::NetworkStatsHandler
{
public:
	NetworkProfileHandler(Network::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Network::Address& addr);
	virtual ~NetworkProfileHandler();
	
	void timeout();
	void sendStream(MemoryStream* s);

	virtual void onSendMessage(const Network::MessageHandler& msgHandler, int size);
	virtual void onRecvMessage(const Network::MessageHandler& msgHandler, int size);

private:
	struct ProfileVal
	{
		ProfileVal()
		{
			name = "";
			send_size = 0;
			send_count = 0;
			send_avgsize = 0;
			total_send_size = 0;
			total_send_count = 0;

			recv_size = 0;
			recv_count = 0;
			recv_avgsize = 0;
			total_recv_size = 0;
			total_recv_count = 0;
		}

		// 名称
		std::string		name;

		uint32			send_size;
		uint32			send_avgsize;
		uint32			send_count;

		uint32			total_send_size;
		uint32			total_send_count;

		uint32			recv_size;
		uint32			recv_count;
		uint32			recv_avgsize;

		uint32			total_recv_size;
		uint32			total_recv_count;
	};

	typedef KBEUnordered_map<std::string,  ProfileVal> PROFILEVALS;
	PROFILEVALS profileVals_;
};

}

#endif
