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

	/** ����ӿ�
		ĳ��app�㲥���Լ��ĵ�ַ
	*/
	void onBroadcastInterface(Network::Channel* pChannel, int32 uid, std::string& username, 
							COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ID componentIDEx, 
							COMPONENT_ORDER globalorderid, COMPONENT_ORDER grouporderid, COMPONENT_GUS gus,
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx, uint32 pid,
							float cpu, float mem, uint32 usedmem, int8 state, uint32 machineID, uint64 extradata,
							uint64 extradata1, uint64 extradata2, uint64 extradata3, uint32 backRecvAddr, uint16 backRecvPort);
	
	/** ����ӿ�
		ĳ��appѰ����һ��app�ĵ�ַ
	*/
	void onFindInterfaceAddr(Network::Channel* pChannel, int32 uid, std::string& username, 
		COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_TYPE findComponentType, uint32 finderAddr, uint16 finderRecvPort);

	/** ����ӿ�
		��ѯ���нӿ���Ϣ
	*/
	void onQueryAllInterfaceInfos(Network::Channel* pChannel, int32 uid, std::string& username, 
		uint16 finderRecvPort);

	/** ����ӿ�
	��ѯ����machine����
	*/
	void onQueryMachines(Network::Channel* pChannel, int32 uid, std::string& username,
		uint16 finderRecvPort);

	void handleTimeout(TimerHandle handle, void * arg);

	/* ��ʼ����ؽӿ� */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	bool initNetwork();

	/** ����ӿ�
		����������
		@uid: �ṩ������uid����
		@components: ������Щ���(���ܲ�ȡ�ֲ�ʽ��������)
	*/
	void startserver(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** �źŴ���
	*/
	virtual bool installSignals();
	virtual void onSignalled(int sigNum);

#if KBE_PLATFORM != PLATFORM_WIN32
	/**
	* ��linux������һ���½���
	*/
	uint16 startLinuxProcess(int32 uid, COMPONENT_TYPE componentType, uint64 cid, uint16 gus, 
		std::string& KBE_ROOT, std::string& KBE_RES_PATH, std::string& KBE_BIN_PATH);
#else
	/**
	* ��windows������һ���½���
	*/
	DWORD startWindowsProcess(int32 uid, COMPONENT_TYPE componentType, uint64 cid, uint16 gus, 
		std::string& KBE_ROOT, std::string& KBE_RES_PATH, std::string& KBE_BIN_PATH);
#endif

	/** ����ӿ�
		�رշ�����
		@uid: �ṩ������uid����
	*/
	void stopserver(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
	ɱ��������
	@uid: �ṩ������uid����
	*/
	void killserver(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/**
		�Ա������е�������м���Ƿ����
	*/
	bool checkComponentUsable(const Components::ComponentInfos* info, bool getdatas, bool autoerase);

protected:
	// udp�㲥��ַ
	u_int32_t					broadcastAddr_;
	Network::EndPoint			ep_;
	Network::EndPoint			epBroadcast_;

	Network::EndPoint			epLocal_;

	Network::UDPPacketReceiver* pEPPacketReceiver_;
	Network::UDPPacketReceiver* pEBPacketReceiver_;
	Network::UDPPacketReceiver* pEPLocalPacketReceiver_;

	// ����ʹ�õ�uid
	std::vector<int32>			localuids_;
};

}

#endif // KBE_MACHINE_H
