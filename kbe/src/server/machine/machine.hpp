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

#ifndef KBE_MACHINE_HPP
#define KBE_MACHINE_HPP
	
// common include	
#include "server/kbemain.hpp"
#include "server/serverapp.hpp"
#include "server/idallocate.hpp"
#include "server/serverconfig.hpp"
#include "server/componentbridge.hpp"
#include "cstdkbe/timer.hpp"
#include "network/endpoint.hpp"
#include "network/udp_packet_receiver.hpp"
#include "network/common.hpp"

//#define NDEBUG
#include <map>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class Machine:	public ServerApp, 
				public Singleton<Machine>
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK = TIMEOUT_SERVERAPP_MAX + 1
	};
	
	Machine(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Machine();
	
	bool run();
	
	bool findBroadcastInterface();

	/** 网络接口
		某个app广播了自己的地址
	*/
	void onBroadcastInterface(Mercury::Channel* pChannel, int32 uid, std::string& username, 
							int8 componentType, uint64 componentID, uint64 componentIDEx, 
							int8 globalorderid, int8 grouporderid,
							uint32 intaddr, uint16 intport,
							uint32 extaddr, uint16 extport, std::string& extaddrEx, uint32 pid,
							float cpu, float mem, uint32 usedmem, int8 state, uint32 machineID, uint64 extradata,
							uint64 extradata1, uint64 extradata2, uint64 extradata3);
	
	/** 网络接口
		某个app寻找另一个app的地址
	*/
	void onFindInterfaceAddr(Mercury::Channel* pChannel, int32 uid, std::string& username, 
		int8 componentType, uint64 componentID, int8 findComponentType, uint32 finderAddr, uint16 finderRecvPort);

	/** 网络接口
		查询所有接口信息
	*/
	void onQueryAllInterfaceInfos(Mercury::Channel* pChannel, int32 uid, std::string& username, 
		uint16 finderRecvPort);

	void handleTimeout(TimerHandle handle, void * arg);

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	bool initNetwork();

	/** 网络接口
		启动服务器
		@uid: 提供启动的uid参数
		@components: 启动哪些组件(可能采取分布式启动方案)
	*/
	void startserver(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		关闭服务器
		@uid: 提供启动的uid参数
	*/
	void stopserver(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/**
		对本机运行的组件进行检查是否可用
	*/
	bool checkComponentUsable(const Components::ComponentInfos* info);
protected:
	// udp广播地址
	u_int32_t broadcastAddr_;
	Mercury::EndPoint ep_;
	Mercury::EndPoint epBroadcast_;

	Mercury::EndPoint epLocal_;

	Mercury::UDPPacketReceiver* pEPPacketReceiver_;
	Mercury::UDPPacketReceiver* pEBPacketReceiver_;
	Mercury::UDPPacketReceiver* pEPLocalPacketReceiver_;

	// 本机使用的uid
	std::vector<int32>			localuids_;
};

}

#endif // KBE_MACHINE_HPP
