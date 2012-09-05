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
	使用方法:
	ObjectPool<TimerData>* objPool = new ObjectPool<TimerData>();
	struct TimerData * td = objPool->createObject();
	objPool->reclaimObject(td);		// 归还对象
*/
#ifndef __OBJECTPOOL_H__
#define __OBJECTPOOL_H__
	
// common include	
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <map>	
#include <list>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

#define OBJECT_POOL_INIT_SIZE	16

template< typename T >
class ObjectPool
{
public:
	typedef std::list<T* > OBJECTS;

	ObjectPool(unsigned int preAssignVal = OBJECT_POOL_INIT_SIZE)
	{
		for(unsigned int i=0; i<preAssignVal; i++){
			objects_.push_back(new T);
		}
	}

	~ObjectPool()
	{
		ObjectPool::OBJECTS::iterator iter = objects_.begin();
		for(; iter!=objects_.end(); iter++)
			SAFE_RELEASE((*iter));
				
		objects_.clear();	
	}	
	
	const ObjectPool::OBJECTS& objects(void)const { return objects_; }

	/** 强制创建一个指定类型的对象。 如果缓冲里已经创建则返回现有的，否则
		创建一个新的， 这个对象必须是继承自T的。
	*/
	template<typename T1>
	T* createObject(void)
	{
		if(objects_.size() > 0){
			T* t = static_cast<T1*>(*objects_.begin());
			objects_.pop_front();
			return t;
		}
		
		// INFO_MSG("ObjectPool:create new object! total:%d\n", m_totalCount_);
		return new T1;
	}

	/** 创建一个对象。 如果缓冲里已经创建则返回现有的，否则
		创建一个新的。
	*/
	T* createObject(void)
	{
		if(objects_.size() > 0){
			T* t = static_cast<T*>(*objects_.begin());
			objects_.pop_front();
			return t;
		}
		
		// INFO_MSG("ObjectPool:create new object! total:%d\n", m_totalCount_);
		return new T;
	}

	/**回收一个对象*/
	void reclaimObject(T* obj)
	{
		objects_.push_back(obj);
	}

	size_t size(void)const{ return objects_.size(); }
	
protected:
	ObjectPool::OBJECTS objects_;							// 对象缓冲器
};

}
#endif
