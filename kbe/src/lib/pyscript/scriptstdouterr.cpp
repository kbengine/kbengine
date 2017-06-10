/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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


#include "scriptstdouterr.h"

#ifndef CODE_INLINE
#include "scriptstdouterr.inl"
#endif

namespace KBEngine{ namespace script{

							

//-------------------------------------------------------------------------------------
ScriptStdOutErr::ScriptStdOutErr():
pStderr_(NULL),
pStdout_(NULL),
isInstall_(false),
sbuffer_()
{
}

//-------------------------------------------------------------------------------------
ScriptStdOutErr::~ScriptStdOutErr()
{
}

//-------------------------------------------------------------------------------------
void ScriptStdOutErr::info_msg(const wchar_t* msg, uint32 msglen)
{
	std::wstring str;
	str.assign(msg, msglen);
	sbuffer_ += str;

	if(msg[0] == L'\n')
	{
		std::string out;
		strutil::wchar2utf8(sbuffer_, out);
		SCRIPT_INFO_MSG(out);
		sbuffer_ = L"";
	}
}

//-------------------------------------------------------------------------------------
void ScriptStdOutErr::error_msg(const wchar_t* msg, uint32 msglen)
{
	std::wstring str;
	str.assign(msg, msglen);
	sbuffer_ += str;

	if(msg[0] == L'\n')
	{
		std::string out;
		strutil::wchar2utf8(sbuffer_, out);
		SCRIPT_ERROR_MSG(out);
		sbuffer_ = L"";
	}
}

//-------------------------------------------------------------------------------------
bool ScriptStdOutErr::install(void)
{
	pStderr_ = new ScriptStdErr(this);
	pStdout_ = new ScriptStdOut(this);
	isInstall_ = pStderr_->install() && pStdout_->install();

	PyObject * m = PyImport_ImportModule("builtins");
	if (!m)
	{
		ERROR_MSG("ScriptStdOutErr: Failed to import builtins module\n");
		return false;
	}

	pyPrint_ = PyObject_GetAttrString(m, "print");
	Py_DECREF(m);

	return isInstall_;	
}

//-------------------------------------------------------------------------------------
bool ScriptStdOutErr::uninstall(void)
{
	if (pStderr_)
	{
		if(!pStderr_->uninstall())
		{
			Py_DECREF(pStderr_);
			return false;
		}

		Py_DECREF(pStderr_);
		pStderr_ = NULL;
	}

	if (pStdout_)
	{
		if(!pStdout_->uninstall())
		{
			Py_DECREF(pStdout_);
			return false;
		}

		Py_DECREF(pStdout_);
		pStdout_ = NULL;
	}

	if (pyPrint_)
	{
		Py_DECREF(pyPrint_);
		pyPrint_ = NULL;
	}

	isInstall_ = false;
	return true;	
}

//-------------------------------------------------------------------------------------
void ScriptStdOutErr::pyPrint(const std::string& str)
{
	PyObject* pyRet = PyObject_CallFunction(pyPrint_, 
		const_cast<char*>("(s)"), str.c_str());
	
	SCRIPT_ERROR_CHECK();
	
	if(pyRet)
	{
		S_RELEASE(pyRet);
	}
}

//-------------------------------------------------------------------------------------

}
}
