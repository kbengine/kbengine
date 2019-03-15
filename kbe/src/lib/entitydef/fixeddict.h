// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
	/** ���໯ ��һЩpy�������������� */
	INSTANCE_SCRIPT_HREADER(FixedDict, Map)
public:	
	static PyMappingMethods mappingMethods;
	static PySequenceMethods mappingSequenceMethods;

	FixedDict(DataType* dataType);
	FixedDict(DataType* dataType, bool isPersistentsStream);

	virtual ~FixedDict();

	DataType* getDataType(void){ return _dataType; }

	/** 
		֧��pickler ���� 
	*/
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol);

	/** 
		unpickle���� 
	*/
	static PyObject* __unpickle__(PyObject* self, PyObject* args);
	
	/** 
		�ű�����װʱ������ 
	*/
	static void onInstallScript(PyObject* mod);
	
	/** 
		map����������� 
	*/
	static PyObject* mp_subscript(PyObject* self, PyObject* key);

	static int mp_ass_subscript(PyObject* self, PyObject* key, 
		PyObject* value);

	static int mp_length(PyObject* self);

	/** 
		��ʼ���̶��ֵ�
	*/
	void initialize(std::string strDictInitData);
	void initialize(PyObject* pyDictInitData);
	void initialize(MemoryStream* streamInitData, bool isPersistentsStream);

	/** 
		������ݸı� 
	*/
	bool checkDataChanged(const char* keyName, 
		PyObject* value,
		bool isDelete = false);
	
	/**
		�����ֵ����ݵ��Լ��������� 
	*/
	PyObject* update(PyObject* args);

	/** 
		��ö�������� 
	*/
	PyObject* tp_repr();
	PyObject* tp_str();

	bool isSameType(PyObject* pyValue);

protected:
	FixedDictType* _dataType;
} ;

}
#endif // KBE_FIXED_DICT_H
