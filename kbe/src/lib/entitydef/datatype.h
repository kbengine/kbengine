/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#ifndef KBE_DATA_TYPE_H
#define KBE_DATA_TYPE_H

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#pragma warning (disable : 4661)
#endif
#include "entitydef/common.h"	
#include "common/refcountable.h"
#include "common/memorystream.h"
#include "pyscript/scriptobject.h"
#include "pyscript/pickler.h"
#include "xml/xml.h"	


namespace KBEngine{

#define OUT_TYPE_ERROR(T)								\
{														\
	char err[] = {"must be set to a " T " type."};		\
	PyErr_SetString(PyExc_TypeError, err);				\
	PyErr_PrintEx(0);									\
}

class RefCountable;


class DataType : public RefCountable
{
public:	
	DataType(DATATYPE_UID did = 0);
	virtual ~DataType();	

	virtual bool isSameType(PyObject* pyValue) = 0;

	virtual void addToStream(MemoryStream* mstream, PyObject* pyValue) = 0;

	virtual PyObject* createFromStream(MemoryStream* mstream) = 0;

	static bool finalise();

	/**	
		����������pyobj�����ǵ�ǰ����ʱ���յ�ǰ���ʹ�����һ��obj
		ǰ���Ǽ�ʹ���PyObject���ǵ�ǰ���ͣ� ������ӵ��ת���Ĺ���
		��һ��python�ֵ�ת��Ϊһ���̶��ֵ䣬 �ֵ��е�key��ƥ��
	*/
	virtual PyObject* createNewItemFromObj(PyObject* pyobj)
	{
		Py_INCREF(pyobj);
		return pyobj;
	}
	
	virtual PyObject* createNewFromObj(PyObject* pyobj)
	{
		Py_INCREF(pyobj);
		return pyobj;
	}
		
	virtual bool initialize(XML* xml, TiXmlNode* node);

	virtual PyObject* parseDefaultStr(std::string defaultVal) = 0;

	virtual const char* getName(void) const = 0;

	INLINE DATATYPE_UID id() const;

	INLINE void aliasName(std::string aliasName);
	INLINE const char* aliasName(void) const;

	virtual DATATYPE type() const{ return DATA_TYPE_UNKONWN; }
protected:
	DATATYPE_UID id_;
	std::string aliasName_;
};

template <typename SPECIFY_TYPE>
class IntType : public DataType
{
protected:
public:	
	IntType(DATATYPE_UID did = 0);
	virtual ~IntType();	

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* parseDefaultStr(std::string defaultVal);
	const char* getName(void) const{ return "INT";}
	virtual DATATYPE type() const{ return DATA_TYPE_DIGIT; }
};

//-------------------------------------------------------------------------------------
template <>
inline const char* IntType<uint8>::getName(void) const
{
        return "UINT8";
}

//-------------------------------------------------------------------------------------
template <>
inline const char* IntType<uint16>::getName(void) const
{
        return "UINT16";
}

//-------------------------------------------------------------------------------------
template <>
inline const char* IntType<uint32>::getName(void) const
{
        return "UINT32";
}

//-------------------------------------------------------------------------------------
template <>
inline const char* IntType<int8>::getName(void) const
{
        return "INT8";
}

//-------------------------------------------------------------------------------------
template <>
inline const char* IntType<int16>::getName(void) const
{
        return "INT16";
}

//-------------------------------------------------------------------------------------
template <>
inline const char* IntType<int32>::getName(void) const
{
        return "INT32";
}

//-------------------------------------------------------------------------------------
template <>
inline PyObject* IntType<uint8>::parseDefaultStr(std::string defaultVal)
{
	uint32 i = 0;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> i;
	}
	
	PyObject* pyval = PyLong_FromUnsignedLong((uint8)i);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		PyErr_Format(PyExc_TypeError, "UINT8Type::parseDefaultStr: defaultVal(%s) is error! val=[%s]", 
			pyval != NULL ? pyval->ob_type->tp_name : "NULL", defaultVal.c_str());

		PyErr_PrintEx(0);

		S_RELEASE(pyval);
		return PyLong_FromUnsignedLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
template <>
inline PyObject* IntType<uint16>::parseDefaultStr(std::string defaultVal)
{
	uint32 i = 0;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> i;
	}

	PyObject* pyval = PyLong_FromUnsignedLong((uint16)i);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		PyErr_Format(PyExc_TypeError, "UINT16Type::parseDefaultStr: defaultVal(%s) is error! val=[%s]", 
			pyval != NULL ? pyval->ob_type->tp_name : "NULL", defaultVal.c_str());

		PyErr_PrintEx(0);

		S_RELEASE(pyval);
		return PyLong_FromUnsignedLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
template <>
inline PyObject* IntType<uint32>::parseDefaultStr(std::string defaultVal)
{
	uint32 i = 0;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> i;
	}

	PyObject* pyval = PyLong_FromUnsignedLong(i);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		PyErr_Format(PyExc_TypeError, "UINT32Type::parseDefaultStr: defaultVal(%s) is error! val=[%s]", 
			pyval != NULL ? pyval->ob_type->tp_name : "NULL", defaultVal.c_str());

		PyErr_PrintEx(0);

		S_RELEASE(pyval);
		return PyLong_FromUnsignedLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
template <>
inline PyObject* IntType<int8>::parseDefaultStr(std::string defaultVal)
{
	int32 i = 0;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> i;
	}

	PyObject* pyval = PyLong_FromLong((int8)i);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		PyErr_Format(PyExc_TypeError, "INT8Type::parseDefaultStr: defaultVal(%s) is error! val=[%s]", 
			pyval != NULL ? pyval->ob_type->tp_name : "NULL", defaultVal.c_str());

		PyErr_PrintEx(0);

		S_RELEASE(pyval);
		return PyLong_FromLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
template <>
inline PyObject* IntType<int16>::parseDefaultStr(std::string defaultVal)
{
	int32 i = 0;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> i;
	}

	PyObject* pyval = PyLong_FromLong((int16)i);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		PyErr_Format(PyExc_TypeError, "INT16Type::parseDefaultStr: defaultVal(%s) is error! val=[%s]", 
			pyval != NULL ? pyval->ob_type->tp_name : "NULL", defaultVal.c_str());

		PyErr_PrintEx(0);

		S_RELEASE(pyval);
		return PyLong_FromLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------
template <>
inline PyObject* IntType<int32>::parseDefaultStr(std::string defaultVal)
{
	int32 i = 0;
	if(!defaultVal.empty())
	{
		std::stringstream stream;
		stream << defaultVal;
		stream >> i;
	}

	PyObject* pyval = PyLong_FromLong(i);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		PyErr_Format(PyExc_TypeError, "INT32Type::parseDefaultStr: defaultVal(%s) is error! val=[%s]", 
			pyval != NULL ? pyval->ob_type->tp_name : "NULL", defaultVal.c_str());

		PyErr_PrintEx(0);

		S_RELEASE(pyval);
		return PyLong_FromLong(0);
	}

	return pyval;
}

//-------------------------------------------------------------------------------------

class UInt64Type : public DataType
{
protected:
public:	
	UInt64Type(DATATYPE_UID did = 0);
	virtual ~UInt64Type();	

	bool isSameType(PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);

	PyObject* createFromStream(MemoryStream* mstream);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "UINT64";}

	virtual DATATYPE type() const{ return DATA_TYPE_DIGIT; }
};

class UInt32Type : public DataType
{
protected:
public:	
	UInt32Type(DATATYPE_UID did = 0);
	virtual ~UInt32Type();	

	bool isSameType(PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);

	PyObject* createFromStream(MemoryStream* mstream);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "UINT32";}

	virtual DATATYPE type() const{ return DATA_TYPE_DIGIT; }
};

class Int64Type : public DataType
{
protected:
public:	
	Int64Type(DATATYPE_UID did = 0);
	virtual ~Int64Type();	

	bool isSameType(PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);

	PyObject* createFromStream(MemoryStream* mstream);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "INT64";}

	virtual DATATYPE type() const{ return DATA_TYPE_DIGIT; }
};

class FloatType : public DataType
{
protected:
public:	
	FloatType(DATATYPE_UID did = 0);
	virtual ~FloatType();	

	bool isSameType(PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);

	PyObject* createFromStream(MemoryStream* mstream);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "FLOAT";}

	virtual DATATYPE type() const{ return DATA_TYPE_DIGIT; }
};

class DoubleType : public DataType
{
protected:
public:	
	DoubleType(DATATYPE_UID did = 0);
	virtual ~DoubleType();	

	bool isSameType(PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);

	PyObject* createFromStream(MemoryStream* mstream);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "DOUBLE";}

	virtual DATATYPE type() const{ return DATA_TYPE_DIGIT; }
};

class VectorType : public DataType
{
public:	
	VectorType(uint32 elemCount, DATATYPE_UID did = 0);
	virtual ~VectorType();	

	bool isSameType(PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);

	PyObject* createFromStream(MemoryStream* mstream);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return name_.c_str();}

	virtual DATATYPE type() const{ return DATA_TYPE_VECTOR; }
protected:
	std::string name_;
	ArraySize elemCount_;
};

class StringType : public DataType
{
protected:
public:	
	StringType(DATATYPE_UID did = 0);
	virtual ~StringType();	

	bool isSameType(PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);

	PyObject* createFromStream(MemoryStream* mstream);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "STRING";}

	virtual DATATYPE type() const{ return DATA_TYPE_STRING; }
};

class UnicodeType : public DataType
{
protected:
public:	
	UnicodeType(DATATYPE_UID did = 0);
	virtual ~UnicodeType();	

	bool isSameType(PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);

	PyObject* createFromStream(MemoryStream* mstream);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "UNICODE";}

	virtual DATATYPE type() const{ return DATA_TYPE_UNICODE; }
};

class PythonType : public DataType
{
protected:
public:	
	PythonType(DATATYPE_UID did = 0);
	virtual ~PythonType();	

	virtual bool isSameType(PyObject* pyValue);
	virtual void addToStream(MemoryStream* mstream, PyObject* pyValue);

	virtual PyObject* createFromStream(MemoryStream* mstream);

	virtual PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "PYTHON";}

	virtual DATATYPE type() const{ return DATA_TYPE_PYTHON; }
};

class PyDictType : public PythonType
{
protected:
public:	
	PyDictType(DATATYPE_UID did = 0);
	virtual ~PyDictType();	

	bool isSameType(PyObject* pyValue);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "PY_DICT";}

	virtual DATATYPE type() const{ return DATA_TYPE_PYDICT; }
};

class PyTupleType : public PythonType
{
protected:
public:	
	PyTupleType(DATATYPE_UID did = 0);
	virtual ~PyTupleType();	

	bool isSameType(PyObject* pyValue);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "PY_TUPLE";}

	virtual DATATYPE type() const{ return DATA_TYPE_PYTUPLE; }
};

class PyListType : public PythonType
{
protected:
public:	
	PyListType(DATATYPE_UID did = 0);
	virtual ~PyListType();	

	bool isSameType(PyObject* pyValue);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "PY_LIST";}

	virtual DATATYPE type() const{ return DATA_TYPE_PYLIST; }
};

class BlobType : public DataType
{
protected:
public:	
	BlobType(DATATYPE_UID did = 0);
	virtual ~BlobType();	

	bool isSameType(PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);

	PyObject* createFromStream(MemoryStream* mstream);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "BLOB";}

	virtual DATATYPE type() const{ return DATA_TYPE_BLOB; }
};

class MailboxType : public DataType
{
protected:
public:	
	MailboxType(DATATYPE_UID did = 0);
	virtual ~MailboxType();	

	bool isSameType(PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);

	PyObject* createFromStream(MemoryStream* mstream);

	PyObject* parseDefaultStr(std::string defaultVal);

	const char* getName(void) const{ return "MAILBOX";}

	virtual DATATYPE type() const{ return DATA_TYPE_MAILBOX; }
};

class FixedArrayType : public DataType
{
public:	
	FixedArrayType(DATATYPE_UID did = 0);
	virtual ~FixedArrayType();	
	
	DataType* getDataType(){ return dataType_; }

	bool isSameType(PyObject* pyValue);
	bool isSameItemType(PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	void addToStreamEx(MemoryStream* mstream, PyObject* pyValue, bool onlyPersistents);

	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createFromStreamEx(MemoryStream* mstream, bool onlyPersistents);

	PyObject* parseDefaultStr(std::string defaultVal);

	bool initialize(XML* xml, TiXmlNode* node);

	const char* getName(void) const{ return "ARRAY";}

	/**	
		����������pyobj�����ǵ�ǰ����ʱ���յ�ǰ���ʹ�����һ��obj
		ǰ���Ǽ�ʹ���PyObject���ǵ�ǰ���ͣ� ������ӵ��ת���Ĺ���
		��һ��python�ֵ�ת��Ϊһ���̶��ֵ䣬 �ֵ��е�key��ƥ��
	*/
	virtual PyObject* createNewItemFromObj(PyObject* pyobj);
	virtual PyObject* createNewFromObj(PyObject* pyobj);

	virtual DATATYPE type() const{ return DATA_TYPE_FIXEDARRAY; }
protected:
	DataType*			dataType_;		// �����������������
};

class FixedDictType : public DataType
{
public:
	struct DictItemDataType
	{
		DataType* dataType;

		// ��Ϊһ�����������alias�пɶ�dict�е�ĳ����ָ���Ƿ�־û�
		bool persistent;

		// ������������ݿ��еĳ���
		uint32 databaseLength;
	};

	typedef KBEShared_ptr< DictItemDataType > DictItemDataTypePtr;
	typedef std::vector< std::pair< std::string, DictItemDataTypePtr > > FIXEDDICT_KEYTYPE_MAP;
public:	
	FixedDictType(DATATYPE_UID did = 0);
	virtual ~FixedDictType();
	
	/** 
		�������̶��ֵ��key��� 
	*/	
	FIXEDDICT_KEYTYPE_MAP& getKeyTypes(void){ return keyTypes_; }

	const char* getName(void) const{ return "FIXED_DICT";}

	bool isSameType(PyObject* pyValue);
	DataType* isSameItemType(const char* keyName, PyObject* pyValue);

	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	void addToStreamEx(MemoryStream* mstream, PyObject* pyValue, bool onlyPersistents);

	PyObject* createFromStream(MemoryStream* mstream);
	PyObject* createFromStreamEx(MemoryStream* mstream, bool onlyPersistents);

	PyObject* parseDefaultStr(std::string defaultVal);
	bool initialize(XML* xml, TiXmlNode* node);
	
	/**	
		����������pyobj�����ǵ�ǰ����ʱ���յ�ǰ���ʹ�����һ��obj
		ǰ���Ǽ�ʹ���PyObject���ǵ�ǰ���ͣ� ������ӵ��ת���Ĺ���
		��һ��python�ֵ�ת��Ϊһ���̶��ֵ䣬 �ֵ��е�key��ƥ��
	*/
	virtual PyObject* createNewItemFromObj(const char* keyName, PyObject* pyobj);
	virtual PyObject* createNewFromObj(PyObject* pyobj);

	/** 
		��ù̶��ֵ����е�key���� 
	*/
	std::string getKeyNames(void);

	/** 
		���debug��Ϣ�����ع̶��ֵ����е�key���ƺ�����
	*/
	std::string debugInfos(void);

	/** 
		����implģ��
	*/
	bool loadImplModule(std::string moduleName);

	/** 
		impl���ʵ��
	*/
	PyObject* impl_createObjFromDict(PyObject* dictData);
	PyObject* impl_getDictFromObj(PyObject* pyobj);
	bool impl_isSameType(PyObject* pyobj);

	bool hasImpl() const { return implObj_ != NULL; }

	virtual DATATYPE type() const{ return DATA_TYPE_FIXEDDICT; }

	std::string& moduleName(){ return moduleName_; }
	
protected:
	// ����̶��ֵ���ĸ���key������
	FIXEDDICT_KEYTYPE_MAP			keyTypes_;				

	// ʵ�ֽű�ģ��
	PyObject*						implObj_;				

	PyObject*						pycreateObjFromDict_;
	PyObject*						pygetDictFromObj_;

	PyObject*						pyisSameType_;

	std::string						moduleName_;		
};


template class IntType<uint8>;
template class IntType<uint16>;
template class IntType<int8>;
template class IntType<int16>;
template class IntType<int32>;

//-------------------------------------------------------------------------------------
template <typename SPECIFY_TYPE>
IntType<SPECIFY_TYPE>::IntType(DATATYPE_UID did):
DataType(did)
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
	if(pyValue == NULL)
	{
		OUT_TYPE_ERROR("INT");
		return false;
	}

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
		ERROR_MSG(fmt::format("IntType::isSameType:{} is out of range (currVal = {}).\n",
			ival, int(val)));
		
		return false;
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
template <typename SPECIFY_TYPE>
void IntType<SPECIFY_TYPE>::addToStream(MemoryStream* mstream, 
	PyObject* pyValue)
{
	SPECIFY_TYPE v = (SPECIFY_TYPE)PyLong_AsLong(pyValue);
	
	if(PyErr_Occurred())
	{
		PyErr_Clear();
		
		v = (SPECIFY_TYPE)PyLong_AsUnsignedLong(pyValue);
		
		if(PyErr_Occurred())
		{
			PyErr_Clear();
			PyErr_Format(PyExc_TypeError, "IntType::addToStream: pyValue(%s) is error!", 
				(pyValue == NULL) ? "NULL": pyValue->ob_type->tp_name);

			PyErr_PrintEx(0);

			v = 0;
		}
	}
			
	(*mstream) << v;
}

//-------------------------------------------------------------------------------------
template <typename SPECIFY_TYPE>
PyObject* IntType<SPECIFY_TYPE>::createFromStream(MemoryStream* mstream)
{
	SPECIFY_TYPE val = 0;
	if(mstream)
		(*mstream) >> val;

	PyObject* pyval = PyLong_FromLong(val);

	if (PyErr_Occurred()) 
	{
		PyErr_Clear();
		S_RELEASE(pyval);
		
		pyval = PyLong_FromUnsignedLong(val);
		
		if (PyErr_Occurred()) 
		{
			PyErr_Format(PyExc_TypeError, "IntType::createFromStream: errval=%d, default return is 0", val);
			PyErr_PrintEx(0);
			S_RELEASE(pyval);
			return PyLong_FromLong(0);
		}
	}

	return pyval;
}

}

#ifdef CODE_INLINE
#include "datatype.inl"
#endif
#endif // KBE_DATA_TYPE_H

