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

#include "datatype.hpp"
#include "pyscript/py_memorystream.hpp"

namespace KBEngine{

class Blob : public script::PyMemoryStream
{		
	/** ���໯ ��һЩpy�������������� */
	INSTANCE_SCRIPT_HREADER(Blob, PyMemoryStream)
public:	
	Blob(bool readonly = false);
	Blob(std::string& strInitData, bool readonly = false);
	Blob(PyObject* pyBytesInitData, bool readonly = false);
	Blob(MemoryStream* streamInitData, bool readonly = false);

	virtual ~Blob();

	/** 
		֧��pickler ���� 
	*/
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol);

	/** 
		unpickle���� 
	*/
	static PyObject* __unpickle__(PyObject* self, PyObject* args);

	/** 
		�ű�����װʱ������ 
	*/
	static void onInstallScript(PyObject* mod);
	
	static PyObject* py_new();

	/** 
		��ʼ���̶��ֵ�
	*/
	void initialize(std::string strDictInitData);
	void initialize(PyObject* pyDictInitData);
	void initialize(MemoryStream* streamInitData);


protected:
} ;

}
#endif // KBE_PY_BLOB_HPP

