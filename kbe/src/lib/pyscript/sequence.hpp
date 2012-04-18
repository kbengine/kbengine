/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef _SCRIPT_SEQUENCE_H
#define _SCRIPT_SEQUENCE_H
#include "cstdkbe/cstdkbe.hpp"
#include "scriptobject.hpp"
#include "pickler.hpp"
	
namespace KBEngine{ namespace script{
class Sequence : public ScriptObject
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(Sequence, ScriptObject)
public:	
	static PySequenceMethods seqMethods;
	Sequence(PyTypeObject* pyType, bool isInitialised = false);
	virtual ~Sequence();
	
	static int seq_length(PyObject* self);
	static PyObject* seq_concat(PyObject* self, PyObject* seq);
	static PyObject* seq_repeat(PyObject* self, int n);
	static PyObject* seq_item(PyObject* self, int index);
	static PyObject* seq_slice(PyObject* self, int startIndex, int endIndex);
	static int seq_ass_item(PyObject* self, int index, PyObject* value);
	static int seq_ass_slice(PyObject* self, int index1, int index2, PyObject* oterSeq);
	static int seq_contains(PyObject* self, PyObject* value);
	static PyObject* seq_inplace_concat(PyObject* self, PyObject* oterSeq);
	static PyObject* seq_inplace_repeat(PyObject * self, int n);
	
	int length(void){ return values_.size(); }
	std::vector<PyObject*>& getValues(void){ return values_; }
	int findFrom(uint32 startIndex, PyObject* value);
	
	virtual bool isSameType(PyObject* pyValue);
protected:
	std::vector<PyObject*>				values_;
} ;

}
}
#endif