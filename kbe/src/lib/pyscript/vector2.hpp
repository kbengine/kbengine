/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef _SCRIPT_VECTOR2_H
#define _SCRIPT_VECTOR2_H
#include "cstdkbe/cstdkbe.hpp"
#include "math/math.hpp"
#include "scriptobject.hpp"
#include "pickler.hpp"
	
namespace KBEngine{ namespace script{
	
class ScriptVector2 : public ScriptObject
{		
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(ScriptVector2, ScriptObject)
public:	
	static PySequenceMethods seqMethods;
	static PyNumberMethods numberMethods;
		
	ScriptVector2(Vector2 v);
	ScriptVector2(Vector2* v);
	ScriptVector2(float x, float y);
	virtual ~ScriptVector2();

	/** 获得对象的描述 */
	PyObject* tp_repr();

	/** 脚本模块对象从python中创建 */
	static PyObject* tp_new(PyTypeObject* type, PyObject* args, PyObject* kwds);

	/** seq相关操作 */
	static int seq_length(PyObject* self);
	static PyObject* seq_item(PyObject* self, int index);
	static PyObject* seq_slice(PyObject* self, int startIndex, int endIndex);
	static int seq_ass_item(PyObject* self, int index, PyObject* value);
	
	/** 加减乘除相关操作 */
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
	
	/** 暴漏一些方法 */
	static PyObject* pyDistTo(PyObject* self, PyObject* args);
	static PyObject* pyDistSqrTo(PyObject* self, PyObject* args);
	static PyObject* pyCross2D(PyObject* self, PyObject* args);
	static PyObject* pyScale(PyObject* self, PyObject* args);
	static PyObject* pyDot(PyObject* self, PyObject* args);
	static PyObject* pyNormalise(PyObject* self, PyObject* args);
	static PyObject* pyTuple(PyObject* self, PyObject* args);
	static PyObject* pyList(PyObject* self, PyObject* args);
	static PyObject* pySet(PyObject* self, PyObject* args);
	
	DECLARE_PY_GET_MOTHOD(pyGetVectorLength);
	DECLARE_PY_GET_MOTHOD(pyGetVectorLengthSquared);
	
	DECLARE_PY_GETSET_MOTHOD(pyGetX, pySetX);
	DECLARE_PY_GETSET_MOTHOD(pyGetY, pySetY);
	
	/** 支持pickler 方法 */
	static PyObject* __reduce_ex__(PyObject* self, PyObject* protocol);
	/** unpickle方法 */
	static PyObject* __unpickle__(PyObject* self, PyObject* args);

	/** 脚本被安装时被调用 */
	static void onInstallScript(PyObject* mod);
	
	int length(void){ return VECTOR_SIZE; }
	Vector2& getVector(void){ return *val_; }
	void setVector(const Vector2& v){ *val_ = v; }
	
	/** 检查某个python对象是否可以转换为本类型 */
	static bool check(PyObject* value, bool isPrintErr = true);
	
	/** 将某个经过check检查过的python对象转换为vector2 */
	static void convertPyObjectToVector2(Vector2& v, PyObject* obj);
private:
	Vector2*			val_;
	bool				isCopy_;
	bool				isReadOnly_;
	static const int 	VECTOR_SIZE;
} ;

}
}
#endif