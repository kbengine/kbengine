/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __KBENGINE_DEF_PROPERTY_H__
#define __KBENGINE_DEF_PROPERTY_H__
#include "cstdkbe/cstdkbe.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif
// common include
#include "array.hpp"
#include "fixeddict.hpp"
#include "datatype.hpp"
#include "cstdkbe/refcountable.hpp"
#include "cstdkbe/memorystream.hpp"
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>	
#include <map>	
#include <vector>
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
class PropertyDescription : public RefCountable
{
public:	
	PropertyDescription(std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
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
	std::string& getName(void){ return name_; };
	
	/** 获取字符串数据类别名 UINT32, BAG..*/
	std::string& getDataTypeName(void){ return dataTypeName_; }
	
	/** 属性的数字类别， 用于网络上传输识别 */
	uint32 getUType(void)const{ return utype_; }
	
	/** 设置这个属性为索引键 */
	void setIdentifier(bool isIdentifier){ isIdentifier_ = isIdentifier; }
	
	/** 设置这个属性在数据库中的长度 */
	void setDatabaseLength(uint32 databaseLength){ databaseLength_ = databaseLength; }
	
	/** 获取这个属性描述在def文件中被定义的默认值 */
	MemoryStream* getDefaultVal(void){ return defaultVal_; }
	
	/** 获得属性描述的总数量 */
	static uint32 getDescriptionCount(void){ return propertyDescriptionCount_; }
	
	/** 根据类型产生一个描述实例 */
	static PropertyDescription* createDescription(std::string& dataTypeName, std::string& name,
		uint32 flags, bool isPersistent, DataType* dataType, bool isIdentifier, uint32 databaseLength,
		std::string& defaultStr, uint8& detailLevel);
	
	/** 脚本请求设置这个属性的值 */
	virtual int onSetValue(PyObject* parentObj, PyObject* value);	
protected:	
	std::string		name_;											// 这个属性的名称
	std::string		dataTypeName_;									// 这个属性的字符串数据类别名
	uint32			flags_;											// 这个属性的一些标志  比如 cell_public
	bool			isPersistent_;									// 是否是一个存储到数据库的属性
	DataType*		dataType_;										// 这个属性的数据类别
	bool			isIdentifier_;									// 是否是一个索引键
	uint32			databaseLength_;								// 这个属性在数据库中的长度
	uint32			utype_;											// 这个属性的数字类别， 用于网络上传输识别
	MemoryStream*	defaultVal_;									// 这个属性的默认值
	int8			detailLevel_;									// 这个属性的lod详情级别 看common中的:属性的lod广播级别范围的定义
	static uint32	propertyDescriptionCount_;						// 所有的属性描述的数量	
};

class FixedDictDescription : public PropertyDescription
{
protected:	
public:	
	FixedDictDescription(std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
		DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel);
	virtual ~FixedDictDescription();
	
	/** 脚本请求设置这个属性的值 */
	int onSetValue(PyObject* parentObj, PyObject* value);		
};

class ArrayDescription : public PropertyDescription
{
protected:	
public:	
	ArrayDescription(std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
		DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel);
	virtual ~ArrayDescription();
	
	/** 脚本请求设置这个属性的值 */
	int onSetValue(PyObject* parentObj, PyObject* value);
};

class VectorDescription : public PropertyDescription
{
public:	
	VectorDescription(std::string dataTypeName, std::string name, uint32 flags, bool isPersistent, 
		DataType* dataType, bool isIdentifier, uint32 databaseLength, std::string defaultStr, uint8 detailLevel, uint8 elemCount);
	virtual ~VectorDescription();
	
	/** 脚本请求设置这个属性的值 */
	int onSetValue(PyObject* parentObj, PyObject* value);
protected:	
	uint8 elemCount_;
};

}
#endif
