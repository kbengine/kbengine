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


namespace KBEngine {
namespace script{

INLINE int Script::run_simpleString(std::string command, std::string* retBufferPtr)
{
	return run_simpleString(command.c_str(), retBufferPtr);
}

INLINE PyObject* Script::getModule(void) const 
{ 
	return module_; 
}

INLINE PyObject* Script::getExtraModule(void) const 
{ 
	return extraModule_; 
}

INLINE ScriptStdOutErr* Script::pyStdouterr() const
{
	return pyStdouterr_;
}

INLINE void Script::pyPrint(const std::string& str)
{
	pyStdouterr_->pyPrint(str);
}

}
}

