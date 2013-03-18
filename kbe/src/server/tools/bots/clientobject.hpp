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

#ifndef __CLIENT_OBJECT_H__
#define __CLIENT_OBJECT_H__

#include "client_lib/entity.hpp"
#include "client_lib/clientobjectbase.hpp"
#include "network/tcp_packet_receiver.hpp"
#include "network/encryption_filter.hpp"
#include "pyscript/pyobject_pointer.hpp"

namespace KBEngine{ 

/*
*/

class ClientObject : public ClientObjectBase, Mercury::TCPPacketReceiver
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
		C_ERROR_LOGIN_GATEWAY_FAILED = 4,
	};

	enum C_STATE
	{
		C_STATE_INIT = 0,
		C_STATE_CREATE = 1,
		C_STATE_LOGIN = 2,
		C_STATE_LOGIN_GATEWAY_CREATE = 3,
		C_STATE_LOGIN_GATEWAY = 4,
		C_STATE_PLAY = 5,
	};

	ClientObject(std::string name, Mercury::NetworkInterface& ninterface);
	virtual ~ClientObject();

	bool processSocket(bool expectingPacket);

	bool initCreate();
	bool initLoginGateWay();

	void gameTick();

	ClientObject::C_ERROR lasterror(){ return error_; }

	virtual void onHelloCB_(Mercury::Channel* pChannel, const std::string& verInfo, 
		COMPONENT_TYPE componentType);

	/** 网络接口
		创建账号成功和失败回调
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_ACCOUNT_CREATE:创建失败（已经存在）, 
									MERCURY_SUCCESS:账号创建成功

									SERVER_ERROR_CODE failedcode;
		@二进制附带数据:二进制额外数据: uint32长度 + bytearray
	*/
	virtual void onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s);

	/** 网络接口
	   登录失败回调
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_SRV_OVERLOAD:服务器负载过重, 
									MERCURY_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	virtual void onLoginFailed(Mercury::Channel * pChannel, MemoryStream& s);

	/** 网络接口
	   登录成功
	   @ip: 服务器ip地址
	   @port: 服务器端口
	*/
	virtual void onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s);
protected:
	C_ERROR error_;
	C_STATE state_;
	Mercury::BlowfishFilter* pBlowfishFilter_;
};


}
#endif
