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


#ifndef KBE_ENTITY_CELL_BASE_CLIENT__MAILBOX_HPP
#define KBE_ENTITY_CELL_BASE_CLIENT__MAILBOX_HPP
	
// common include
#include "cstdkbe/cstdkbe.hpp"
#include "entitydef/entitymailboxabstract.hpp"
#include "pyscript/scriptobject.hpp"

	
#ifdef KBE_SERVER
#include "server/components.hpp"
#endif
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

namespace Mercury
{
class Channel;
}

class ScriptDefModule;
class RemoteEntityMethod;
class MethodDescription;

class EntityMailbox : public EntityMailboxAbstract
{
	/** ���໯ ��һЩpy�������������� */
	INSTANCE_SCRIPT_HREADER(EntityMailbox, EntityMailboxAbstract)
public:
	typedef std::tr1::function<RemoteEntityMethod* (MethodDescription* md, EntityMailbox* pMailbox)> MailboxCallHookFunc;
	typedef std::tr1::function<PyObject* (COMPONENT_ID componentID, ENTITY_ID& eid)> GetEntityFunc;
	typedef std::tr1::function<Mercury::Channel* (EntityMailbox&)> FindChannelFunc;

	EntityMailbox(ScriptDefModule* scriptModule, const Mercury::Address* pAddr, COMPONENT_ID componentID, 
		ENTITY_ID eid, ENTITY_MAILBOX_TYPE type);

	~EntityMailbox();
	
	/** 
		�ű������ȡ���Ի��߷��� 
	*/
	PyObject* onScriptGetAttribute(PyObject* attr);						
			
	/** 
		��ö�������� 
	*/
	PyObject* tp_repr();
	PyObject* tp_str();
	
	void c_str(char* s, size_t size);

	/** 
		unpickle���� 
	*/
	static PyObject* __unpickle__(PyObject* self, PyObject* args);

	/** 
		�ű�����װʱ������ 
	*/
	static void onInstallScript(PyObject* mod);

	/** 
		����mailbox��__getEntityFunc������ַ 
	*/
	static void setGetEntityFunc(GetEntityFunc func){ 
		__getEntityFunc = func; 
	};

	/** 
		����mailbox��__getEntityFunc������ַ 
	*/
	static void setFindChannelFunc(FindChannelFunc func){ 
		__findChannelFunc = func; 
	};

	/** 
		����mailbox��__hookCallFunc������ַ 
	*/
	static void setMailboxCallHookFunc(MailboxCallHookFunc* pFunc){ 
		__hookCallFuncPtr = pFunc; 
	};

	virtual RemoteEntityMethod* createRemoteMethod(MethodDescription* md);

	virtual Mercury::Channel* getChannel(void);

	void reload();

	typedef std::vector<EntityMailbox*> MAILBOXS;
	static MAILBOXS mailboxs;
private:
	static GetEntityFunc					__getEntityFunc;		// ���һ��entity��ʵ��ĺ�����ַ
	static MailboxCallHookFunc*				__hookCallFuncPtr;
	static FindChannelFunc					__findChannelFunc;
protected:
	std::string								scriptModuleName_;
	ScriptDefModule*						scriptModule_;			// ��entity��ʹ�õĽű�ģ�����

	void _setATIdx(MAILBOXS::size_type idx){ atIdx_ = idx; }
	MAILBOXS::size_type	atIdx_;
};

}
#endif // KBE_ENTITY_CELL_BASE_CLIENT__MAILBOX_HPP
