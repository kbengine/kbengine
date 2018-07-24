// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef _SCRIPT_SEQUENCE_H
#define _SCRIPT_SEQUENCE_H
#include "common/common.h"
#include "scriptobject.h"
#include "pickler.h"
	
namespace KBEngine{ namespace script{

class Sequence : public ScriptObject
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(Sequence, ScriptObject)
public:	
	static PySequenceMethods seqMethods;
	static PyMappingMethods seqMapping;

	Sequence(PyTypeObject* pyType, bool isInitialised = false);
	virtual ~Sequence();
	
	static Py_ssize_t seq_length(PyObject* self);
	static PyObject* seq_concat(PyObject* self, PyObject* seq);
	static PyObject* seq_repeat(PyObject* self, Py_ssize_t n);
	static PyObject* seq_item(PyObject* self, Py_ssize_t index);
	static PyObject* seq_slice(PyObject* self, Py_ssize_t startIndex, Py_ssize_t endIndex);
	static int seq_ass_item(PyObject* self, Py_ssize_t index, PyObject* value);
	static int seq_ass_slice(PyObject* self, Py_ssize_t index1, Py_ssize_t index2, PyObject* oterSeq);
	static int seq_contains(PyObject* self, PyObject* value);
	static PyObject* seq_inplace_concat(PyObject* self, PyObject* oterSeq);
	static PyObject* seq_inplace_repeat(PyObject * self, Py_ssize_t n);
	
	static PyObject* seq_subscript(PyObject* self, PyObject* item);

	INLINE int length(void) const;
	INLINE std::vector<PyObject*>& getValues(void);

	int findFrom(uint32 startIndex, PyObject* value);
	
	virtual bool isSameType(PyObject* pyValue);
	virtual bool isSameItemType(PyObject* pyValue);
	virtual PyObject* createNewItemFromObj(PyObject* pyItem);

protected:
	std::vector<PyObject*>				values_;
} ;

}
}

#ifdef CODE_INLINE
#include "sequence.inl"
#endif

#endif
