/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __KBENGINE_DEF_METHOD_H__
#define __KBENGINE_DEF_METHOD_H__
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)

// common include	
#include "datatype.hpp"
#include "datatypes.hpp"
#include "log/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "network/Packet.hpp"
#include "entitymailboxabstract.hpp"
#include "pyscript/scriptobject.hpp"	
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

class LIB_DLLAPI MethodDescription
{
protected:	
	static uint32				methodDescriptionCount_;						// 所有的属性描述的数量
	std::string					m_name_;										// 这个方法的名称
	uint32						m_utype_;										// 这个方法的数字类别， 用于网络上传输识别
	std::vector<DataType*>		m_argTypes_;									// 这个属性的参数类别列表
	bool						m_isExposed_;									// 是否是一个暴露方法
public:	
	MethodDescription(std::string name, bool isExposed = false);
	virtual ~MethodDescription();
	
	std::string& getName(void){ return m_name_; };
	uint32 getUType(void)const{ return m_utype_; }
	static uint32 getDescriptionCount(void){ return methodDescriptionCount_; }
	bool isExposed(void)const{ return m_isExposed_; }
	void setExposed(void){ m_isExposed_ = true; }
	bool pushArgType(DataType* dataType);
	std::vector<DataType*>& getArgTypes(void){ return m_argTypes_; }
	size_t getArgSize(void);
	
	/** 检查一个call是否合法 */
	bool checkArgs(PyObject* args);		
	
	/** 将每个参数打包添加到流， 这个流里包含的信息是这个方法在脚本被调用时里传入的参数 */
	void addToStream(MemoryStream* mstream, PyObject* args);
	/** 将一个call流解包 并返回一个PyObject类型的args */
	PyObject* createFromStream(MemoryStream* mstream);
	
	/** 呼叫一个方法 */
	PyObject* call(PyObject* func, PyObject* args);	
};

class LIB_DLLAPI script::ScriptObject;
class LIB_DLLAPI RemoteEntityMethod : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(RemoteEntityMethod, script::ScriptObject)	
protected:	
	MethodDescription*		m_methodDescription_;					// 这个方法的描述
	EntityMailboxAbstract*	m_pMailbox_;							// 这个方法所属的mailbox
public:	
	RemoteEntityMethod(MethodDescription* methodDescription, EntityMailboxAbstract* mailbox);
	virtual ~RemoteEntityMethod();
	std::string& getName(void){ return m_methodDescription_->getName(); };
	MethodDescription* getDescription(void)const{ return m_methodDescription_; }
	static PyObject* tp_call(PyObject* self, PyObject* args, PyObject* kwds);
	EntityMailboxAbstract* getMailbox(void)const { return m_pMailbox_; }
};
}
#endif
