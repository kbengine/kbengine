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

#ifndef KBE_SCRIPTSTDOUTERRHOOK_H
#define KBE_SCRIPTSTDOUTERRHOOK_H

#include "common/common.h"
#include "scriptobject.h"
#include "scriptstdouterr.h"

namespace KBEngine{ namespace script{

class ScriptStdOutErrHook : public ScriptStdOutErr
{
public:
	ScriptStdOutErrHook();
	~ScriptStdOutErrHook();

	virtual void error_msg(const wchar_t* msg, uint32 msglen);
	virtual void info_msg(const wchar_t* msg, uint32 msglen);

	INLINE void setHookBuffer(std::string* buffer);

	INLINE void setPrint(bool v);

protected:
	std::string* buffer_;
	std::wstring wbuffer_;
	bool isPrint_;
} ;

}
}

#ifdef CODE_INLINE
#include "scriptstdouterrhook.inl"
#endif

#endif // KBE_SCRIPTSTDOUTERRHOOK_H
