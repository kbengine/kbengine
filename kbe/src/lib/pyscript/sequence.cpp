// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "sequence.h"

#ifndef CODE_INLINE
#include "sequence.inl"
#endif

namespace KBEngine{ namespace script{

PyMappingMethods Sequence::seqMapping = {
	(lenfunc)seq_length,
	(binaryfunc)seq_subscript,
	0
};

PySequenceMethods Sequence::seqMethods =
{
	seq_length,				// inquiry sq_length;				len(x)
	seq_concat,				// binaryfunc sq_concat;			x + y
	seq_repeat,				// intargfunc sq_repeat;			x * n
	seq_item,				// intargfunc sq_item;				x[i]
	0,//seq_slice,				// intintargfunc sq_slice;			x[i:j]
	seq_ass_item,			// intobjargproc sq_ass_item;		x[i] = v
	0,//seq_ass_slice,			// intintobjargproc sq_ass_slice;	x[i:j] = v
	seq_contains,			// objobjproc sq_contains;			v in x
	seq_inplace_concat,		// binaryfunc sq_inplace_concat;	x += y
	seq_inplace_repeat		// intargfunc sq_inplace_repeat;	x *= n
};

SCRIPT_METHOD_DECLARE_BEGIN(Sequence)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Sequence)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Sequence)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(Sequence, 0, &Sequence::seqMethods, &Sequence::seqMapping, 0, 0)
	
//-------------------------------------------------------------------------------------
Sequence::Sequence(PyTypeObject* pyType, bool isInitialised):
ScriptObject(pyType, isInitialised)
{
}

//-------------------------------------------------------------------------------------
Sequence::~Sequence()
{
	std::vector<PyObject*>::iterator iter = values_.begin();
	for(; iter != values_.end(); ++iter)
	{
		Py_DECREF((*iter));
	}

	values_.clear();
}

//-------------------------------------------------------------------------------------
bool Sequence::isSameType(PyObject* pyValue)
{
	if(pyValue == NULL)
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Sequence::isSameItemType(PyObject* pyValue)
{
	return true;
}

//-------------------------------------------------------------------------------------
int Sequence::findFrom(uint32 startIndex, PyObject* value)
{
	for (uint32 i = startIndex; i < values_.size(); ++i)
		if (value == &*values_[i]) return i;

	for (uint32 i = startIndex; i < values_.size(); ++i)
		if(PyObject_RichCompareBool(value, &*values_[i], Py_EQ)) 
			return i;

	return -1;
}

//-------------------------------------------------------------------------------------
Py_ssize_t Sequence::seq_length(PyObject* self)
{
	Sequence* seq = static_cast<Sequence*>(self);
	return seq->length();
}

//-------------------------------------------------------------------------------------
PyObject* Sequence::seq_concat(PyObject* self, PyObject* seq)
{
	if (!PySequence_Check(seq))
	{
		PyErr_SetString(PyExc_TypeError, "Sequence::seq_concat: argument to + must be a sequence");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	Sequence* self_seq = static_cast<Sequence*>(self);
	std::vector<PyObject*>& values = self_seq->getValues();
	
	int seqSize1 = (int)values.size();
	int seqSize2 = (int)PySequence_Size(seq);
	PyObject* pyList = PyList_New(seqSize1 + seqSize2);

	for (int i = 0; i < seqSize1; ++i)
		PyList_SET_ITEM(pyList, i, values[i]);

	for (int i = 0; i < seqSize2; ++i)
	{
		PyList_SET_ITEM(pyList, seqSize1 + i, PySequence_GetItem(seq, i));
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
PyObject* Sequence::seq_repeat(PyObject* self, Py_ssize_t n)
{
	if (n <= 0) 
		return PyList_New(0);

	Sequence* seq = static_cast<Sequence*>(self);
	std::vector<PyObject*>& values = seq->getValues();
	int seqSize1 = (int)values.size();

	PyObject* pyList = PyList_New(seqSize1 * n);
	// 可能没内存了
	if (pyList == NULL) 
		return NULL;

	for (int j = 0; j < seqSize1; ++j)
	{
		PyList_SET_ITEM(pyList, j, values[j]);
	}

	for (int i = 1; i < n; i++)
	{
		for (int j = 0; j < seqSize1; ++j)
		{
			PyObject* pyTemp = PyList_GET_ITEM(pyList, j);
			PyList_SET_ITEM(pyList, i * seqSize1 + j, pyTemp);
			Py_INCREF(pyTemp);
		}
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
PyObject* Sequence::seq_item(PyObject* self, Py_ssize_t index)
{
	Sequence* seq = static_cast<Sequence*>(self);
	std::vector<PyObject*>& values = seq->getValues();

	if (uint32(index) < values.size())
	{
		PyObject* pyobj = values[index];
		Py_INCREF(pyobj);
		return pyobj;
	}

	PyErr_SetString(PyExc_IndexError, "Sequence index out of range");
	return NULL;
}

//-------------------------------------------------------------------------------------
PyObject* Sequence::seq_subscript(PyObject* self, PyObject* item)
{
	if (PyIndex_Check(item)) {
		Py_ssize_t i;
		i = PyNumber_AsSsize_t(item, PyExc_IndexError);
		if (i == -1 && PyErr_Occurred())
			return NULL;
		if (i < 0) {
			Sequence* seq = static_cast<Sequence*>(self);
			i += seq->length();
		}
		return seq_item(self, i);
	}
	else if (PySlice_Check(item)) {
		Py_ssize_t start, stop, step, slicelength;

		if (PySlice_GetIndicesEx(item, Py_SIZE(self),
			&start, &stop, &step, &slicelength) < 0) {
			return NULL;
		}

		if (slicelength <= 0) {
			return PyList_New(0);
		}
		else if (step == 1) {
			return seq_slice(self, start, stop);
		}
		else {
			return seq_slice(self, start, stop);
		}
	}

	PyErr_Format(PyExc_TypeError,
		"Sequence indices must be integers, not %.200s",
		item->ob_type->tp_name);

	return NULL;
}

//-------------------------------------------------------------------------------------
PyObject* Sequence::seq_slice(PyObject* self, Py_ssize_t startIndex, Py_ssize_t endIndex)
{
	if (startIndex < 0)
		startIndex = 0;

	Sequence* seq = static_cast<Sequence*>(self);
	std::vector<PyObject*>& values = seq->getValues();
	if (endIndex > int(values.size()))
		endIndex = values.size();

	if (endIndex < startIndex)
		endIndex = startIndex;

	int length = (int)(endIndex - startIndex);

	if (length == int(values.size())) 
	{
		Py_INCREF(seq);
		return seq;
	}

	PyObject* pyRet = PyList_New(length);
	for (int i = (int)startIndex; i < (int)endIndex; ++i)
	{
		Py_INCREF(values[i]);
		PyList_SET_ITEM(pyRet, i - startIndex, values[i]);
	}

	return pyRet;
}

//-------------------------------------------------------------------------------------
int Sequence::seq_ass_item(PyObject* self, Py_ssize_t index, PyObject* value)
{
	Sequence* seq = static_cast<Sequence*>(self);
	std::vector<PyObject*>& values = seq->getValues();

	if (uint32(index) >= values.size())
	{
		PyErr_SetString(PyExc_IndexError, "Sequence assignment index out of range");
		PyErr_PrintEx(0);
		return -1;
	}

	if(value)
	{
		// 检查类别是否正确
		if(seq->isSameItemType(value))
		{
			values[index] = seq->createNewItemFromObj(value);
		}
		else
		{
			PyErr_SetString(PyExc_IndexError, "Sequence set to type error!");
			PyErr_PrintEx(0);
			return -1;
		}
	}
	else
	{
		Py_DECREF((*(values.begin() + index)));
		values.erase(values.begin() + index);
	}

	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* Sequence::createNewItemFromObj(PyObject* pyItem)
{
	Py_INCREF(pyItem);
	return pyItem;
}

//-------------------------------------------------------------------------------------
int Sequence::seq_ass_slice(PyObject* self, Py_ssize_t index1, Py_ssize_t index2, PyObject* oterSeq)
{
	Sequence* seq = static_cast<Sequence*>(self);
	std::vector<PyObject*>& values = seq->getValues();
		
	// 是否是删除元素
	if (!oterSeq)
	{
		if (index1 < index2)
		{
			for (Py_ssize_t istart = index1; istart < index2; ++istart)
			{
				Py_DECREF(values[istart]);
			}

			values.erase(values.begin() + index1, values.begin() + index2);
		}

		return 0;
	}

	// oterSeq必须是一个 sequence
	if (!PySequence_Check(oterSeq))
	{
		PyErr_Format(PyExc_TypeError, "Sequence slices can only be assigned to a sequence");
		PyErr_PrintEx(0);
		return -1;
	}

	if (oterSeq == seq)
	{
		PyErr_Format(PyExc_TypeError, "Sequence does not support assignment of itself to a slice of itself");
		PyErr_PrintEx(0);
		return -1;
	}

	int sz = (int)values.size();
	int osz = (int)PySequence_Size(oterSeq);

	// 保证index不会越界
	if (index1 > sz) index1 = sz;
	if (index1 < 0) index1 = 0;
	if (index2 > sz) index2 = sz;
	if (index2 < 0) index2 = 0;

	// 检查一下 看看有无错误类别
	for (int i = 0; i < osz; ++i)
	{
		PyObject* pyVal = PySequence_GetItem(oterSeq, i);
		bool ok = seq->isSameItemType(pyVal);
		Py_DECREF(pyVal);

		if (!ok)
		{
			PyErr_Format(PyExc_TypeError, "Array elements must be set to type %s (setting slice %d-%d)", "ss", index1, index2);
			PyErr_PrintEx(0);
			return -1;
		}
	}

	if (index1 < index2)
	{
		for (Py_ssize_t istart = index1; istart < index2; ++istart)
		{
			Py_DECREF(values[istart]);
		}

		values.erase(values.begin() + index1, values.begin() + index2);
	}

	// 先让vector分配好内存
	values.insert(values.begin() + index1, osz, (PyObject*)NULL);
	for(int i = 0; i < osz; ++i)
	{
		PyObject* pyTemp = PySequence_GetItem(oterSeq, i);
		if(pyTemp == NULL)
		{
			PyErr_Format(PyExc_TypeError, "Sequence::seq_ass_slice::PySequence_GetItem %d is NULL.", i);
			PyErr_PrintEx(0);
		}
		
		values[index1 + i] = seq->createNewItemFromObj(pyTemp);

		if (pyTemp)
			Py_DECREF(pyTemp);
	}

	return 0;
}

//-------------------------------------------------------------------------------------
int Sequence::seq_contains(PyObject* self, PyObject* value)
{
	Sequence* seq = static_cast<Sequence*>(self);
	return seq->findFrom(0, value) >= 0;
}

//-------------------------------------------------------------------------------------
PyObject* Sequence::seq_inplace_concat(PyObject* self, PyObject* oterSeq)
{
	if (!PySequence_Check(oterSeq))
	{
		PyErr_SetString(PyExc_TypeError, "Sequence:Argument to += must be a sequence");
		PyErr_PrintEx(0);
		return NULL;
	}

	Sequence* seq = static_cast<Sequence*>(self);
	std::vector<PyObject*>& values = seq->getValues();
	
	int szA = (int)values.size();
	int szB = (int)PySequence_Size(oterSeq);

	if (szB == 0) 
	{
		Py_INCREF(seq);
		return seq;
	}

	// 检查类型是否正确
	for (int i = 0; i < szB; ++i)
	{
		PyObject * pyVal = PySequence_GetItem(oterSeq, i);
		bool ok = seq->isSameItemType(pyVal);
		Py_DECREF(pyVal);

		if (!ok)
		{
			PyErr_Format(PyExc_TypeError, "Array elements must be set to type %s (appending with +=)", "sss");
			PyErr_PrintEx(0);
			return NULL;
		}
	}
	
	// 先让vector分配好内存
	values.insert(values.end(), szB, (PyObject*)NULL);

	for (int i = 0; i < szB; ++i)
	{
		PyObject* pyTemp = PySequence_GetItem(oterSeq, i);
		if(pyTemp == NULL)
		{
			PyErr_Format(PyExc_TypeError, "Sequence::seq_inplace_concat::PySequence_GetItem %d is NULL.", i);
			PyErr_PrintEx(0);
		}

		values[szA + i] = pyTemp;
	}

	Py_INCREF(seq);
	return seq;
}

//-------------------------------------------------------------------------------------
PyObject* Sequence::seq_inplace_repeat(PyObject* self, Py_ssize_t n)
{
	Sequence* seq = static_cast<Sequence*>(self);
	if(n == 1) 
	{
		Py_INCREF(seq);
		return seq;
	}

	std::vector<PyObject*>& values = seq->getValues();
	int sz = (int)values.size();

	if (n <= 0)
	{
		for(size_t j = 0; j < values.size(); ++j)
		{
			Py_DECREF(values[j]);
		}

		values.clear();
	}
	else
	{
		values.insert(values.end(), (n - 1)*sz, (PyObject*)NULL);

		for(int i = 1; i < n; ++i)
		{
			for(int j = 0; j < sz; ++j)
			{
				Py_INCREF(&*values[j]);
				values[i * sz + j] = &*values[j];
			}
		}
	}

	Py_INCREF(seq);
	return seq;
}

//-------------------------------------------------------------------------------------

}
}
