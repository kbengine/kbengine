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
#include "server/components.hpp"
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
public:
	EntityMailbox(ScriptModule* scriptModule, const Mercury::Address* pAddr, COMPONENT_ID componentID, 
		ENTITY_ID eid, ENTITY_MAILBOX_TYPE type);
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
private:
	static GetEntityFunc					__getEntityFunc;		// 获得一个entity的实体的函数地址
protected:
	ScriptModule*							scriptModule_;			// 该entity所使用的脚本模块对象
};

}
#endif
