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

#ifndef _SCRIPT_MAP_H
#define _SCRIPT_MAP_H
#include "cstdkbe/cstdkbe.hpp"
#include "scriptobject.hpp"
#include "pickler.hpp"
	
namespace KBEngine{ namespace script{

class Map : public ScriptObject
{		
	/** ���໯ ��һЩpy�������������� */
	INSTANCE_SCRIPT_HREADER(Map, ScriptObject)
public:	
	static PyMappingMethods mappingMethods;

	Map(PyTypeObject* pyType, bool isInitialised = false);
	virtual ~Map();

	/** 
		��¶һЩ�ֵ䷽����python 
	*/
	static PyObject* __py_has_key(PyObject* self, PyObject* args);
	static PyObject* __py_keys(PyObject* self, PyObject* args);
	static PyObject* __py_values(PyObject* self, PyObject* args);
	static PyObject* __py_items(PyObject* self, PyObject* args);
	static PyObject* __py_update(PyObject* self, PyObject* args);
	static PyObject* __py_get(PyObject* self, PyObject* args);

	/** 
		map����������� 
	*/
	static PyObject* mp_subscript(PyObject* self, PyObject* key);

	static int mp_ass_subscript(PyObject* self, 
		PyObject* key, PyObject* value);

	static int mp_length(PyObject* self);

	/** 
		��ȡ�ֵ���� 
	*/
	INLINE PyObject* getDictObject(void)const;
	
	/** 
		���ݸı�֪ͨ 
	*/
	virtual void onDataChanged(std::string& key, std::string& value, 
		bool isDelete = false);

protected:
	PyObject* pyDict_;											// �ֵ����ݣ� ���е����ݶ���������д
} ;

}
}

#ifdef CODE_INLINE
#include "map.ipp"
#endif

#endif
