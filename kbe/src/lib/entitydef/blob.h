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

#ifndef KBE_PY_BLOB_HPP
#define KBE_PY_BLOB_HPP

#include "datatype.h"
#include "pyscript/py_memorystream.h"

namespace KBEngine{

class Blob : public script::PyMemoryStream
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(Blob, PyMemoryStream)
public:	
	Blob(bool readonly = false);
	Blob(std::string& strInitData, bool readonly = false);
	Blob(PyObject* pyBytesInitData, bool readonly = false);
	Blob(MemoryStream* streamInitData, bool readonly = false);

	virtual ~Blob();

	/** 
		支持pickler 方法 
	*/
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol);

	/** 
		unpickle方法 
	*/
	static PyObject* __unpickle__(PyObject* self, PyObject* args);

	/** 
		脚本被安装时被调用 
	*/
	static void onInstallScript(PyObject* mod);
	
	static PyObject* py_new();

	/** 
		初始化固定字典
	*/
	void initialize(std::string strDictInitData);
	void initialize(PyObject* pyDictInitData);
	void initialize(MemoryStream* streamInitData);


protected:
} ;

}
#endif // KBE_PY_BLOB_HPP

