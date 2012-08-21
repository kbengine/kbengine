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

	
/*
	CallbackMgr( 回调管理器 )
		由于一些回调操作都是异步的， 我们通过一个管理器将这些回调管理起来， 并对外返回一个
		标识该回调的唯一id， 外部可以通过该id来触发这个回调。
		
	用法:
	typedef CallbackMgr<std::tr1::function<void(Base*, int64, bool)>> CALLBACK_MGR;
	CALLBACK_MGR callbackMgr;
	void xxx(Base*, int64, bool){}
	CALLBACK_ID callbackID = callbackMgr.save(&xxx); // 可以使用bind来绑定一个类成员函数
*/

#ifndef __CALLBACKMGR_H__
#define __CALLBACKMGR_H__

// common include	
#include "Python.h"
#include "idallocate.hpp"
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "pyscript/pyobject_pointer.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

template<typename T>
class CallbackMgr
{
public:
	CallbackMgr()
	{
	}

	~CallbackMgr()
	{
		cbMap_.clear();
	}	
	
	
	/** 向管理器添加一个回调 */
	CALLBACK_ID save(T callback)
	{
		CALLBACK_ID cbID = idAlloc_.alloc();
		cbMap_.insert(typename std::map<CALLBACK_ID, T>::value_type(cbID, callback));
		return cbID;
	}
	
	/** 通过callbackID取走回调 */
	T take(CALLBACK_ID cbID)
	{
		typename std::map<CALLBACK_ID, T>::iterator itr = cbMap_.find(cbID);
		if(itr != cbMap_.end()){
			T t = itr->second;
			idAlloc_.reclaim(itr->first);
			cbMap_.erase(itr);
			return t;
		}
		
		return NULL;
	}
protected:
	typename std::map<CALLBACK_ID, T> cbMap_;					// 所有的回调都存储在这个map中
	IDAllocate<CALLBACK_ID> idAlloc_;							// 回调的id分配器
};

template<>
inline CallbackMgr<PyObject*>::~CallbackMgr()
{
	std::map<CALLBACK_ID, PyObject*>::iterator iter = cbMap_.begin();
	for(; iter!= cbMap_.end(); iter++)
	{
		Py_DECREF(iter->second);
	}

	cbMap_.clear();
}	

typedef CallbackMgr<PyObjectPtr> PY_CALLBACKMGR;

}
#endif
