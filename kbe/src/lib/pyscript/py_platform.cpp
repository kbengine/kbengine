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

#include "script.h"
#include "py_platform.h"
#include "py_macros.h"
#include "helper/profile.h"

namespace KBEngine{ namespace script {
	
bool PyPlatform::isInit = false;


//-------------------------------------------------------------------------------------
bool PyPlatform::initialize(void)
{
	if(isInit)
		return true;

	isInit = true;
	return isInit;
}

//-------------------------------------------------------------------------------------
void PyPlatform::finalise(void)
{
}

//-------------------------------------------------------------------------------------
bool PyPlatform::rmdir(const std::string& path)
{
	wchar_t* s = strutil::char2wchar(path.c_str());
	bool ret = rmdir(s);
	free(s);
	return ret;
}

//-------------------------------------------------------------------------------------
bool PyPlatform::rmdir(const std::wstring& path)
{
	PyObject* pyShutilModule = PyImport_ImportModule("shutil");
	if (pyShutilModule)
	{
		PyObject* pyResult = PyObject_CallMethod(pyShutilModule, const_cast<char*>("rmtree"), const_cast<char*>("u#"), path.c_str(), path.size());
		Py_DECREF(pyShutilModule);

		if (pyResult != NULL)
		{
			Py_DECREF(pyResult);
		}
		else
		{
			SCRIPT_ERROR_CHECK();
			return false;
		}
	}
	else
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool PyPlatform::pathExists(const std::string& path)
{
	wchar_t* s = strutil::char2wchar(path.c_str());
	bool ret = pathExists(s);
	free(s);
	return ret;
}

//-------------------------------------------------------------------------------------
bool PyPlatform::pathExists(const std::wstring& path)
{
	PyObject* pyModule = PyImport_ImportModule("os");
	if (pyModule)
	{
		PyObject* pathModule = PyObject_GetAttrString(pyModule, "path");
		Py_DECREF(pyModule);

		if (pathModule)
		{
			PyObject* pyResult = PyObject_CallMethod(pathModule, const_cast<char*>("exists"), const_cast<char*>("u#"), path.c_str(), path.size());
			Py_DECREF(pathModule);

			if (pyResult != NULL)
			{
				bool ret = pyResult == Py_True;
				Py_DECREF(pyResult);
				return ret;
			}
			else
			{
				SCRIPT_ERROR_CHECK();
				return false;
			}
		}
		else
		{
			SCRIPT_ERROR_CHECK();
			return false;
		}
	}
	else
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------

}
}
