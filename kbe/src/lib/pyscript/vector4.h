// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef _SCRIPT_VECTOR4_H
#define _SCRIPT_VECTOR4_H
#include "common/common.h"
#include "math/math.h"
#include "scriptobject.h"
#include "pickler.h"
	
namespace KBEngine{ namespace script{
	
class ScriptVector4 : public ScriptObject
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(ScriptVector4, ScriptObject)
public:	
	static PySequenceMethods seqMethods;
	static PyNumberMethods numberMethods;
		
	ScriptVector4(Vector4 v);
	ScriptVector4(Vector4* v);
	ScriptVector4(float x, float y, float z, float w);
	virtual ~ScriptVector4();

	/** 
		获得对象的描述 
	*/
	PyObject* tp_repr();
	PyObject* tp_str();

	/** 
		脚本模块对象从python中创建 
	*/
	static PyObject* tp_new(PyTypeObject* type, PyObject* args, PyObject* kwds);

	/** 
		seq相关操作 
	*/
	static Py_ssize_t seq_length(PyObject* self);
	static PyObject* seq_item(PyObject* self, Py_ssize_t index);
	static PyObject* seq_slice(PyObject* self, Py_ssize_t startIndex, Py_ssize_t endIndex);
	static int seq_ass_item(PyObject* self, Py_ssize_t index, PyObject* value);
	
	/** 
		加减乘除相关操作 
	*/
	static PyObject* py_add(PyObject *a, PyObject *b);
	static PyObject* py_subtract(PyObject *a, PyObject *b);
	static PyObject* py_multiply(PyObject *a, PyObject *b);
	static PyObject* py_divide(PyObject *a, PyObject *b);
	
	static PyObject* py_negative(PyObject *self);
	static PyObject* py_positive(PyObject *self);
	
	static int py_nonzero(PyObject *self);
	
	static PyObject* py_inplace_add(PyObject *self, PyObject *b);
	static PyObject* py_inplace_subtract(PyObject *self, PyObject *b);
	static PyObject* py_inplace_multiply(PyObject *self, PyObject *b);
	static PyObject* py_inplace_divide(PyObject *self, PyObject *b);
	
	/** 
		暴漏一些方法 
	*/
	static PyObject* __py_pyDistTo(PyObject* self, PyObject* args);
	static PyObject* __py_pyDistSqrTo(PyObject* self, PyObject* args);
	static PyObject* __py_pyScale(PyObject* self, PyObject* args);
	static PyObject* __py_pyDot(PyObject* self, PyObject* args);
	static PyObject* __py_pyNormalise(PyObject* self, PyObject* args);
	static PyObject* __py_pyTuple(PyObject* self, PyObject* args);
	static PyObject* __py_pyList(PyObject* self, PyObject* args);
	static PyObject* __py_pySet(PyObject* self, PyObject* args);

	DECLARE_PY_GET_MOTHOD(pyGetVectorLength);
	DECLARE_PY_GET_MOTHOD(pyGetVectorLengthSquared);

	DECLARE_PY_GETSET_MOTHOD(pyGetX, pySetX);
	DECLARE_PY_GETSET_MOTHOD(pyGetY, pySetY);
	DECLARE_PY_GETSET_MOTHOD(pyGetZ, pySetZ);
	DECLARE_PY_GETSET_MOTHOD(pyGetW, pySetW);
	
	/** 
		支持pickler 方法 
	*/
	static PyObject* __reduce_ex__(PyObject* self, PyObject* protocol);

	/** 
		unpickle方法 
	*/
	static PyObject* __unpickle__(PyObject* self, PyObject* args);

	/** 
		脚本被安装时被调用 
	*/
	static void onInstallScript(PyObject* mod);
	
	int length(void){ return VECTOR_SIZE; }
	Vector4& getVector(void){ return *val_; }
	void setVector(const Vector4& v){ *val_ = v; }
	
	/** 
		检查某个python对象是否可以转换为本类型 
	*/
	static bool check(PyObject* value, bool isPrintErr = true);
	
	/** 
		将某个经过check检查过的python对象转换为vector4 
	*/
	static bool convertPyObjectToVector4(Vector4& v, PyObject* obj);

private:
	Vector4*			val_;
	bool				isCopy_;
	bool				isReadOnly_;
	static const int 	VECTOR_SIZE;
} ;

}
}
#endif
