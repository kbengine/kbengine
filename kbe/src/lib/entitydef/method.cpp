#include "method.hpp"
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
MethodDescription::MethodDescription(std::string name, bool isExposed):
m_name_(name),
m_isExposed_(isExposed)
{
	MethodDescription::methodDescriptionCount_++;
	m_utype_ = MethodDescription::methodDescriptionCount_;
}

//-------------------------------------------------------------------------------------
MethodDescription::~MethodDescription()
{
	std::vector<DataType*>::iterator iter = m_argTypes_.begin();
	for(; iter != m_argTypes_.end(); iter++)
		(*iter)->decRef();
	m_argTypes_.clear();
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
	m_argTypes_.push_back(dataType);
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
	uint8 argsSize = m_argTypes_.size();
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
		if (!m_argTypes_[i]->isSameType(pyArg))
		{
			PyObject* pExample = m_argTypes_[i]->createObject(NULL);
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
	uint8 argsSize = m_argTypes_.size();
	int offset = 0;

	// 将utype放进去，方便对端识别这个方法
	(*mstream) << (uint32)m_utype_;

	// 如果是exposed方法则先将entityID打包进去
	if(isExposed())
	{
		offset = 1;
		(*mstream) << (ENTITY_ID)PyLong_AsLong(PyTuple_GetItem(args, 0));
	}

	// 将每一个参数添加到流中
	for(uint8 i=0; i <argsSize; i++)
	{
		PyObject* pyArg = PyTuple_GetItem(args, i + offset);
		m_argTypes_[i]->addToStream(mstream, pyArg);
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
		
		(*mstream) >> (ENTITY_ID)id;
		PyTuple_SET_ITEM(&*pyArgsTuple, 0, PyLong_FromLong(id));
	}
	else
		pyArgsTuple = PyTuple_New(argSize);

	for(size_t index=0; index<argSize; index++)
	{
		PyTuple_SET_ITEM(&*pyArgsTuple, index + offset, m_argTypes_[index]->createFromStream(mstream));
	}
	
	return pyArgsTuple;
}

//-------------------------------------------------------------------------------------
size_t MethodDescription::getArgSize(void)
{
	return m_argTypes_.size();
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
m_methodDescription_(methodDescription),
m_pMailbox_(mailbox)
{
	Py_INCREF(m_pMailbox_);
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod::~RemoteEntityMethod()
{
	Py_DECREF(m_pMailbox_);
}

//-------------------------------------------------------------------------------------
PyObject* RemoteEntityMethod::tp_call(PyObject* self, PyObject* args, PyObject* kwds)	
{	
	RemoteEntityMethod* rmethod = static_cast<RemoteEntityMethod*>(self);
	MethodDescription* methodDescription = rmethod->getDescription();
	EntityMailboxAbstract* mailbox = rmethod->getMailbox();

	if(methodDescription->checkArgs(args))
	{
		//Packet* sp = mailbox->createMail(MAIL_TYPE_REMOTE_CALL);
		//methodDescription->addToStream(sp, args);
		//mailbox->post(sp);
	}
	
	S_Return;
}		
	
//-------------------------------------------------------------------------------------

}
