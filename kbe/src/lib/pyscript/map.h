// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef _SCRIPT_MAP_H
#define _SCRIPT_MAP_H
#include "common/common.h"
#include "scriptobject.h"
#include "pickler.h"
	
namespace KBEngine{ namespace script{

class Map : public ScriptObject
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(Map, ScriptObject)

public:	
	static PyMappingMethods mappingMethods;
	static PySequenceMethods mappingSequenceMethods;

	Map(PyTypeObject* pyType, bool isInitialised = false);
	virtual ~Map();

	/** 
		暴露一些字典方法给python 
	*/
	static PyObject* __py_has_key(PyObject* self, PyObject* args);
	static PyObject* __py_keys(PyObject* self, PyObject* args);
	static PyObject* __py_values(PyObject* self, PyObject* args);
	static PyObject* __py_items(PyObject* self, PyObject* args);
	static PyObject* __py_update(PyObject* self, PyObject* args);
	static PyObject* __py_get(PyObject* self, PyObject* args);

	static int seq_contains(PyObject* self, PyObject* value);

	/** 
		map操作函数相关 
	*/
	static PyObject* mp_subscript(PyObject* self, PyObject* key);

	static int mp_ass_subscript(PyObject* self, 
		PyObject* key, PyObject* value);

	static int mp_length(PyObject* self);

	static PyObject* mp_keyiter(PyObject* self);
	static PyObject* mp_iternextkey(PyObject* iter);

	/** 
		获取字典对象 
	*/
	INLINE PyObject* getDictObject(void) const;
	
	/** 
		数据改变通知 
	*/
	virtual void onDataChanged(PyObject* key, PyObject* value, 
		bool isDelete = false);

protected:
	// 字典数据， 所有的数据都往这里面写
	PyObject* pyDict_;
} ;

}
}

#ifdef CODE_INLINE
#include "map.inl"
#endif

#endif
