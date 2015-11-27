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


#include "python_app.h"

namespace KBEngine{

//-------------------------------------------------------------------------------------
PythonApp::PythonApp(Network::EventDispatcher& dispatcher, 
					 Network::NetworkInterface& ninterface, 
					 COMPONENT_TYPE componentType,
					 COMPONENT_ID componentID):
ServerApp(dispatcher, ninterface, componentType, componentID),
script_(),
entryScript_()
{
}

//-------------------------------------------------------------------------------------
PythonApp::~PythonApp()
{
}

//-------------------------------------------------------------------------------------
bool PythonApp::inInitialize()
{
	if(!installPyScript())
		return false;

	if(!installPyModules())
		return false;
	
	return true;
}

//-------------------------------------------------------------------------------------
void PythonApp::finalise(void)
{
	uninstallPyScript();

	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
int PythonApp::registerPyObjectToScript(const char* attrName, PyObject* pyObj)
{ 
	return script_.registerToModule(attrName, pyObj); 
}

//-------------------------------------------------------------------------------------
int PythonApp::unregisterPyObjectToScript(const char* attrName)
{ 
	return script_.unregisterToModule(attrName); 
}

//-------------------------------------------------------------------------------------
bool PythonApp::installPyScript()
{
	if(Resmgr::getSingleton().respaths().size() <= 0 || 
		Resmgr::getSingleton().getPyUserResPath().size() == 0 || 
		Resmgr::getSingleton().getPySysResPath().size() == 0 ||
		Resmgr::getSingleton().getPyUserScriptsPath().size() == 0)
	{
		KBE_ASSERT(false && "PythonApp::installPyScript: KBE_RES_PATH is error!\n");
		return false;
	}

	std::wstring user_scripts_path = L"";
	wchar_t* tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(Resmgr::getSingleton().getPyUserScriptsPath().c_str()));
	if(tbuf != NULL)
	{
		user_scripts_path += tbuf;
		free(tbuf);
	}
	else
	{
		KBE_ASSERT(false && "PythonApp::installPyScript: KBE_RES_PATH error[char2wchar]!\n");
		return false;
	}

	std::wstring pyPaths = user_scripts_path + L"common;";
	pyPaths += user_scripts_path + L"data;";
	pyPaths += user_scripts_path + L"user_type;";

	switch(componentType_)
	{
	case BASEAPP_TYPE:
		pyPaths += user_scripts_path + L"server_common;";
		pyPaths += user_scripts_path + L"base;";
		break;
	case CELLAPP_TYPE:
		pyPaths += user_scripts_path + L"server_common;";
		pyPaths += user_scripts_path + L"cell;";
		break;
	case DBMGR_TYPE:
		pyPaths += user_scripts_path + L"server_common;";
		pyPaths += user_scripts_path + L"db;";
		break;
	case INTERFACES_TYPE:
		pyPaths += user_scripts_path + L"server_common;";
		pyPaths += user_scripts_path + L"interface;";
		break;
	case LOGINAPP_TYPE:
		pyPaths += user_scripts_path + L"server_common;";
		pyPaths += user_scripts_path + L"login;";
		break;
	default:
		pyPaths += user_scripts_path + L"client;";
		break;
	};
	
	std::string kbe_res_path = Resmgr::getSingleton().getPySysResPath();
	kbe_res_path += "scripts/common";

	tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(kbe_res_path.c_str()));
	bool ret = getScript().install(tbuf, pyPaths, "KBEngine", componentType_);
	free(tbuf);
	return ret;
}

//-------------------------------------------------------------------------------------
bool PythonApp::uninstallPyScript()
{
	return uninstallPyModules() && getScript().uninstall();
}

//-------------------------------------------------------------------------------------
bool PythonApp::installPyModules()
{
	// ��װ���ģ��
	PyObject *entryScriptFileName = NULL;
	if(componentType() == BASEAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getBaseApp();
		entryScriptFileName = PyUnicode_FromString(info.entryScriptFile);
	}
	else if(componentType() == CELLAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getCellApp();
		entryScriptFileName = PyUnicode_FromString(info.entryScriptFile);
	}
	else if(componentType() == INTERFACES_TYPE)
	{
		ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getInterfaces();
		entryScriptFileName = PyUnicode_FromString(info.entryScriptFile);
	}
	else if (componentType() == LOGINAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getLoginApp();
		entryScriptFileName = PyUnicode_FromString(info.entryScriptFile);
	}
	else if (componentType() == DBMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getDBMgr();
		entryScriptFileName = PyUnicode_FromString(info.entryScriptFile);
	}
	else
	{
		ERROR_MSG("PythonApp::installPyModules: entryScriptFileName is NULL!\n");
	}

	// ע�ᴴ��entity�ķ�����py
	// ��ű�ע��app����״̬
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	publish,			__py_getAppPublish,						METH_VARARGS,	0);

	// ע�����ýű��������
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	scriptLogType,		__py_setScriptLogType,					METH_VARARGS,	0);
	
	// �����Դȫ·��
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	getResFullPath,		__py_getResFullPath,					METH_VARARGS,	0);

	// �Ƿ����ĳ����Դ
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	hasRes,				__py_hasRes,							METH_VARARGS,	0);

	// ��һ���ļ�
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	open,				__py_kbeOpen,							METH_VARARGS,	0);

	// �г�Ŀ¼�������ļ�
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	listPathRes,		__py_listPathRes,						METH_VARARGS,	0);

	// ƥ�����·�����ȫ·��
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	matchPath,			__py_matchPath,							METH_VARARGS,	0);

	// debug׷��kbe��װ��py�������
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	debugTracing,		script::PyGC::__py_debugTracing,		METH_VARARGS,	0);

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_NORMAL", log4cxx::ScriptLevel::SCRIPT_INT))
	{
		ERROR_MSG( "PythonApp::installPyModules: Unable to set KBEngine.LOG_TYPE_NORMAL.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_INFO", log4cxx::ScriptLevel::SCRIPT_INFO))
	{
		ERROR_MSG( "PythonApp::installPyModules: Unable to set KBEngine.LOG_TYPE_INFO.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_ERR", log4cxx::ScriptLevel::SCRIPT_ERR))
	{
		ERROR_MSG( "PythonApp::installPyModules: Unable to set KBEngine.LOG_TYPE_ERR.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_DBG", log4cxx::ScriptLevel::SCRIPT_DBG))
	{
		ERROR_MSG( "PythonApp::installPyModules: Unable to set KBEngine.LOG_TYPE_DBG.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_WAR", log4cxx::ScriptLevel::SCRIPT_WAR))
	{
		ERROR_MSG( "PythonApp::installPyModules: Unable to set KBEngine.LOG_TYPE_WAR.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "NEXT_ONLY", KBE_NEXT_ONLY))
	{
		ERROR_MSG( "PythonApp::installPyModules: Unable to set KBEngine.NEXT_ONLY.\n");
	}
	
	onInstallPyModules();

	if (entryScriptFileName != NULL)
	{
		entryScript_ = PyImport_Import(entryScriptFileName);
		SCRIPT_ERROR_CHECK();
		S_RELEASE(entryScriptFileName);

		if(entryScript_.get() == NULL)
		{
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool PythonApp::uninstallPyModules()
{
	// script::PyGC::set_debug(script::PyGC::DEBUG_STATS|script::PyGC::DEBUG_LEAK);
	// script::PyGC::collect();

	script::PyGC::debugTracing();
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* PythonApp::__py_getAppPublish(PyObject* self, PyObject* args)
{
	return PyLong_FromLong(g_appPublish);
}

//-------------------------------------------------------------------------------------
PyObject* PythonApp::__py_setScriptLogType(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::scriptLogType(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	int type = -1;

	if(PyArg_ParseTuple(args, "i", &type) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::scriptLogType(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	DebugHelper::getSingleton().setScriptMsgType(type);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* PythonApp::__py_getResFullPath(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getResFullPath(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* respath = NULL;

	if(PyArg_ParseTuple(args, "s", &respath) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getResFullPath(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(!Resmgr::getSingleton().hasRes(respath))
		return PyUnicode_FromString("");

	std::string fullpath = Resmgr::getSingleton().matchRes(respath);
	return PyUnicode_FromString(fullpath.c_str());
}

//-------------------------------------------------------------------------------------
PyObject* PythonApp::__py_hasRes(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::hasRes(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* respath = NULL;

	if(PyArg_ParseTuple(args, "s", &respath) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::hasRes(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	return PyBool_FromLong(Resmgr::getSingleton().hasRes(respath));
}

//-------------------------------------------------------------------------------------
PyObject* PythonApp::__py_kbeOpen(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 2)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::open(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* respath = NULL;
	char* fargs = NULL;

	if(PyArg_ParseTuple(args, "s|s", &respath, &fargs) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::open(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	std::string sfullpath = Resmgr::getSingleton().matchRes(respath);

	PyObject *ioMod = PyImport_ImportModule("io");

	// SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	PyObject *openedFile = PyObject_CallMethod(ioMod, const_cast<char*>("open"), 
		const_cast<char*>("ss"), 
		const_cast<char*>(sfullpath.c_str()), 
		fargs);

	Py_DECREF(ioMod);
	return openedFile;
}

//-------------------------------------------------------------------------------------
PyObject* PythonApp::__py_matchPath(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::matchPath(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* respath = NULL;

	if(PyArg_ParseTuple(args, "s", &respath) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::matchPath(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	std::string path = Resmgr::getSingleton().matchPath(respath);
	return PyUnicode_FromStringAndSize(path.c_str(), path.size());
}

//-------------------------------------------------------------------------------------
PyObject* PythonApp::__py_listPathRes(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount < 1 || argCount > 2)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path, pathargs=\'*.*\'] is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	std::wstring wExtendName = L"*";
	PyObject* pathobj = NULL;
	PyObject* path_argsobj = NULL;

	if(argCount == 1)
	{
		if(PyArg_ParseTuple(args, "O", &pathobj) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path] is error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else
	{
		if(PyArg_ParseTuple(args, "O|O", &pathobj, &path_argsobj) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path, pathargs=\'*.*\'] is error!");
			PyErr_PrintEx(0);
			return 0;
		}
		
		if(PyUnicode_Check(path_argsobj))
		{
			wchar_t* fargs = NULL;
			fargs = PyUnicode_AsWideCharString(path_argsobj, NULL);
			wExtendName = fargs;
			PyMem_Free(fargs);
		}
		else
		{
			if(PySequence_Check(path_argsobj))
			{
				wExtendName = L"";
				Py_ssize_t size = PySequence_Size(path_argsobj);
				for(int i=0; i<size; ++i)
				{
					PyObject* pyobj = PySequence_GetItem(path_argsobj, i);
					if(!PyUnicode_Check(pyobj))
					{
						PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path, pathargs=\'*.*\'] is error!");
						PyErr_PrintEx(0);
						return 0;
					}
					
					wchar_t* wtemp = NULL;
					wtemp = PyUnicode_AsWideCharString(pyobj, NULL);
					wExtendName += wtemp;
					wExtendName += L"|";
					PyMem_Free(wtemp);
				}
			}
			else
			{
				PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[pathargs] is error!");
				PyErr_PrintEx(0);
				return 0;
			}
		}
	}

	if(!PyUnicode_Check(pathobj))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path] is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(PyUnicode_GET_LENGTH(pathobj) == 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path] is NULL!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(wExtendName.size() == 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[pathargs] is NULL!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(wExtendName[0] == '.')
		wExtendName.erase(wExtendName.begin());

	if(wExtendName.size() == 0)
		wExtendName = L"*";

	wchar_t* respath = PyUnicode_AsWideCharString(pathobj, NULL);
	if(respath == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path] is NULL!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* cpath = strutil::wchar2char(respath);
	std::string foundPath = Resmgr::getSingleton().matchPath(cpath);
	free(cpath);
	PyMem_Free(respath);

	respath = strutil::char2wchar(foundPath.c_str());

	std::vector<std::wstring> results;
	Resmgr::getSingleton().listPathRes(respath, wExtendName, results);
	PyObject* pyresults = PyTuple_New(results.size());

	std::vector<std::wstring>::iterator iter = results.begin();
	int i = 0;

	for(; iter != results.end(); ++iter)
	{
		PyTuple_SET_ITEM(pyresults, i++, PyUnicode_FromWideChar((*iter).c_str(), (*iter).size()));
	}

	free(respath);
	return pyresults;
}

//-------------------------------------------------------------------------------------
void PythonApp::startProfile_(Network::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen)
{
	switch(profileType)
	{
	case 0:	// pyprofile
		new PyProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		return;
	default:
		break;
	};

	ServerApp::startProfile_(pChannel, profileName, profileType, timelen);
}

//-------------------------------------------------------------------------------------
void PythonApp::onExecScriptCommand(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string cmd;
	s.readBlob(cmd);

	PyObject* pycmd = PyUnicode_DecodeUTF8(cmd.data(), cmd.size(), NULL);
	if(pycmd == NULL)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}

	DEBUG_MSG(fmt::format("PythonApp::onExecScriptCommand: size({}), command={}.\n", 
		cmd.size(), cmd));

	std::string retbuf = "";
	PyObject* pycmd1 = PyUnicode_AsEncodedString(pycmd, "utf-8", NULL);
	script_.run_simpleString(PyBytes_AsString(pycmd1), &retbuf);

	if(retbuf.size() == 0)
	{
		retbuf = "\r\n";
	}

	// ��������ظ��ͻ���
	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	ConsoleInterface::ConsoleExecCommandCBMessageHandler msgHandler;
	(*pBundle).newMessage(msgHandler);
	ConsoleInterface::ConsoleExecCommandCBMessageHandlerArgs1::staticAddToBundle((*pBundle), retbuf);
	pChannel->send(pBundle);

	Py_DECREF(pycmd);
	Py_DECREF(pycmd1);
}

//-------------------------------------------------------------------------------------
void PythonApp::onReloadScript(bool fullReload)
{
}

//-------------------------------------------------------------------------------------
void PythonApp::reloadScript(bool fullReload)
{
	onReloadScript(fullReload);

	// SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	// ���нű����������
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onInit"), 
										const_cast<char*>("i"), 
										1);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
}
