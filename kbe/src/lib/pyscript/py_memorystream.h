// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_PY_MEMORYSTREAM_H
#define KBE_PY_MEMORYSTREAM_H

#include "scriptobject.h"
#include "helper/debug_helper.h"
#include "common/common.h"
#include "common/memorystream.h"

namespace KBEngine{ namespace script{

class PyMemoryStream : public ScriptObject
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(PyMemoryStream, ScriptObject)
public:	
	static PySequenceMethods seqMethods;

	PyMemoryStream(bool readonly = false);
	PyMemoryStream(std::string& strInitData, bool readonly = false);
	PyMemoryStream(PyObject* pyBytesInitData, bool readonly = false);
	PyMemoryStream(MemoryStream* streamInitData, bool readonly = false);

	PyMemoryStream(PyTypeObject* pyType, bool isInitialised = false, bool readonly = false);
	virtual ~PyMemoryStream();


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

	INLINE MemoryStream& stream();

	INLINE PyObject* pyBytes();

	INLINE bool readonly() const;

	void addToStream(MemoryStream* mstream);

	void createFromStream(MemoryStream* mstream);

	/** 
		获得对象的描述 
	*/
	PyObject* tp_repr();
	PyObject* tp_str();

	static PyObject* __py_append(PyObject* self, PyObject* args, PyObject* kwargs);	
	static PyObject* __py_pop(PyObject* self, PyObject* args, PyObject* kwargs);	
	
	static Py_ssize_t seq_length(PyObject* self);
	INLINE int length(void) const;

	static PyObject* __py_bytes(PyObject* self, PyObject* args, PyObject* kwargs);	
	
	static PyObject* __py_rpos(PyObject* self, PyObject* args, PyObject* kwargs);
	static PyObject* __py_wpos(PyObject* self, PyObject* args, PyObject* kwargs);

	static PyObject* __py_fill(PyObject* self, PyObject* args, PyObject* kwargs);

protected:
	MemoryStream stream_;
	bool readonly_;
} ;

}
}

#ifdef CODE_INLINE
#include "py_memorystream.inl"
#endif

#endif // KBE_PY_MEMORYSTREAM_H
