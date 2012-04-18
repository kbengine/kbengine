/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef _ARRAY_TYPE_H
#define _ARRAY_TYPE_H
#include <string>
#include "datatype.hpp"
#include "pyscript/sequence.hpp"
#include "pyscript/pickler.hpp"

namespace KBEngine{

class Array : public script::Sequence
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(Array, Sequence)
public:	
	Array(DataType* dataType);
	Array(DataType* dataType, std::string& strInitData);
	virtual ~Array();

	const DataType* getDataType(void){ return _dataType; }
	
	/** 初始化固定字典*/
	void initialize(std::string strInitData);

	/** 支持pickler 方法 */
	static PyObject* __reduce_ex__(PyObject* self, PyObject* protocol);
	/** unpickle方法 */
	static PyObject* __unpickle__(PyObject* self, PyObject* args);
	
	/** 脚本被安装时被调用 */
	static void onInstallScript(PyObject* mod);
	
	/** 一下为一个list所需的操作接口 */
	static PyObject* _append(PyObject* self, PyObject* args, PyObject* kwargs);	
	static PyObject* _count(PyObject* self, PyObject* args, PyObject* kwargs);
	static PyObject* _extend(PyObject* self, PyObject* args, PyObject* kwargs);	
	static PyObject* _index(PyObject* self, PyObject* args, PyObject* kwargs);
	static PyObject* _insert(PyObject* self, PyObject* args, PyObject* kwargs);	
	static PyObject* _pop(PyObject* self, PyObject* args, PyObject* kwargs);
	static PyObject* _remove(PyObject* self, PyObject* args, PyObject* kwargs);
	
	bool isSameType(PyObject* pyValue);
protected:
	ArrayType* _dataType;
} ;

}
#endif