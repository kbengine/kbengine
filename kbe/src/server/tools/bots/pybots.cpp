// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "bots.h"
#include "pybots.h"
#include "clientobject.h"
#include "bots_interface.h"
#include "resmgr/resmgr.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "server/serverconfig.h"
#include "helper/console_helper.h"

#include "../../../server/baseapp/baseapp_interface.h"
#include "../../../server/loginapp/loginapp_interface.h"

namespace KBEngine{

PyMappingMethods PyBots::mappingMethods =
{
	(lenfunc)mp_length,								// mp_length
	(binaryfunc)mp_subscript,						// mp_subscript
	NULL											// mp_ass_subscript
};

SCRIPT_METHOD_DECLARE_BEGIN(PyBots)
SCRIPT_METHOD_DECLARE("has_key",			pyHas_key,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("keys",				pyKeys,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("values",				pyValues,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("items",				pyItems,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("get",				pyGet,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(PyBots)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(PyBots)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(PyBots, 0, 0, &PyBots::mappingMethods, 0, 0)	

//-------------------------------------------------------------------------------------
PyBots::PyBots():
ScriptObject(getScriptType(), false)
{
}

//-------------------------------------------------------------------------------------
PyBots::~PyBots()
{
}

//-------------------------------------------------------------------------------------
int PyBots::mp_length(PyObject * self)
{
	return (int)Bots::getSingleton().clients().size();
}
	
//-------------------------------------------------------------------------------------
PyObject * PyBots::mp_subscript(PyObject* self, PyObject* key /*entityID*/)
{
	Bots* bots = &Bots::getSingleton();
	int32 clientID = PyLong_AsLong(key);
	if (PyErr_Occurred())
		return NULL;

	ClientObject * pyClient = bots->findClientByAppID(clientID);

	if(pyClient == NULL)
	{
		PyErr_Format(PyExc_KeyError, "%d", clientID);
		PyErr_PrintEx(0);
		return NULL;
	}

	Py_INCREF(pyClient);
	return pyClient;
}

//-------------------------------------------------------------------------------------
PyObject* PyBots::pyHas_key(int32 clientID)
{
	return PyLong_FromLong((Bots::getSingleton().findClientByAppID(clientID) != NULL));
}

//-------------------------------------------------------------------------------------
PyObject* PyBots::pyKeys()
{
	Bots::CLIENTS& refclients = Bots::getSingleton().clients();
	PyObject* pyList = PyList_New(refclients.size());
	int i = 0;

	Bots::CLIENTS::const_iterator iter = refclients.begin();
	while (iter != refclients.end())
	{
		PyObject* clientID = PyLong_FromLong(iter->second->appID());
		PyList_SET_ITEM(pyList, i, clientID);

		i++;
		iter++;
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
PyObject* PyBots::pyValues()
{
	Bots::CLIENTS& refclients = Bots::getSingleton().clients();
	PyObject* pyList = PyList_New(refclients.size());
	int i = 0;

	Bots::CLIENTS::const_iterator iter = refclients.begin();
	while (iter != refclients.end())
	{
		Py_INCREF(iter->second);
		PyList_SET_ITEM(pyList, i, iter->second);

		i++;
		iter++;
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
PyObject* PyBots::pyItems()
{
	Bots::CLIENTS& refclients = Bots::getSingleton().clients();
	PyObject* pyList = PyList_New(refclients.size());
	int i = 0;

	Bots::CLIENTS::const_iterator iter = refclients.begin();
	while (iter != refclients.end())
	{
		PyObject * pTuple = PyTuple_New(2);
		PyObject* clientID = PyLong_FromLong(iter->second->appID());
		Py_INCREF(iter->second);							// PyObject Entity* 增加一个引用

		PyTuple_SET_ITEM(pTuple, 0, clientID);
		PyTuple_SET_ITEM(pTuple, 1, iter->second);
		PyList_SET_ITEM(pyList, i, pTuple);
		i++;
		iter++;
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
PyObject* PyBots::__py_pyGet(PyObject* self, PyObject * args, PyObject* kwds)
{
	Bots* bots = &Bots::getSingleton();
	PyObject * pDefault = Py_None;
	int32 id = 0;
	if (!PyArg_ParseTuple( args, "i|O", &id, &pDefault))
	{
		return NULL;
	}

	PyObject* pClient = static_cast<PyObject*>(bots->findClientByAppID(id));

	if (!pClient)
	{
		pClient = pDefault;
	}

	Py_INCREF(pClient);
	return pClient;
}

//-------------------------------------------------------------------------------------

}
