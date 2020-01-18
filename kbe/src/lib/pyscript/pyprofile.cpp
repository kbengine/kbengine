// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "script.h"
#include "pyprofile.h"
#include "pyobject_pointer.h"
#include "common/memorystream.h"

namespace KBEngine{ 
namespace script{

PyProfile::PROFILES PyProfile::profiles_;
bool PyProfile::isInit = false;
PyObject* PyProfile::profileMethod_ = NULL;
Script* PyProfile::pScript_ = NULL;

//-------------------------------------------------------------------------------------
bool PyProfile::initialize(Script* pScript)
{
	if (isInit)
		return true;

	PyObject* cProfileModule = PyImport_ImportModule("cProfile");

	if (!cProfileModule)
	{
		ERROR_MSG("can't import cProfile!\n");
		PyErr_PrintEx(0);
		return false;
	}
	else
	{
		Py_DECREF(cProfileModule);
	}

	profileMethod_ = PyObject_GetAttrString(cProfileModule, "Profile");

	isInit = profileMethod_ != NULL;
	pScript_ = pScript;
	return isInit;
}

//-------------------------------------------------------------------------------------
void PyProfile::finalise(void)
{
	profiles_.clear();

	Py_XDECREF(profileMethod_);
	profileMethod_ = NULL;
}

//-------------------------------------------------------------------------------------
bool PyProfile::start(std::string profile)
{
	PyProfile::PROFILES::iterator iter = profiles_.find(profile);
	if(iter != profiles_.end())
	{
		ERROR_MSG(fmt::format("PyProfile::start: profile({}) already exists!\n", profile));
		return false;
	}

	PyObject* pyRet = PyObject_CallFunction(profileMethod_, 
		const_cast<char*>(""));
	
	if(!pyRet)
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}
	
	PyObject* pyRet1 = PyObject_CallMethod(pyRet, const_cast<char*>("enable"),
		const_cast<char*>(""));

	if(!pyRet1)
	{
		SCRIPT_ERROR_CHECK();
		Py_DECREF(pyRet);
		return false;
	}

	Py_DECREF(pyRet1);

	profiles_[profile] = pyRet;

	char buf[MAX_BUF];
	kbe_snprintf(buf, MAX_BUF, "print(\"PyProfile::start: profile=%s.\")", profile.c_str());
	pScript_->run_simpleString(buf, NULL);
	return true;
}

//-------------------------------------------------------------------------------------
bool PyProfile::stop(std::string profile)
{
	PyProfile::PROFILES::iterator iter = profiles_.find(profile);
	if(iter == profiles_.end())
	{
		ERROR_MSG(fmt::format("PyProfile::stop: profile({}) is not exists!\n", profile));
		return false;
	}

	PyObject* pyRet = PyObject_CallMethod(iter->second.get(), const_cast<char*>("disable"),
		const_cast<char*>(""));
	
	if(!pyRet)
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}
	
	Py_DECREF(pyRet);

	char buf[MAX_BUF];
	kbe_snprintf(buf, MAX_BUF, "print(\"PyProfile::stop: profile=%s.\")", profile.c_str());
	pScript_->run_simpleString(buf, NULL);
	return true;
}

//-------------------------------------------------------------------------------------
bool PyProfile::remove(std::string profile)
{
	PyProfile::PROFILES::iterator iter = profiles_.find(profile);
	if(iter == profiles_.end())
	{
		ERROR_MSG(fmt::format("PyProfile::remove: profile({}) is not exists!\n", profile));
		return false;
	}

	Py_DECREF(iter->second.get());

	profiles_.erase(iter);
	return true;
}

//-------------------------------------------------------------------------------------
void PyProfile::print_stats(const std::string& sort, const std::string& profileName)
{
	PyProfile::PROFILES::iterator iter = profiles_.find(profileName.c_str());
	if(iter == profiles_.end())
	{
		return;
	}

	PyObject* pyRet = PyObject_CallMethod(iter->second.get(), const_cast<char*>("print_stats"),
		const_cast<char*>("s"), const_cast<char*>(sort.c_str()));
	
	if(pyRet)
		Py_DECREF(pyRet);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void PyProfile::addToStream(std::string profile, MemoryStream* s)
{
	PyProfile::PROFILES::iterator iter = profiles_.find(profile);
	if(iter == profiles_.end())
	{
		ERROR_MSG(fmt::format("PyProfile::getstats: profile({}) is not exists!\n", profile));
		return;
	}

	ScriptStdOutErrHook* pScriptStdOutErrHook = new ScriptStdOutErrHook();
	if(!pScriptStdOutErrHook->install())
	{												
		ERROR_MSG("PyProfile::addToStream: pyStdouterrHook_->install() is failed!\n");
		delete pScriptStdOutErrHook;
		SCRIPT_ERROR_CHECK();
		return;
	}

	std::string retBufferPtr;
	pScriptStdOutErrHook->setHookBuffer(&retBufferPtr);
	pScriptStdOutErrHook->setPrint(false);

	PyObject* pyRet = PyObject_CallMethod(iter->second.get(), const_cast<char*>("print_stats"),
		const_cast<char*>("s"), const_cast<char*>("time"));
	
	pScriptStdOutErrHook->setPrint(true);
	pScriptStdOutErrHook->uninstall();

	delete pScriptStdOutErrHook;
	SCRIPT_ERROR_CHECK();

	if(!pyRet)
	{
		return;
	}
	
	(*s) << retBufferPtr;

	// DEBUG_MSG(fmt::format("PyProfile::addToStream:{}", retBufferPtr));

	Py_DECREF(pyRet);
}

//-------------------------------------------------------------------------------------
bool PyProfile::dump(std::string profile, std::string fileName)
{
	/* ╪сть╫А╧Ш
		import pstats
		p = pstats.Stats("*.prof")
		p.sort_stats("time").print_stats()
	*/

	PyProfile::PROFILES::iterator iter = profiles_.find(profile);
	if(iter == profiles_.end())
	{
		ERROR_MSG(fmt::format("PyProfile::dump: profile({}) is not exists!\n", profile));
		return false;
	}

	FILE* f = fopen(fileName.c_str(), "wb");
	if(f == NULL)
	{
		ERROR_MSG(fmt::format("PyProfile::dump: profile({}) can't open fileName={}!\n", profile, fileName));
		return false;
	}

	PyObject* pyRet = PyObject_CallMethod(iter->second.get(), const_cast<char*>("dump_stats"),
		const_cast<char*>("s"), fileName.c_str());

	SCRIPT_ERROR_CHECK();

	if(!pyRet)
	{
		ERROR_MSG(fmt::format("PyProfile::dump: save to {} is error!\n", fileName));
		return false;
	}
	else
	{
		DEBUG_MSG(fmt::format("PyProfile::dump: save to {}.\n", fileName));
	}

	Py_DECREF(pyRet);

	pyRet = PyObject_CallMethod(iter->second.get(), const_cast<char*>("print_stats"),
		const_cast<char*>("s"), const_cast<char*>("time"));
	
	SCRIPT_ERROR_CHECK();
	
	if(pyRet)
		Py_DECREF(pyRet);

	return true;
}

//-------------------------------------------------------------------------------------

}
}
