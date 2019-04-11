// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "method.h"
#include "entitydef.h"
#include "network/bundle.h"

#ifndef CODE_INLINE
#include "method.inl"
#endif

namespace KBEngine{

uint32 MethodDescription::methodDescriptionCount_ = 0;
	
//-------------------------------------------------------------------------------------
MethodDescription::MethodDescription(ENTITY_METHOD_UID utype, COMPONENT_ID domain,
	std::string name,
	EXPOSED_TYPE exposedType) :
methodDomain_(domain),
name_(name),
utype_(utype),
argTypes_(),
exposedType_(exposedType),
aliasID_(-1)
{
	MethodDescription::methodDescriptionCount_++;

	EntityDef::md5().append((void*)name_.c_str(), (int)name_.size());
	EntityDef::md5().append((void*)&utype_, sizeof(ENTITY_METHOD_UID));
	EntityDef::md5().append((void*)&exposedType_, sizeof(EXPOSED_TYPE));
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
void MethodDescription::setExposed(EXPOSED_TYPE type)
{
	if (exposedType_ == EXPOSED_AND_CALLER_CHECK)
		return;

	exposedType_ = type;
	EntityDef::md5().append((void*)&exposedType_, sizeof(EXPOSED_TYPE));
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
	EntityDef::md5().append((void*)&exposedType_, sizeof(EXPOSED_TYPE));
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
	
	int offset = (isExposed() == EXPOSED_AND_CALLER_CHECK && g_componentType == CELLAPP_TYPE && isCell()) ? 1 : 0;
	uint8 argsSize = (uint8)argTypes_.size();
	uint8 giveArgsSize = (uint8)PyTuple_Size(args);

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

	// 将utype放进去，方便对端识别这个方法
	// 这里如果aliasID_大于0则采用一个优化的办法， 使用1字节传输
	// 注意：在加载def时指定了客户端方法才设置aliasID，因此服务器内部不使用aliasID
	if(aliasID_ <= 0)
	{
		(*mstream) << utype_;
	}
	else
	{
		uint8 utype = (uint8)aliasID_;
		(*mstream) << utype;
	}

	// 如果是exposed方法则先将entityID打包进去
	if(isExposed() == EXPOSED_AND_CALLER_CHECK && g_componentType == CELLAPP_TYPE && isCell())
	{
		offset = 1;
	}

	// 将每一个参数添加到流中
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
	
	if(isExposed() == EXPOSED_AND_CALLER_CHECK && g_componentType == CELLAPP_TYPE && isCell())
	{
		offset = 1;
		pyArgsTuple = PyTuple_New(argSize + offset);

		// 设置一个调用者ID提供给脚本判断来源是否正确
		KBE_ASSERT(EntityDef::context().currEntityID > 0);
		PyTuple_SET_ITEM(pyArgsTuple, 0, PyLong_FromLong(EntityDef::context().currEntityID));
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
		if (isExposed() == EXPOSED_AND_CALLER_CHECK && PyErr_ExceptionMatches(PyExc_TypeError))
		{
			WARNING_MSG(fmt::format("MethodDescription::call: {} is exposed of method, if there is a missing arguments error, "
				"try adding callerEntityID, For example: \ndef func(msg): => def func(callerEntityID, msg):\n",
				this->getName()));
		}

		PyErr_PrintEx(0);
	}

	return pyResult;
}

//-------------------------------------------------------------------------------------

}
