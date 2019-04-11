// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_MACHINE_H
#define KBE_MACHINE_H
	
// common include	
#include "server/kbemain.h"
#include "server/serverapp.h"
#include "server/idallocate.h"
#include "server/serverconfig.h"
#include "server/components.h"
#include "common/timer.h"
#include "network/endpoint.h"
#include "network/udp_packet_receiver.h"
#include "network/common.h"

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
	
	Machine(Network::EventDispatcher& dispatcher, 
		Network::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Machine();
	
	bool run();
	
	bool findBroadcastInterface();

	/** 网络接口
		某个app广播了自己的地址
	*/
	void onBroadcastInterface(Network::Channel* pChannel, int32 uid, std::string& username, 
							COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ID componentIDEx, 
							COMPONENT_ORDER globalorderid, COMPONENT_ORDER grouporderid, COMPONENT_GUS gus,
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx, uint32 pid,
							float cpu, float mem, uint32 usedmem, int8 state, uint32 machineID, uint64 extradata,
							uint64 extradata1, uint64 extradata2, uint64 extradata3, uint32 backRecvAddr, uint16 backRecvPort);
	
	/** 网络接口
		某个app寻找另一个app的地址
	*/
	void onFindInterfaceAddr(Network::Channel* pChannel, int32 uid, std::string& username, 
		COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_TYPE findComponentType, uint32 finderAddr, uint16 finderRecvPort);

	/** 网络接口
		查询所有接口信息
	*/
	void onQueryAllInterfaceInfos(Network::Channel* pChannel, int32 uid, std::string& username, 
		uint16 finderRecvPort);

	/** 网络接口
	查询所有machine进程
	*/
	void onQueryMachines(Network::Channel* pChannel, int32 uid, std::string& username,
		uint16 finderRecvPort);

	void queryComponentID(Network::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID,
		int32 uid, uint16 finderRecvPort, int macMD5, int32 pid);

	void removeComponentID(COMPONENT_TYPE componentType, COMPONENT_ID componentID, int32 uid);

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
	void startserver(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 信号处理
	*/
	virtual bool installSignals();
	virtual void onSignalled(int sigNum);

#if KBE_PLATFORM != PLATFORM_WIN32
	/**
	* 在linux下启动一个新进程
	*/
	uint16 startLinuxProcess(int32 uid, COMPONENT_TYPE componentType, uint64 cid, uint16 gus, 
		std::string& KBE_ROOT, std::string& KBE_RES_PATH, std::string& KBE_BIN_PATH);
#else
	/**
	* 在windows下启动一个新进程
	*/
	DWORD startWindowsProcess(int32 uid, COMPONENT_TYPE componentType, uint64 cid, uint16 gus, 
		std::string& KBE_ROOT, std::string& KBE_RES_PATH, std::string& KBE_BIN_PATH);
#endif

	/** 网络接口
		关闭服务器
		@uid: 提供启动的uid参数
	*/
	void stopserver(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
	杀死服务器
	@uid: 提供启动的uid参数
	*/
	void killserver(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/**
		对本机运行的组件进行检查是否可用
	*/
	bool checkComponentUsable(const Components::ComponentInfos* info, bool getdatas, bool autoerase);

protected:
	// udp广播地址
	u_int32_t					broadcastAddr_;
	Network::EndPoint			ep_;
	Network::EndPoint			epBroadcast_;

	Network::EndPoint			epLocal_;

	Network::UDPPacketReceiver* pEPPacketReceiver_;
	Network::UDPPacketReceiver* pEBPacketReceiver_;
	Network::UDPPacketReceiver* pEPLocalPacketReceiver_;

	// 本机使用的uid
	std::vector<int32>			localuids_;

	typedef std::vector<COMPONENT_ID> ID_LOGS;
	typedef std::map<COMPONENT_TYPE, ID_LOGS> CID_MAP;

	std::map<int32, CID_MAP>		cidMap_;
	std::map<std::string, COMPONENT_ID>		pidMD5Map_;
};

}

#endif // KBE_MACHINE_H
