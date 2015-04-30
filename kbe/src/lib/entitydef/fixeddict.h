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

#ifndef KBE_FIXED_DICT_H
#define KBE_FIXED_DICT_H

#include <string>
#include "datatype.h"
#include "helper/debug_helper.h"
#include "common/common.h"
#include "pyscript/map.h"
#include "pyscript/pickler.h"

namespace KBEngine{

class FixedDict : public script::Map
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(FixedDict, Map)
public:	
	static PyMappingMethods mappingMethods;
	static PySequenceMethods mappingSequenceMethods;

	FixedDict(DataType* dataType);
	FixedDict(DataType* dataType, std::string& strDictInitData);
	FixedDict(DataType* dataType, PyObject* pyDictInitData);
	FixedDict(DataType* dataType, MemoryStream* streamInitData, bool isPersistentsStream);

	virtual ~FixedDict();

	DataType* getDataType(void){ return _dataType; }

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
	
	/** 
		map操作函数相关 
	*/
	static PyObject* mp_subscript(PyObject* self, PyObject* key);

	static int mp_ass_subscript(PyObject* self, PyObject* key, 
		PyObject* value);

	static int mp_length(PyObject* self);

	/** 
		初始化固定字典
	*/
	void initialize(std::string strDictInitData);
	void initialize(PyObject* pyDictInitData);
	void initialize(MemoryStream* streamInitData, bool isPersistentsStream);

	/** 
		检查数据改变 
	*/
	bool checkDataChanged(const char* keyName, 
		PyObject* value,
		bool isDelete = false);
	
	/**
		更新字典数据到自己的数据中 
	*/
	PyObject* update(PyObject* args);

	/** 
		获得对象的描述 
	*/
	PyObject* tp_repr();
	PyObject* tp_str();

protected:
	FixedDictType* _dataType;
} ;

}
#endif // KBE_FIXED_DICT_H
