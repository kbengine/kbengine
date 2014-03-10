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

#include "script.hpp"
#include "pyprofile.hpp"
#include "pyobject_pointer.hpp"
#include "cstdkbe/memorystream.hpp"

namespace KBEngine{ 
namespace script{

PyProfile::PROFILES PyProfile::profiles_;
bool PyProfile::isInit = false;
PyObject* PyProfile::profileMethod_ = NULL;
Script* PyProfile::pScript_ = NULL;

//-------------------------------------------------------------------------------------
bool PyProfile::initialize(Script* pScript)
{
	if(isInit)
		return true;
	
	PyObject* cProfileModule = PyImport_ImportModule("cProfile");

	if(!cProfileModule)
	{
		ERROR_MSG("can't import cProfile!\n");
		PyErr_PrintEx(0);
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
		ERROR_MSG(boost::format("PyProfile::start: profile(%1%) already exists!\n") % profile);
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
		ERROR_MSG(boost::format("PyProfile::stop: profile(%1%) is not exists!\n") % profile);
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
		ERROR_MSG(boost::format("PyProfile::remove: profile(%1%) is not exists!\n") % profile);
		return false;
	}

	Py_DECREF(iter->second.get());

	profiles_.erase(iter);
	return true;
}

//-------------------------------------------------------------------------------------
void PyProfile::addToStream(std::string profile, MemoryStream* s)
{
	PyProfile::PROFILES::iterator iter = profiles_.find(profile);
	if(iter == profiles_.end())
	{
		ERROR_MSG(boost::format("PyProfile::getstats: profile(%1%) is not exists!\n") % profile);
		return;
	}

	if(!pScript_->pyStdouterrHook()->install()){												
		ERROR_MSG("PyProfile::addToStream: pyStdouterrHook_->install() is failed!\n");
		SCRIPT_ERROR_CHECK();
		return;
	}

	std::string retBufferPtr;
	pScript_->pyStdouterrHook()->setHookBuffer(&retBufferPtr);
	pScript_->pyStdouterrHook()->setPrint(false);
	PyObject* pyRet = PyObject_CallMethod(iter->second.get(), const_cast<char*>("print_stats"),
		const_cast<char*>(""));
	
	pScript_->pyStdouterrHook()->setPrint(true);
	pScript_->pyStdouterrHook()->uninstall();

	SCRIPT_ERROR_CHECK();

	if(!pyRet)
	{
		return;
	}
	
	(*s) << retBufferPtr;

	// DEBUG_MSG(boost::format("PyProfile::addToStream:%1%") % retBufferPtr);

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
		ERROR_MSG(boost::format("PyProfile::dump: profile(%1%) is not exists!\n") % profile);
		return false;
	}

	FILE* f = fopen(fileName.c_str(), "rw+");
	if(f == NULL)
	{
		ERROR_MSG(boost::format("PyProfile::dump: profile(%1%) can't open fileName=%2%!\n") % profile % fileName);
		return false;
	}

	PyObject* pyRet = PyObject_CallMethod(iter->second.get(), const_cast<char*>("dump_stats"),
		const_cast<char*>("s"), fileName.c_str());

	SCRIPT_ERROR_CHECK();

	if(!pyRet)
	{
		return false;
	}
	
	DEBUG_MSG(boost::format("PyProfile::dump: save to %1%.") % fileName);

	Py_DECREF(pyRet);
	return true;
}

//-------------------------------------------------------------------------------------

}
}
