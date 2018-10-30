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


#include "scriptstdouterrhook.h"

#ifndef CODE_INLINE
#include "scriptstdouterrhook.inl"
#endif

namespace KBEngine{ namespace script{
								
//-------------------------------------------------------------------------------------
ScriptStdOutErrHook::ScriptStdOutErrHook():
isPrint_(true)
{
}

//-------------------------------------------------------------------------------------
ScriptStdOutErrHook::~ScriptStdOutErrHook()
{
}

//-------------------------------------------------------------------------------------
void ScriptStdOutErrHook::info_msg(const char* msg, uint32 msglen)
{
	if(isPrint_)
		ScriptStdOutErr::info_msg(msg, msglen);

	buffer_ += msg;

	if(msg[0] == '\n')
	{
		if(pBuffer_)
		{
			(*pBuffer_) += buffer_;
			buffer_ = "";
		}
	}
}

//-------------------------------------------------------------------------------------
void ScriptStdOutErrHook::error_msg(const char* msg, uint32 msglen)
{
	if(isPrint_)
		ScriptStdOutErr::error_msg(msg, msglen);

	buffer_ += msg;

	if(msg[0] == '\n')
	{
		if(pBuffer_)
		{
			(*pBuffer_) += buffer_;
			buffer_ = "";
		}
	}
}

//-------------------------------------------------------------------------------------

}
}
