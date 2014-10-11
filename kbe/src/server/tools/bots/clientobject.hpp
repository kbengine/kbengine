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

#ifndef KBE_CLIENT_OBJECT_HPP
#define KBE_CLIENT_OBJECT_HPP

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
	
	void reset(void);

	bool initCreate();
	bool initLoginGateWay();

	void gameTick();

	ClientObject::C_ERROR lasterror(){ return error_; }

	virtual void onHelloCB_(Mercury::Channel* pChannel, const std::string& verInfo,
		const std::string& scriptVerInfo, const std::string& protocolMD5, 
		const std::string& entityDefMD5, COMPONENT_TYPE componentType);

	/** ����ӿ�
		�����˺ųɹ���ʧ�ܻص�
	   @failedcode: ʧ�ܷ����� MERCURY_ERR_SRV_NO_READY:������û��׼����, 
									MERCURY_ERR_ACCOUNT_CREATE:����ʧ�ܣ��Ѿ����ڣ�, 
									MERCURY_SUCCESS:�˺Ŵ����ɹ�

									SERVER_ERROR_CODE failedcode;
		@�����Ƹ�������:�����ƶ�������: uint32���� + bytearray
	*/
	virtual void onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s);

	/** ����ӿ�
	   ��¼ʧ�ܻص�
	   @failedcode: ʧ�ܷ����� MERCURY_ERR_SRV_NO_READY:������û��׼����, 
									MERCURY_ERR_SRV_OVERLOAD:���������ع���, 
									MERCURY_ERR_NAME_PASSWORD:�û����������벻��ȷ
	*/
	virtual void onLoginFailed(Mercury::Channel * pChannel, MemoryStream& s);

	/** ����ӿ�
	   ��¼�ɹ�
	   @ip: ������ip��ַ
	   @port: �������˿�
	*/
	virtual void onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s);

protected:
	C_ERROR error_;
	C_STATE state_;
	Mercury::BlowfishFilter* pBlowfishFilter_;
};


}

#endif // KBE_CLIENT_OBJECT_HPP
