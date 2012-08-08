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

#include "method.hpp"
#include "network/bundle.hpp"

namespace KBEngine{

uint32	MethodDescription::methodDescriptionCount_ = 0;


SCRIPT_METHOD_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(RemoteEntityMethod, tp_call, 0, 0, 0, 0)	
	
//-------------------------------------------------------------------------------------
MethodDescription::MethodDescription(ENTITY_METHOD_UID utype, std::string name, bool isExposed):
name_(name),
utype_(utype),
argTypes_(),
isExposed_(isExposed)
{
	MethodDescription::methodDescriptionCount_++;
	utype_ = MethodDescription::methodDescriptionCount_;
}

//-------------------------------------------------------------------------------------
MethodDescription::~MethodDescription()
{
	std::vector<DataType*>::iterator iter = argTypes_.begin();
	for(; iter != argTypes_.end(); iter++)
		(*iter)->decRef();
	argTypes_.clear();
}

//-------------------------------------------------------------------------------------
bool MethodDescription::pushArgType(DataType* dataType)
{
	if(dataType == NULL)
	{
		ERROR_MSG("MethodDescription::pushArgType: dataType is NULL!\n");
		return false;
	}

	dataType->incRef();
	argTypes_.push_back(dataType);
	return true;
}

//-------------------------------------------------------------------------------------
bool MethodDescription::checkArgs(PyObject* args)
{
	if (!PyTuple_Check(args))
	{
		PyErr_Format(PyExc_SystemError, "Method::checkArgs: method[%s] args is not a tuple.\n", getName().c_str());
		PyErr_Print();
		return false;
	}
	
	int offset = (isExposed() == true) ? 1 : 0;
	uint8 argsSize = argTypes_.size();
	uint8 giveArgsSize = PyTuple_Size(args);

	if (giveArgsSize != argsSize + offset)
	{
		PyErr_Format(PyExc_TypeError, "Method::checkArgs: method[%s] requires exactly %d argument%s; %d given", 
				getName().c_str(),
				argsSize,
				(argsSize == 1) ? "" : "s",
				PyTuple_Size(args));

		PyErr_Print();
		return false;
	}	
	
	
	// 检查是否是一个exposed方法
	if(offset > 0)
	{
		PyObject* pyExposed = PyTuple_GetItem(args, 0);
		if (!PyLong_Check(pyExposed))
		{
			PyObject* pyeid = PyObject_GetAttrString(pyExposed, "id");
			if (pyeid == NULL || !PyLong_Check(pyeid))
			{
				Py_XDECREF(pyeid);
				PyErr_Format( PyExc_TypeError,
					"Method::checkArgs: method[%s] requires None, an id, or an object with an "
					"id as its first agument", getName().c_str());

				PyErr_Print();
				return false;
			}
			
			Py_DECREF(pyeid);
		}
	}	
	
	for(uint8 i=0; i <argsSize; i++)
	{
		PyObject* pyArg = PyTuple_GetItem(args, i + offset);
		if (!argTypes_[i]->isSameType(pyArg))
		{
			PyObject* pExample = argTypes_[i]->createObject(NULL);
			PyErr_Format(PyExc_TypeError,
				"Method::checkArgs: method[%s] argument %d: Expected %s, %s found",
				getName().c_str(),
				i+1,
				pExample->ob_type->tp_name,
				pyArg->ob_type->tp_name );
			
			PyErr_Print();
			Py_DECREF(pExample);
			return false;
		}
	}	
	return true;
}

//-------------------------------------------------------------------------------------
void MethodDescription::addToStream(MemoryStream* mstream, PyObject* args)
{
	uint8 argsSize = argTypes_.size();
	int offset = 0;

	// 将utype放进去，方便对端识别这个方法
	(*mstream) << utype_;

	// 如果是exposed方法则先将entityID打包进去
	if(isExposed())
	{
		offset = 1;
		ENTITY_ID eid = PyLong_AsLong(PyTuple_GetItem(args, 0));
		(*mstream) << eid;
	}

	// 将每一个参数添加到流中
	for(uint8 i=0; i <argsSize; i++)
	{
		PyObject* pyArg = PyTuple_GetItem(args, i + offset);
		argTypes_[i]->addToStream(mstream, pyArg);
	}	
}

//-------------------------------------------------------------------------------------
PyObject* MethodDescription::createFromStream(MemoryStream* mstream)
{
	size_t argSize = getArgSize();
	PyObject* pyArgsTuple = NULL;
	int offset = 0;
	
	if(isExposed())
	{
		offset = 1;
		ENTITY_ID id = 0;
		pyArgsTuple = PyTuple_New(argSize + offset);
		
		(*mstream) >> id;
		PyTuple_SET_ITEM(&*pyArgsTuple, 0, PyLong_FromLong(id));
	}
	else
		pyArgsTuple = PyTuple_New(argSize);

	for(size_t index=0; index<argSize; index++)
	{
		PyTuple_SET_ITEM(&*pyArgsTuple, index + offset, argTypes_[index]->createFromStream(mstream));
	}
	
	return pyArgsTuple;
}

//-------------------------------------------------------------------------------------
size_t MethodDescription::getArgSize(void)
{
	return argTypes_.size();
}

//-------------------------------------------------------------------------------------
PyObject* MethodDescription::call(PyObject* func, PyObject* args)
{
	PyObject* pyResult = NULL;
	if (!PyCallable_Check(func))
	{
		PyErr_Format(PyExc_TypeError, "MethodDescription::call: Script[%s] call attempted on a error object!", getName().c_str());
		PyErr_Print();
	}
	else
	{
		if(checkArgs(args))
			pyResult = PyObject_CallObject(func, args);
	}

	SCRIPT_ERROR_CHECK();
	return pyResult;
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod::RemoteEntityMethod(MethodDescription* methodDescription, EntityMailboxAbstract* mailbox):
script::ScriptObject(getScriptType(), false),
methodDescription_(methodDescription),
pMailbox_(mailbox)
{
	Py_INCREF(pMailbox_);
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod::~RemoteEntityMethod()
{
	Py_DECREF(pMailbox_);
}

//-------------------------------------------------------------------------------------
PyObject* RemoteEntityMethod::tp_call(PyObject* self, PyObject* args, PyObject* kwds)	
{	
	RemoteEntityMethod* rmethod = static_cast<RemoteEntityMethod*>(self);
	MethodDescription* methodDescription = rmethod->getDescription();
	EntityMailboxAbstract* mailbox = rmethod->getMailbox();

	if(methodDescription->checkArgs(args))
	{
		Mercury::Bundle bundle;
		mailbox->newMail(bundle);
		MemoryStream mstream;
		methodDescription->addToStream(&mstream, args);
		bundle.append(mstream.data(), mstream.wpos());
		mailbox->postMail(bundle);
	}
	
	S_Return;
}		
	
//-------------------------------------------------------------------------------------

}
