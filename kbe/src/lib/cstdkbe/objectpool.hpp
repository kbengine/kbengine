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

#ifndef KBE_OBJECTPOOL_HPP
#define KBE_OBJECTPOOL_HPP
	
// common include	
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <map>	
#include <list>	
#include <vector>
#include <queue> 

#include "thread/threadmutex.hpp"

// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

#define OBJECT_POOL_INIT_SIZE	16
#define OBJECT_POOL_INIT_MAX_SIZE	OBJECT_POOL_INIT_SIZE * 16

template< typename T >
class SmartPoolObject;

template< typename T >
class ObjectPool
{
public:
	typedef std::list<T*> OBJECTS;

	ObjectPool(std::string name):
		objects_(),
		max_(OBJECT_POOL_INIT_MAX_SIZE),
		isDestroyed_(false),
		mutex_(),
		name_(name),
		totalAlloc_(0),
		obj_count_(0)
	{
	}

	ObjectPool(std::string name, unsigned int preAssignVal, size_t max):
		objects_(),
		max_((max == 0 ? 1 : max)),
		isDestroyed_(false),
		mutex_(),
		name_(name),
		totalAlloc_(0),
		obj_count_(0)
	{
	}

	~ObjectPool()
	{
		destroy();
	}	
	
	void destroy()
	{
		mutex_.lockMutex();
		isDestroyed_ = true;
		typename OBJECTS::iterator iter = objects_.begin();
		for(; iter!=objects_.end(); iter++)
		{
			if(!(*iter)->destructorPoolObject())
			{
				delete (*iter);
			}
		}
				
		objects_.clear();	
		obj_count_ = 0;
		mutex_.unlockMutex();
	}

	const OBJECTS& objects(void)const { return objects_; }

	void assignObjs(unsigned int preAssignVal = OBJECT_POOL_INIT_SIZE)
	{
		for(unsigned int i=0; i<preAssignVal; i++){
			objects_.push_back(new T);
			++totalAlloc_;
			++obj_count_;
		}
	}

	/** 
		强制创建一个指定类型的对象。 如果缓冲里已经创建则返回现有的，否则
		创建一个新的， 这个对象必须是继承自T的。
	*/
	template<typename T1>
	T* createObject(void)
	{
		mutex_.lockMutex();

		while(true)
		{
			if(obj_count_ > 0)
			{
				T* t = static_cast<T1*>(*objects_.begin());
				objects_.pop_front();
				--obj_count_;
				mutex_.unlockMutex();
				return t;
			}

			assignObjs();
		}

		mutex_.unlockMutex();

		return NULL;
	}

	/** 
		创建一个对象。 如果缓冲里已经创建则返回现有的，否则
		创建一个新的。
	*/
	T* createObject(void)
	{
		mutex_.lockMutex();

		while(true)
		{
			if(obj_count_ > 0)
			{
				T* t = static_cast<T*>(*objects_.begin());
				objects_.pop_front();
				--obj_count_;

				// 先重置状态
				t->onReclaimObject();

				mutex_.unlockMutex();
				return t;
			}

			assignObjs();
		}

		mutex_.unlockMutex();

		return NULL;
	}

	/**
		回收一个对象
	*/
	void reclaimObject(T* obj)
	{
		mutex_.lockMutex();
		reclaimObject_(obj);
		mutex_.unlockMutex();
	}

	/**
		回收一个对象容器
	*/
	void reclaimObject(std::list<T*>& objs)
	{
		mutex_.lockMutex();
		
		typename std::list< T* >::iterator iter = objs.begin();
		for(; iter != objs.end(); iter++)
		{
			reclaimObject_((*iter));
		}
		
		objs.clear();
		mutex_.unlockMutex();
	}

	/**
		回收一个对象容器
	*/
	void reclaimObject(std::vector< T* >& objs)
	{
		mutex_.lockMutex();
		
		typename std::vector< T* >::iterator iter = objs.begin();
		for(; iter != objs.end(); iter++)
		{
			reclaimObject_((*iter));
		}
		
		objs.clear();
		mutex_.unlockMutex();
	}

	/**
		回收一个对象容器
	*/
	void reclaimObject(std::queue<T*>& objs)
	{
		mutex_.lockMutex();
		
		while(!objs.empty())
		{
			T* t = objs.front();
			objs.pop();
			reclaimObject_(t);
		}

		mutex_.unlockMutex();
	}

	size_t size(void)const{ return obj_count_; }
	
	std::string c_str()
	{
		mutex_.lockMutex();

		char buf[1024];
		sprintf(buf, "ObjectPool::c_str(): name=%s, objs=%d/%d, isDestroyed=%s.\n", 
			name_.c_str(), (int)obj_count_, (int)max_, (isDestroyed ? "true" : "false"));

		mutex_.unlockMutex();
		return buf;
	}

	size_t max()const{ return max_; }
	size_t totalAlloc()const{ return totalAlloc_; }

	bool isDestroyed()const{ return isDestroyed_; }

protected:
	/**
		回收一个对象
	*/
	void reclaimObject_(T* obj)
	{
		if(obj != NULL)
		{
			if(size() >= max_ || isDestroyed_)
			{
				delete obj;
				--totalAlloc_;
			}
			else
			{
				objects_.push_back(obj);
				++obj_count_;
			}
		}
	}

protected:
	OBJECTS objects_;							// 对象缓冲器

	size_t max_;

	bool isDestroyed_;

	KBEngine::thread::ThreadMutex mutex_;

	std::string name_;

	size_t totalAlloc_;

	size_t obj_count_;
};

/*
	池对象， 所有使用池的对象必须实现回收功能。
*/
class PoolObject
{
public:
	virtual ~PoolObject(){}
	virtual void onReclaimObject() = 0;
	
	virtual size_t getPoolObjectBytes(){ return 0; }

	/**
		池对象被析构前的通知
		某些对象可以在此做一些工作
	*/
	virtual bool destructorPoolObject()
	{
		return false;
	}
};

template< typename T >
class SmartObjectPool : public ObjectPool<T>
{
public:
};

template< typename T >
class SmartPoolObject
{
public:
	SmartPoolObject(T* pPoolObject, ObjectPool<T>& objectPool):
	  pPoolObject_(pPoolObject),
	  objectPool_(objectPool)
	{
	}

	~SmartPoolObject()
	{
		onReclaimObject();
	}

	void onReclaimObject()
	{
		if(pPoolObject_ != NULL)
		{
			objectPool_.reclaimObject(pPoolObject_);
			pPoolObject_ = NULL;
		}
	}

	T* get()
	{
		return pPoolObject_;
	}

	T* operator->()
	{
		return pPoolObject_;
	}

	T& operator*()
	{
		return *pPoolObject_;
	}
private:
	T* pPoolObject_;
	ObjectPool<T>& objectPool_;
};


#define NEW_POOL_OBJECT(TYPE) TYPE::ObjPool().createObject();


}
#endif
