// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBENGINE_DEF_METHOD_H
#define KBENGINE_DEF_METHOD_H

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif

#include "datatype.h"
#include "datatypes.h"
#include "helper/debug_helper.h"
#include "network/packet.h"
#include "entitycallabstract.h"
#include "pyscript/scriptobject.h"	


namespace KBEngine{

class MethodDescription
{
public:
	// 暴露方法的类型
	enum EXPOSED_TYPE
	{
		// 默认，非暴露方法
		NO_EXPOSED = 0,

		// 默认，脚本方法可以不加调用者参数
		EXPOSED = 1,

		// 脚本方法第一个参数为调用者ID，提供脚本检查调用者合法性
		EXPOSED_AND_CALLER_CHECK = 2
	};

public:	
	MethodDescription(ENTITY_METHOD_UID utype, COMPONENT_ID domain,
		std::string name, 
		bool isExposed = false);

	virtual ~MethodDescription();
	
	INLINE const char* getName(void) const;

	INLINE ENTITY_METHOD_UID getUType(void) const;
	INLINE void setUType(ENTITY_METHOD_UID muid);

	static uint32 getDescriptionCount(void){ return methodDescriptionCount_; }
	static void resetDescriptionCount(void){ methodDescriptionCount_ = 0; }

	INLINE EXPOSED_TYPE isExposed(void) const;

	void setExposed(EXPOSED_TYPE type = EXPOSED);

	bool pushArgType(DataType* dataType);

	INLINE std::vector<DataType*>& getArgTypes(void);

	size_t getArgSize(void);
	
	/** 
		检查一个call是否合法 
	*/
	bool checkArgs(PyObject* args);		
	
	/** 
		将每个参数打包添加到流， 
		这个流里包含的信息是这个方法在脚本被调用时里传入的参数 
	*/
	void addToStream(MemoryStream* mstream, PyObject* args);

	/** 
		将一个call流解包 并返回一个PyObject类型的args 
	*/
	PyObject* createFromStream(MemoryStream* mstream);
	
	/** 
		呼叫一个方法 
	*/
	PyObject* call(PyObject* func, PyObject* args);	

	INLINE COMPONENT_ID domain() const;

	INLINE bool isClient() const;
	INLINE bool isCell() const;
	INLINE bool isBase() const;

	/** 
		别名id， 当暴露的方法或者广播的属性总个数小于255时
		我们不使用utype而使用1字节的aliasID来传输
	*/
	INLINE int16 aliasID() const;
	INLINE uint8 aliasIDAsUint8() const;
	INLINE void aliasID(int16 v);
	
protected:
	static uint32							methodDescriptionCount_;					// 所有的属性描述的数量

	COMPONENT_ID							methodDomain_;

	std::string								name_;										// 这个方法的名称
	ENTITY_METHOD_UID						utype_;										// 这个方法的数字类别， 用于网络上传输识别

	std::vector<DataType*>					argTypes_;									// 这个属性的参数类别列表

	EXPOSED_TYPE							exposedType_;								// 是否是一个暴露方法

	int16									aliasID_;									// 别名id， 当暴露的方法或者广播的属性总个数小于255时， 我们不使用utype而使用1字节的aliasID来传输
};

}

#ifdef CODE_INLINE
#include "method.inl"
#endif
#endif // KBENGINE_DEF_METHOD_H
