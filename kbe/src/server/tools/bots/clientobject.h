// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CLIENT_OBJECT_H
#define KBE_CLIENT_OBJECT_H

#include "tcp_packet_receiver_ex.h"
#include "tcp_packet_sender_ex.h"
#include "kcp_packet_receiver_ex.h"
#include "kcp_packet_sender_ex.h"
#include "client_lib/entity.h"
#include "client_lib/clientobjectbase.h"
#include "network/encryption_filter.h"
#include "pyscript/pyobject_pointer.h"

namespace KBEngine { 

/*
*/

class ClientObject : public ClientObjectBase
{
	/** 
		子类化 将一些py操作填充进派生类 
	*/
	INSTANCE_SCRIPT_HREADER(ClientObject, ClientObjectBase)	

public:
	enum C_ERROR
	{
		C_ERROR_NONE = 0,
		C_ERROR_INIT_NETWORK_FAILED = 1,
		C_ERROR_CREATE_FAILED = 2,
		C_ERROR_LOGIN_FAILED = 3,
		C_ERROR_LOGIN_BASEAPP_FAILED = 4,
	};

	enum C_STATE
	{
		C_STATE_INIT = 0,
		C_STATE_CREATE = 1,
		C_STATE_LOGIN = 2,
		C_STATE_LOGIN_BASEAPP_CREATE = 3,
		C_STATE_LOGIN_BASEAPP = 4,
		C_STATE_PLAY = 5,
		C_STATE_DESTROYED = 6,
	};

	ClientObject(std::string name, Network::NetworkInterface& ninterface);
	virtual ~ClientObject();
	
	virtual void finalise();
	virtual void reset(void);

	void clearStates(void);

	bool initCreate();
	bool initLoginBaseapp();

	void gameTick();

	ClientObject::C_ERROR lasterror() { return error_; }

	bool isDestroyed() { return state_ == C_STATE_DESTROYED; }
	void destroy() { state_ = C_STATE_DESTROYED; }

	virtual void onHelloCB_(Network::Channel* pChannel, const std::string& verInfo,
		const std::string& scriptVerInfo, const std::string& protocolMD5, 
		const std::string& entityDefMD5, COMPONENT_TYPE componentType);

	/** 网络接口
		创建账号成功和失败回调
	   @failedcode: 失败返回码 NETWORK_ERR_SRV_NO_READY:服务器没有准备好, 
									NETWORK_ERR_ACCOUNT_CREATE:创建失败（已经存在）, 
									NETWORK_SUCCESS:账号创建成功

									SERVER_ERROR_CODE failedcode;
		@二进制附带数据:二进制额外数据: uint32长度 + bytearray
	*/
	virtual void onCreateAccountResult(Network::Channel * pChannel, MemoryStream& s);

	/** 网络接口
	   登录失败回调
	   @failedcode: 失败返回码 NETWORK_ERR_SRV_NO_READY:服务器没有准备好, 
									NETWORK_ERR_SRV_OVERLOAD:服务器负载过重, 
									NETWORK_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	virtual void onLoginFailed(Network::Channel * pChannel, MemoryStream& s);

	/** 网络接口
	   登录成功
	   @ip: 服务器ip地址
	   @port: 服务器端口
	*/
	virtual void onLoginSuccessfully(Network::Channel * pChannel, MemoryStream& s);

	virtual void onLoginBaseappFailed(Network::Channel * pChannel, SERVER_ERROR_CODE failedcode);

	virtual void onLogin(Network::Bundle* pBundle);

protected:
	C_ERROR error_;
	C_STATE state_;
	Network::BlowfishFilter* pBlowfishFilter_;

	Network::TCPPacketSenderEx* pTCPPacketSenderEx_;
	Network::TCPPacketReceiverEx* pTCPPacketReceiverEx_;

	Network::KCPPacketSenderEx* pKCPPacketSenderEx_;
	Network::KCPPacketReceiverEx* pKCPPacketReceiverEx_;
};


}

#endif // KBE_CLIENT_OBJECT_H
