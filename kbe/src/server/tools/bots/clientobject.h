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

#ifndef KBE_CLIENT_OBJECT_H
#define KBE_CLIENT_OBJECT_H

#include "tcp_packet_receiver_ex.h"
#include "tcp_packet_sender_ex.h"
#include "client_lib/entity.h"
#include "client_lib/clientobjectbase.h"
#include "network/encryption_filter.h"
#include "pyscript/pyobject_pointer.h"

namespace KBEngine{ 

/*
*/

class ClientObject : public ClientObjectBase
{
	/** 
		���໯ ��һЩpy�������������� 
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

	bool initCreate();
	bool initLoginBaseapp();

	void gameTick();

	ClientObject::C_ERROR lasterror(){ return error_; }

	bool isDestroyed(){ return state_ == C_STATE_DESTROYED; }
	void destroy(){ state_ = C_STATE_DESTROYED; }

	virtual void onHelloCB_(Network::Channel* pChannel, const std::string& verInfo,
		const std::string& scriptVerInfo, const std::string& protocolMD5, 
		const std::string& entityDefMD5, COMPONENT_TYPE componentType);

	/** ����ӿ�
		�����˺ųɹ���ʧ�ܻص�
	   @failedcode: ʧ�ܷ����� NETWORK_ERR_SRV_NO_READY:������û��׼����, 
									NETWORK_ERR_ACCOUNT_CREATE:����ʧ�ܣ��Ѿ����ڣ�, 
									NETWORK_SUCCESS:�˺Ŵ����ɹ�

									SERVER_ERROR_CODE failedcode;
		@�����Ƹ�������:�����ƶ�������: uint32���� + bytearray
	*/
	virtual void onCreateAccountResult(Network::Channel * pChannel, MemoryStream& s);

	/** ����ӿ�
	   ��¼ʧ�ܻص�
	   @failedcode: ʧ�ܷ����� NETWORK_ERR_SRV_NO_READY:������û��׼����, 
									NETWORK_ERR_SRV_OVERLOAD:���������ع���, 
									NETWORK_ERR_NAME_PASSWORD:�û����������벻��ȷ
	*/
	virtual void onLoginFailed(Network::Channel * pChannel, MemoryStream& s);

	/** ����ӿ�
	   ��¼�ɹ�
	   @ip: ������ip��ַ
	   @port: �������˿�
	*/
	virtual void onLoginSuccessfully(Network::Channel * pChannel, MemoryStream& s);

protected:
	C_ERROR error_;
	C_STATE state_;
	Network::BlowfishFilter* pBlowfishFilter_;

	Network::TCPPacketSenderEx* pTCPPacketSenderEx_;
	Network::TCPPacketReceiverEx* pTCPPacketReceiverEx_;
};


}

#endif // KBE_CLIENT_OBJECT_H
