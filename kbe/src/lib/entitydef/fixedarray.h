// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef _FIXED_ARRAY_TYPE_H
#define _FIXED_ARRAY_TYPE_H
#include <string>
#include "datatype.h"
#include "pyscript/sequence.h"
#include "pyscript/pickler.h"

namespace KBEngine{

class FixedArray : public script::Sequence
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(FixedArray, Sequence)

public:	
	FixedArray(DataType* dataType);
	virtual ~FixedArray();

	const DataType* getDataType(void){ return _dataType; }
	
	/** 
		初始化固定数组
	*/
	void initialize(std::string strInitData);
	void initialize(PyObject* pyObjInitData);

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
		一下为一个list所需的操作接口 
	*/
	static PyObject* __py_append(PyObject* self, PyObject* args, PyObject* kwargs);	
	static PyObject* __py_count(PyObject* self, PyObject* args, PyObject* kwargs);
	static PyObject* __py_extend(PyObject* self, PyObject* args, PyObject* kwargs);	
	static PyObject* __py_index(PyObject* self, PyObject* args, PyObject* kwargs);
	static PyObject* __py_insert(PyObject* self, PyObject* args, PyObject* kwargs);	
	static PyObject* __py_pop(PyObject* self, PyObject* args, PyObject* kwargs);
	static PyObject* __py_remove(PyObject* self, PyObject* args, PyObject* kwargs);
	static PyObject* __py_clear(PyObject* self, PyObject* args, PyObject* kwargs);
	
	bool isSameType(PyObject* pyValue);
	bool isSameItemType(PyObject* pyValue);

	virtual PyObject* createNewItemFromObj(PyObject* pyItem);

	/** 
		获得对象的描述 
	*/
	PyObject* tp_repr();
	PyObject* tp_str();

protected:
	FixedArrayType* _dataType;
} ;

}
#endif
