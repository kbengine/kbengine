/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
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
#include "memorystream.hpp"
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
	int m_elemCount_;
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
	DataType*			m_dataType_;		// 这个数组所处理的类别
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
	FIXEDDICT_KEYTYPE_MAP			m_keyTypes_;		// 这个固定字典里的各个key的类型
public:	
	FixedDictType();
	virtual ~FixedDictType();
	
	/** 获得这个固定字典的key类别 */	
	FIXEDDICT_KEYTYPE_MAP& getKeyTypes(void){ return m_keyTypes_; }
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

#ifdef _LIB
template class IntType<uint8>;
template class IntType<uint16>;
template class IntType<int8>;
template class IntType<int16>;
template class IntType<int32>;
#endif

}
#endif
