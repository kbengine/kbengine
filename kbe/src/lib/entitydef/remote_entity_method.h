/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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


#ifndef KBENGINE_REMOTE_ENTITY_METHOD_H
#define KBENGINE_REMOTE_ENTITY_METHOD_H

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif
#include "datatype.h"
#include "datatypes.h"
#include "helper/debug_helper.h"
#include "network/packet.h"
#include "entitycallabstract.h"
#include "pyscript/scriptobject.h"	


namespace KBEngine{

class MethodDescription;

class RemoteEntityMethod : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(RemoteEntityMethod, script::ScriptObject)	
		
public:	
	RemoteEntityMethod(MethodDescription* methodDescription, 
						EntityCallAbstract* entitycall, PyTypeObject* pyType = NULL);
	
	virtual ~RemoteEntityMethod();

	const char* getName(void) const;

	MethodDescription* getDescription(void) const
	{ 
		return methodDescription_; 
	}

	static PyObject* tp_call(PyObject* self, 
			PyObject* args, PyObject* kwds);

	EntityCallAbstract* getEntityCall(void) const
	{
		return pEntityCall_;
	}
	
protected:	
	MethodDescription*		methodDescription_;					// 这个方法的描述
	EntityCallAbstract*		pEntityCall_;						// 这个方法所属的entitycall
};
}

#endif // KBENGINE_REMOTE_ENTITY_METHOD_H
