/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __ENTITY_CELL_BASE_CLIENT__MAILBOX_H__
#define __ENTITY_CELL_BASE_CLIENT__MAILBOX_H__
	
// common include
#include "cstdkbe/cstdkbe.hpp"
#include "entitydef/entitymailboxabstract.hpp"
#include "pyscript/scriptobject.hpp"
#include "pyscript/pickler.hpp"
#include "entitydef/method.hpp"
#include "entitydef/entitydef.hpp"
	
#ifdef KBE_SERVER
#include "server/engine_componentmgr.hpp"
#endif
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{
	
typedef std::tr1::function<PyObject* (COMPONENT_ID componentID, ENTITY_ID& eid)> GetEntityFunc;

namespace Mercury
{
class Channel;
}

class EntityMailbox : public EntityMailboxAbstract
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(EntityMailbox, EntityMailboxAbstract)
private:
	static GetEntityFunc					__getEntityFunc;		// 获得一个entity的实体的函数地址
protected:
	ScriptModule*							m_scriptModule_;		// 该entity所使用的脚本模块对象
public:
	EntityMailbox(Mercury::Channel* pChannel, ScriptModule* scriptModule, COMPONENT_ID componentID, ENTITY_ID& eid, ENTITY_MAILBOX_TYPE type);
	~EntityMailbox();
	
	/** 脚本请求获取属性或者方法 */
	PyObject* onScriptGetAttribute(PyObject* attr);						
			
	/** 获得对象的描述 */
	PyObject* tp_repr();
	PyObject* tp_str();

	/** unpickle方法 */
	static PyObject* __unpickle__(PyObject* self, PyObject* args);

	/** 脚本被安装时被调用 */
	static void onInstallScript(PyObject* mod);

	/** 设置mailbox的__getEntityFunc函数地址 */
	static void setGetEntityFunc(GetEntityFunc func){ __getEntityFunc = func; };
};

}
#endif
