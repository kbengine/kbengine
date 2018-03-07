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

#ifndef KBE_SCRIPTSTDOUTERR_H
#define KBE_SCRIPTSTDOUTERR_H

#include "helper/debug_helper.h"
#include "common/common.h"
#include "scriptobject.h"
#include "scriptstdout.h"
#include "scriptstderr.h"

namespace KBEngine{ namespace script{
class ScriptStdOutErr
{					
public:	
	ScriptStdOutErr();
	virtual ~ScriptStdOutErr();

	/** 
		安装和卸载这个模块 
	*/
	bool install(void);
	bool uninstall(void);
	bool isInstall(void) const{ return isInstall_; }

	virtual void error_msg(const wchar_t* msg, uint32 msglen);
	virtual void info_msg(const wchar_t* msg, uint32 msglen);

	void pyPrint(const std::string& str);

	INLINE std::wstring& buffer();

protected:
	ScriptStdErr* pStderr_;
	ScriptStdOut* pStdout_;
	PyObject* pyPrint_;
	bool isInstall_;
	std::wstring sbuffer_;
} ;

}
}

#ifdef CODE_INLINE
#include "scriptstdouterr.inl"
#endif

#endif // KBE_SCRIPTSTDOUTERR_H
