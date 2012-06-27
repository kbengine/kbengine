/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __DATA_TYPE_H__
#define __DATA_TYPE_H__

// common include
#include "cstdkbe/cstdkbe.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#pragma warning (disable : 4661)
#endif
#include "cstdkbe/refcountable.hpp"
#include "cstdkbe/memorystream.hpp"
#include "pyscript/scriptobject.hpp"
#include "pyscript/pickler.hpp"
#include "xmlplus/xmlplus.hpp"	
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <map>	
#include <vector>
#include <sstream>
#include <string>
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
#define LIB_DLLAPI  __declspec(dllexport)

#ifdef __cplusplus  
extern "C" {  
#endif  

#ifdef __cplusplus  
}
#endif 

namespace KBEngine{

#define OUT_TYPE_ERROR(T)								\
{														\
	char err[] = {"must be set to a " T " type."};	\
	PyErr_SetString(PyExc_TypeError, err);				\
	PyErr_PrintEx(0);									\
}

class RefCountable;
class DataType : public RefCountable
{
protected:
public:	
	DataType();
	virtual ~DataType();	
	virtual bool isSameType(PyObject* pyValue) = 0;
	virtual void addToStream(MemoryStream* mstream, PyObject* pyValue) = 0;
	virtual PyObject* createFromStream(MemoryStream* mstream) = 0;
	virtual bool initialize(XmlPlus* xmlplus, TiXmlNode* node);
	virtual PyObject* createObject(MemoryStream* defaultVal) = 0;
	virtual MemoryStream* parseDefaultStr(std::string defaultVal) = 0;
	virtual std::string getName(void) = 0;
};

template <typename SPECIFY_TYPE>
class IntType : public DataType
{
protected:
public:	
	IntType();
	virtual ~IntType();	

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createObject(MemoryStream* defaultVal);
	MemoryStream* parseDefaultStr(std::string defaultVal);
	std::string getName(void){ return "INT";}
};

//-------------------------------------------------------------------------------------
template <typename SPECIFY_TYPE>
IntType<SPECIFY_TYPE>::IntType()
{
}

//-------------------------------------------------------------------------------------
template <typename SPECIFY_TYPE>
IntType<SPECIFY_TYPE>::~IntType()
{
}

//-------------------------------------------------------------------------------------
template <typename SPECIFY_TYPE>
bool IntType<SPECIFY_TYPE>::isSameType(PyObject* pyValue)
{
	int ival = 0;
	if(PyLong_Check(pyValue))
	{
		ival = (int)PyLong_AsLong(pyValue);
		if(PyErr_Occurred())
		{
			PyErr_Clear();
			ival = (int)PyLong_AsUnsignedLong(pyValue);
			if (PyErr_Occurred())
			{
				OUT_TYPE_ERROR("INT");
				return false;
			}
		}
	}
	else
	{
		OUT_TYPE_ERROR("INT");
		return false;
	}

	SPECIFY_TYPE val = (SPECIFY_TYPE)ival;
	if(ival != int(val))
	{
		ERROR_MSG("IntType::isSameType:%d is out of range (currVal = %d).\n", ival, int(val));
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------
template <typename SPECIFY_TYPE>
PyObject* IntType<SPECIFY_TYPE>::createObject(MemoryStream* defaultVal)
{
	SPECIFY_TYPE val = 0;
	if(defaultVal)
		(*defaultVal) >> val;

	return PyLong_FromLong(val);
}

//-------------------------------------------------------------------------------------
template <typename SPECIFY_TYPE>
MemoryStream* IntType<SPECIFY_TYPE>::parseDefaultStr(std::string defaultVal)
{
	MemoryStream* bs = NULL;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		SPECIFY_TYPE i;
		stream >> i;
		bs = new MemoryStream();
		(*bs) << i;
	}

	return bs;
}

//-------------------------------------------------------------------------------------
template <typename SPECIFY_TYPE>
void IntType<SPECIFY_TYPE>::addToStream(MemoryStream* mstream, PyObject* pyValue)
{
	(*mstream) << (SPECIFY_TYPE)PyLong_AsLong(pyValue);
}

//-------------------------------------------------------------------------------------
template <typename SPECIFY_TYPE>
PyObject* IntType<SPECIFY_TYPE>::createFromStream(MemoryStream* mstream)
{
	return createObject(mstream);
}

class UInt64Type : public DataType
{
protected:
public:	
	UInt64Type();
	virtual ~UInt64Type();	

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createObject(MemoryStream* defaultVal);
	MemoryStream* parseDefaultStr(std::string defaultVal);
	std::string getName(void){ return "UINT64";}
};

class UInt32Type : public DataType
{
protected:
public:	
	UInt32Type();
	virtual ~UInt32Type();	

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createObject(MemoryStream* defaultVal);
	MemoryStream* parseDefaultStr(std::string defaultVal);
	std::string getName(void){ return "UINT32";}
};

class Int64Type : public DataType
{
protected:
public:	
	Int64Type();
	virtual ~Int64Type();	

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createObject(MemoryStream* defaultVal);
	MemoryStream* parseDefaultStr(std::string defaultVal);
	std::string getName(void){ return "INT64";}
};

class FloatType : public DataType
{
protected:
public:	
	FloatType();
	virtual ~FloatType();	

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createObject(MemoryStream* defaultVal);
	MemoryStream* parseDefaultStr(std::string defaultVal);
	std::string getName(void){ return "FLOAT";}
};

class VectorType : public DataType
{
protected:
	int elemCount_;
public:	
	VectorType(int elemCount);
	virtual ~VectorType();	

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createObject(MemoryStream* defaultVal);
	MemoryStream* parseDefaultStr(std::string defaultVal);
	std::string getName(void){ return "VECTOR3";}
};

class StringType : public DataType
{
protected:
public:	
	StringType();
	virtual ~StringType();	

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createObject(MemoryStream* defaultVal);
	MemoryStream* parseDefaultStr(std::string defaultVal);
	std::string getName(void){ return "STRING";}
};

class PythonType : public DataType
{
protected:
public:	
	PythonType();
	virtual ~PythonType();	

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createObject(MemoryStream* defaultVal);
	MemoryStream* parseDefaultStr(std::string defaultVal);
	std::string getName(void){ return "PYTHON";}
};

class MailboxType : public DataType
{
protected:
public:	
	MailboxType();
	virtual ~MailboxType();	

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createObject(MemoryStream* defaultVal);
	MemoryStream* parseDefaultStr(std::string defaultVal);
	std::string getName(void){ return "MAILBOX";}
};

class ArrayType : public DataType
{
protected:
	DataType*			dataType_;		// 这个数组所处理的类别
public:	
	ArrayType();
	virtual ~ArrayType();	

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createObject(MemoryStream* defaultVal);
	MemoryStream* parseDefaultStr(std::string defaultVal);
	bool initialize(XmlPlus* xmlplus, TiXmlNode* node);
	std::string getName(void){ return "ARRAY";}
};

class FixedDictType : public DataType
{
public:
	typedef std::map<std::string, DataType*> FIXEDDICT_KEYTYPE_MAP;
protected:
	FIXEDDICT_KEYTYPE_MAP			keyTypes_;		// 这个固定字典里的各个key的类型
public:	
	FixedDictType();
	virtual ~FixedDictType();
	
	/** 获得这个固定字典的key类别 */	
	FIXEDDICT_KEYTYPE_MAP& getKeyTypes(void){ return keyTypes_; }
	std::string getName(void){ return "FIXED_DICT";}
	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createObject(MemoryStream* defaultVal);
	MemoryStream* parseDefaultStr(std::string defaultVal);
	bool initialize(XmlPlus* xmlplus, TiXmlNode* node);
	
	/** 获得固定字典所有的key名称 */
	std::string getKeyNames(void);
};


template class IntType<uint8>;
template class IntType<uint16>;
template class IntType<int8>;
template class IntType<int16>;
template class IntType<int32>;


}
#endif
