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

#include "method.h"
#include "entitydef.h"
#include "network/bundle.h"

#ifndef CODE_INLINE
#include "method.inl"
#endif

namespace KBEngine{

uint32	MethodDescription::methodDescriptionCount_ = 0;
	
//-------------------------------------------------------------------------------------
MethodDescription::MethodDescription(ENTITY_METHOD_UID utype, COMPONENT_ID domain,
									 std::string name, 
									 bool isExposed):
methodDomain_(domain),
name_(name),
utype_(utype),
argTypes_(),
isExposed_(isExposed),
currCallerID_(0),
aliasID_(-1)
{
	MethodDescription::methodDescriptionCount_++;

	EntityDef::md5().append((void*)name_.c_str(), name_.size());
	EntityDef::md5().append((void*)&utype_, sizeof(ENTITY_METHOD_UID));
	EntityDef::md5().append((void*)&isExposed_, sizeof(bool));
}

//-------------------------------------------------------------------------------------
MethodDescription::~MethodDescription()
{
	std::vector<DataType*>::iterator iter = argTypes_.begin();
	for(; iter != argTypes_.end(); ++iter)
		(*iter)->decRef();

	argTypes_.clear();
}

//-------------------------------------------------------------------------------------
void MethodDescription::setExposed(void)
{ 
	isExposed_ = true; 
	EntityDef::md5().append((void*)&isExposed_, sizeof(bool));
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

	DATATYPE_UID uid = dataType->id();
	EntityDef::md5().append((void*)&uid, sizeof(DATATYPE_UID));
	EntityDef::md5().append((void*)&isExposed_, sizeof(bool));
	return true;
}

//-------------------------------------------------------------------------------------
bool MethodDescription::checkArgs(PyObject* args)
{
	if (args == NULL || !PyTuple_Check(args))
	{
		PyErr_Format(PyExc_AssertionError, "Method::checkArgs: method[%s] args is not a tuple.\n", 
			getName());

		PyErr_PrintEx(0);
		return false;
	}
	
	int offset = (isExposed() == true && g_componentType == CELLAPP_TYPE && isCell()) ? 1 : 0;
	uint8 argsSize = argTypes_.size();
	uint8 giveArgsSize = PyTuple_Size(args);

	if (giveArgsSize != argsSize + offset)
	{
		PyErr_Format(PyExc_AssertionError, "Method::checkArgs: method[%s] requires exactly %d argument%s%s; %d given", 
				getName(),
				argsSize,
				(offset > 0) ? " + exposed(1)" : "",
				(argsSize == 1) ? "" : "s",
				PyTuple_Size(args));

		PyErr_PrintEx(0);
		return false;
	}	
	
	
	// ����Ƿ���һ��exposed����
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
					"id as its first agument", getName());

				PyErr_PrintEx(0);
				return false;
			}
			
			Py_DECREF(pyeid);
		}
	}	
	
	for(uint8 i=0; i <argsSize; ++i)
	{
		PyObject* pyArg = PyTuple_GetItem(args, i + offset);
		if (!argTypes_[i]->isSameType(pyArg))
		{
			PyObject* pExample = argTypes_[i]->parseDefaultStr("");
			PyErr_Format(PyExc_AssertionError,
				"Method::checkArgs: method[%s] argument %d: Expected %s, %s found",
				getName(),
				i+1,
				pExample->ob_type->tp_name,
				pyArg != NULL ? pyArg->ob_type->tp_name : "NULL");
			
			PyErr_PrintEx(0);
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

	// ��utype�Ž�ȥ������Զ�ʶ���������
	// �������aliasID_����0�����һ���Ż��İ취�� ʹ��1�ֽڴ���
	if(aliasID_ < 0)
	{
		(*mstream) << utype_;
	}
	else
	{
		uint8 utype = (uint8)aliasID_;
		(*mstream) << utype;
	}

	// �����exposed�������Ƚ�entityID�����ȥ
	if(isExposed() && g_componentType == CELLAPP_TYPE && isCell())
	{
		offset = 1;
		ENTITY_ID eid = PyLong_AsLong(PyTuple_GetItem(args, 0));
		(*mstream) << eid;
	}

	// ��ÿһ��������ӵ�����
	for(uint8 i=0; i <argsSize; ++i)
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
	
	if(isExposed() && g_componentType == CELLAPP_TYPE && isCell())
	{
		offset = 1;
		pyArgsTuple = PyTuple_New(argSize + offset);

		// ����һ��������ID�ṩ���ű��ж���Դ�Ƿ���ȷ
		KBE_ASSERT(currCallerID_ > 0);
		PyTuple_SET_ITEM(pyArgsTuple, 0, PyLong_FromLong(currCallerID_));
	}
	else
		pyArgsTuple = PyTuple_New(argSize);

	for(size_t index=0; index<argSize; ++index)
	{
		PyObject* pyitem = argTypes_[index]->createFromStream(mstream);

		if(pyitem == NULL)
		{
			WARNING_MSG(fmt::format("MethodDescription::createFromStream: {} arg[{}][{}] is NULL.\n", 
				this->getName(), index, argTypes_[index]->getName()));
		}

		PyTuple_SET_ITEM(pyArgsTuple, index + offset, pyitem);
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
		PyErr_Format(PyExc_TypeError, "MethodDescription::call: method[%s] call attempted on a error object!", 
			getName());
	}
	else
	{
		if(args == NULL)
		{
			pyResult = PyObject_CallObject(func, NULL);
		}
		else
		{
			if(checkArgs(args))
				pyResult = PyObject_CallObject(func, args);
		}
	}

	if (PyErr_Occurred())
	{
		if (isExposed() && PyErr_ExceptionMatches(PyExc_TypeError))
		{
			WARNING_MSG(fmt::format("MethodDescription::call: {} is method of exposed, if there is a missing arguments error, "
				"try adding exposedID, For example: \ndef func(msg): => def func(exposedID, msg):\n",
				this->getName()));
		}

		PyErr_PrintEx(0);
	}

	return pyResult;
}

//-------------------------------------------------------------------------------------

}
