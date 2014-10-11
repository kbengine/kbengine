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


#ifndef KBE_LOGINAPP_HPP
#define KBE_LOGINAPP_HPP
	
// common include	
#include "server/kbemain.hpp"
#include "server/serverapp.hpp"
#include "server/idallocate.hpp"
#include "server/serverconfig.hpp"
#include "server/pendingLoginmgr.hpp"
#include "cstdkbe/timer.hpp"
#include "network/endpoint.hpp"
	
namespace KBEngine{

class HTTPCBHandler;

class Loginapp :	public ServerApp, 
					public Singleton<Loginapp>
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK = TIMEOUT_SERVERAPP_MAX + 1,
		TIMEOUT_CHECK_STATUS,
		TIMEOUT_MAX
	};

	Loginapp(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Loginapp();
	
	bool run();
	
	virtual void onChannelDeregister(Mercury::Channel * pChannel);

	virtual void handleTimeout(TimerHandle handle, void * arg);
	void handleCheckStatusTick();

	/* ��ʼ����ؽӿ� */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	
	virtual void onShutdownBegin();

	virtual void onHello(Mercury::Channel* pChannel, 
		const std::string& verInfo, 
		const std::string& scriptVerInfo, 
		const std::string& encryptedKey);

	/** ����ӿ�
		ĳ��client��app��֪���ڻ״̬��
	*/
	void onClientActiveTick(Mercury::Channel* pChannel);

	/** ����ӿ�
		�����˺�
	*/
	bool _createAccount(Mercury::Channel* pChannel, std::string& accountName, 
		std::string& password, std::string& datas, ACCOUNT_TYPE type = ACCOUNT_TYPE_NORMAL);
	void reqCreateAccount(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		����email�˺�
	*/
	void reqCreateMailAccount(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		�����˺�
	*/
	void onReqCreateAccountResult(Mercury::Channel* pChannel, MemoryStream& s);
	void onReqCreateMailAccountResult(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		�����˺��������루��������?��
	*/
	void reqAccountResetPassword(Mercury::Channel* pChannel, std::string& accountName);
	void onReqAccountResetPasswordCB(Mercury::Channel* pChannel, std::string& accountName, std::string& email,
		SERVER_ERROR_CODE failedcode, std::string& code);

	/** ����ӿ�
		dbmgr�˺ż����
	*/
	void onAccountActivated(Mercury::Channel* pChannel, std::string& code, bool success);

	/** ����ӿ�
		dbmgr�˺Ű�email����
	*/
	void onAccountBindedEmail(Mercury::Channel* pChannel, std::string& code, bool success);

	/** ����ӿ�
		dbmgr�˺��������뷵��
	*/
	void onAccountResetPassword(Mercury::Channel* pChannel, std::string& code, bool success);

	/** ����ӿ�
		�û���¼������
		clientType[COMPONENT_CLIENT_TYPE]: ǰ�����(�ֻ��� web�� pcexe��)
		clientData[str]: ǰ�˸�������(����������ģ� ���總���ֻ��ͺţ� ��������͵�)
		accountName[str]: �ʺ���
		password[str]: ����
	*/
	void login(Mercury::Channel* pChannel, MemoryStream& s);

	/*
		��¼ʧ��
		failedcode: ʧ�ܷ����� MERCURY_ERR_SRV_NO_READY:������û��׼����, 
									MERCURY_ERR_SRV_OVERLOAD:���������ع���, 
									MERCURY_ERR_NAME_PASSWORD:�û����������벻��ȷ
	*/
	void _loginFailed(Mercury::Channel* pChannel, std::string& loginName, SERVER_ERROR_CODE failedcode, std::string& datas, bool force = false);
	
	/** ����ӿ�
		dbmgr���صĵ�¼�˺ż����
	*/
	void onLoginAccountQueryResultFromDbmgr(Mercury::Channel* pChannel, MemoryStream& s);

	/** ����ӿ�
		baseappmgr���صĵ�¼���ص�ַ
	*/
	void onLoginAccountQueryBaseappAddrFromBaseappmgr(Mercury::Channel* pChannel, std::string& loginName, 
		std::string& accountName, std::string& addr, uint16 port);


	/** ����ӿ�
		dbmgr���ͳ�ʼ��Ϣ
		startGlobalOrder: ȫ������˳�� �������ֲ�ͬ���
		startGroupOrder: ��������˳�� ����������baseapp�еڼ���������
	*/
	void onDbmgrInitCompleted(Mercury::Channel* pChannel, int32 startGlobalOrder, int32 startGroupOrder, const std::string& digest);

	/** ����ӿ�
		�ͻ���Э�鵼��
	*/
	void importClientMessages(Mercury::Channel* pChannel);

	/** ����ӿ�
		��������������
	*/
	void importServerErrorsDescr(Mercury::Channel* pChannel);

	// ����汾��ƥ��
	virtual void onVersionNotMatch(Mercury::Channel* pChannel);

	// ����ű���汾��ƥ��
	virtual void onScriptVersionNotMatch(Mercury::Channel* pChannel);

	/** ����ӿ�
		baseappͬ���Լ��ĳ�ʼ����Ϣ
		startGlobalOrder: ȫ������˳�� �������ֲ�ͬ���
		startGroupOrder: ��������˳�� ����������baseapp�еڼ���������
	*/
	void onBaseappInitProgress(Mercury::Channel* pChannel, float progress);
protected:
	TimerHandle							loopCheckTimerHandle_;

	// ��¼ע���˺Ż�δ��½������
	PendingLoginMgr						pendingCreateMgr_;

	// ��¼��¼������������δ������ϵ��˺�
	PendingLoginMgr						pendingLoginMgr_;

	std::string							digest_;

	HTTPCBHandler*						pHttpCBHandler;

	float								initProgress_;
};

}

#endif // KBE_LOGINAPP_HPP
