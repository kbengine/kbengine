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


#ifndef __KBENGINE_DEF_PROPERTY_H__
#define __KBENGINE_DEF_PROPERTY_H__
#include "cstdkbe/cstdkbe.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif
// common include
#include "fixedarray.hpp"
#include "fixeddict.hpp"
#include "datatype.hpp"
#include "cstdkbe/refcountable.hpp"
#include "cstdkbe/memorystream.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine{

class RefCountable;
class PropertyDescription : public RefCountable
{
public:	
	PropertyDescription(ENTITY_PROPERTY_UID utype, std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
		DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel);
	virtual ~PropertyDescription();
	
	/** 获取这个属性的detailLevel */
	int8 getDetailLevel(void)const{ return detailLevel_; }
	
	/** 是否是一个保存到数据库中的属性 */
	bool isPersistent(void)const{ return isPersistent_; };
	
	/** 获取这个属性的数据类别 */
	DataType* getDataType(void)const{ return dataType_; };
	
	/** 获取属性的标志 cell_public等 */
	const uint32& getFlags(void)const{ return flags_; };
	
	/** 获取属性名称 */
	const char* getName(void)const{ return name_.c_str(); };
	
	/** 获取字符串数据类别名 UINT32, BAG..*/
	const char* getDataTypeName(void)const{ return dataTypeName_.c_str(); }
	
	/** 属性的数字类别， 用于网络上传输识别 */
	ENTITY_PROPERTY_UID getUType(void)const{ return utype_; }
	
	/** 设置这个属性为索引键 */
	void setIdentifier(bool isIdentifier){ isIdentifier_ = isIdentifier; }
	
	/** 设置这个属性在数据库中的长度 */
	void setDatabaseLength(uint32 databaseLength){ databaseLength_ = databaseLength; }
	uint32 getDatabaseLength()const { return databaseLength_; }

	/** 获取这个属性描述在def文件中被定义的默认值 */
	PyObject* newDefaultVal(void);
	
	/** 获得属性描述的总数量 */
	static uint32 getDescriptionCount(void){ return propertyDescriptionCount_; }
	
	/** 根据类型产生一个描述实例 */
	static PropertyDescription* createDescription(ENTITY_PROPERTY_UID utype, std::string& dataTypeName, std::string& name,
		uint32 flags, bool isPersistent, DataType* dataType, bool isIdentifier, uint32 databaseLength,
		std::string& defaultStr, uint8 detailLevel);
	
	/** 脚本请求设置这个属性的值 */
	virtual int onSetValue(PyObject* parentObj, PyObject* value);	
protected:	
	std::string					name_;											// 这个属性的名称
	std::string					dataTypeName_;									// 这个属性的字符串数据类别名
	uint32						flags_;											// 这个属性的一些标志  比如 cell_public
	bool						isPersistent_;									// 是否是一个存储到数据库的属性
	DataType*					dataType_;										// 这个属性的数据类别
	bool						isIdentifier_;									// 是否是一个索引键
	uint32						databaseLength_;								// 这个属性在数据库中的长度
	ENTITY_PROPERTY_UID			utype_;											// 这个属性的数字类别， 用于网络上传输识别
	PyObject*					defaultVal_;									// 这个属性的默认值
	int8						detailLevel_;									// 这个属性的lod详情级别 看common中的:属性的lod广播级别范围的定义
	static uint32				propertyDescriptionCount_;						// 所有的属性描述的数量	
};

class FixedDictDescription : public PropertyDescription
{
protected:	
public:	
	FixedDictDescription(ENTITY_PROPERTY_UID utype, std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
		DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel);
	virtual ~FixedDictDescription();
	
	/** 脚本请求设置这个属性的值 */
	int onSetValue(PyObject* parentObj, PyObject* value);		
};

class ArrayDescription : public PropertyDescription
{
protected:	
public:	
	ArrayDescription(ENTITY_PROPERTY_UID utype, std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
		DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel);
	virtual ~ArrayDescription();
	
	/** 脚本请求设置这个属性的值 */
	int onSetValue(PyObject* parentObj, PyObject* value);
};

class VectorDescription : public PropertyDescription
{
public:	
	VectorDescription(ENTITY_PROPERTY_UID utype, std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
		DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel, uint8 elemCount);
	virtual ~VectorDescription();
	
	/** 脚本请求设置这个属性的值 */
	int onSetValue(PyObject* parentObj, PyObject* value);
protected:	
	uint8 elemCount_;
};

}
#endif
