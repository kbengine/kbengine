// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_OBJECTPOOL_H
#define KBE_OBJECTPOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <map>	
#include <list>	
#include <vector>
#include <queue> 

#include "common/timestamp.h"
#include "thread/threadmutex.h"

namespace KBEngine{

#define OBJECT_POOL_INIT_SIZE			16
#define OBJECT_POOL_INIT_MAX_SIZE		OBJECT_POOL_INIT_SIZE * 1024

// 每5分钟检查一次瘦身
#define OBJECT_POOL_REDUCING_TIME_OUT	300 * stampsPerSecondD()

// 追踪对象分配处
#define OBJECTPOOL_POINT fmt::format("{}#{}", __FUNCTION__, __LINE__).c_str() 

template< typename T >
class SmartPoolObject;

class ObjectPoolLogPoint
{
public:
	ObjectPoolLogPoint() :
		count(0)
	{

	}

	int count;
};

/*
	一些对象会非常频繁的被创建， 例如：MemoryStream, Bundle, TCPPacket等等
	这个对象池对通过服务端峰值有效的预估提前创建出一些对象缓存起来，在用到的时候直接从对象池中
	获取一个未被使用的对象即可。
*/
template< typename T, typename THREADMUTEX = KBEngine::thread::ThreadMutexNull >
class ObjectPool
{
public:
	typedef std::list<T*> OBJECTS;

	ObjectPool(std::string name):
		objects_(),
		max_(OBJECT_POOL_INIT_MAX_SIZE),
		isDestroyed_(false),
		pMutex_(new THREADMUTEX()),
		name_(name),
		total_allocs_(0),
		obj_count_(0),
		lastReducingCheckTime_(timestamp()),
		logPoints_()
	{
	}

	ObjectPool(std::string name, unsigned int preAssignVal, size_t max):
		objects_(),
		max_((max == 0 ? 1 : max)),
		isDestroyed_(false),
		pMutex_(new THREADMUTEX()),
		name_(name),
		total_allocs_(0),
		obj_count_(0),
		lastReducingCheckTime_(timestamp()),
		logPoints_()
	{
	}

	~ObjectPool()
	{
		destroy();
		SAFE_RELEASE(pMutex_);
	}	
	
	void destroy()
	{
		pMutex_->lockMutex();

		isDestroyed_ = true;

		typename OBJECTS::iterator iter = objects_.begin();
		for(; iter!=objects_.end(); ++iter)
		{
			(*iter)->isEnabledPoolObject(false);
			if(!(*iter)->destructorPoolObject())
			{
				delete (*iter);
			}
		}
				
		objects_.clear();	
		obj_count_ = 0;
		pMutex_->unlockMutex();
	}

	const OBJECTS& objects(void) const 
	{ 
		return objects_; 
	}

	void pMutex(KBEngine::thread::ThreadMutexNull* pMutex)
	{
		SAFE_RELEASE(pMutex_);
		pMutex_ = pMutex;
	}

	KBEngine::thread::ThreadMutexNull* pMutex()
	{
		return pMutex_;
	}

	void assignObjs(unsigned int preAssignVal = OBJECT_POOL_INIT_SIZE)
	{
		for(unsigned int i=0; i<preAssignVal; ++i)
		{
			T* t = new T();
			t->isEnabledPoolObject(false);
			objects_.push_back(t);
			++total_allocs_;
			++obj_count_;
		}
	}

	/** 
		创建一个对象。 如果缓冲里已经创建则返回现有的，否则
		创建一个新的。
	*/
	T* createObject(const std::string& logPoint)
	{
		pMutex_->lockMutex();

		while(true)
		{
			if(obj_count_ > 0)
			{
				T* t = static_cast<T*>(*objects_.begin());
				objects_.pop_front();
				--obj_count_;
				incLogPoint(logPoint);
				t->poolObjectCreatePoint(logPoint);
				t->onEabledPoolObject();
				t->isEnabledPoolObject(true);
				pMutex_->unlockMutex();
				return t;
			}

			assignObjs();
		}

		pMutex_->unlockMutex();

		return NULL;
	}

	/**
		回收一个对象
	*/
	void reclaimObject(T* obj)
	{
		pMutex_->lockMutex();
		reclaimObject_(obj);
		pMutex_->unlockMutex();
	}

	/**
		回收一个对象容器
	*/
	void reclaimObject(std::list<T*>& objs)
	{
		pMutex_->lockMutex();

		typename std::list< T* >::iterator iter = objs.begin();
		for(; iter != objs.end(); ++iter)
		{
			reclaimObject_((*iter));
		}
		
		objs.clear();

		pMutex_->unlockMutex();
	}

	/**
		回收一个对象容器
	*/
	void reclaimObject(std::vector< T* >& objs)
	{
		pMutex_->lockMutex();

		typename std::vector< T* >::iterator iter = objs.begin();
		for(; iter != objs.end(); ++iter)
		{
			reclaimObject_((*iter));
		}
		
		objs.clear();

		pMutex_->unlockMutex();
	}

	/**
		回收一个对象容器
	*/
	void reclaimObject(std::queue<T*>& objs)
	{
		pMutex_->lockMutex();

		while(!objs.empty())
		{
			T* t = objs.front();
			objs.pop();
			reclaimObject_(t);
		}

		pMutex_->unlockMutex();
	}

	size_t size(void) const { return obj_count_; }
	
	std::string c_str()
	{
		char buf[1024];

		pMutex_->lockMutex();

		sprintf(buf, "ObjectPool::c_str(): name=%s, objs=%d/%d, isDestroyed=%s.\n", 
			name_.c_str(), (int)obj_count_, (int)max_, (isDestroyed() ? "true" : "false"));

		pMutex_->unlockMutex();

		return buf;
	}

	size_t max() const { return max_; }
	size_t totalAllocs() const { return total_allocs_; }

	bool isDestroyed() const { return isDestroyed_; }

	std::map<std::string, ObjectPoolLogPoint>& logPoints() {
		return logPoints_;
	}

	void incLogPoint(const std::string& logPoint)
	{
		++logPoints_[logPoint].count;
	}

	void decLogPoint(const std::string& logPoint)
	{
		--logPoints_[logPoint].count;
	}

protected:
	/**
		回收一个对象
	*/
	void reclaimObject_(T* obj)
	{
		if(obj != NULL)
		{
			decLogPoint(obj->poolObjectCreatePoint());

			// 先重置状态
			obj->onReclaimObject();
			obj->isEnabledPoolObject(false);
			obj->poolObjectCreatePoint("");

			if(size() >= max_ || isDestroyed_)
			{
				delete obj;
				--total_allocs_;
			}
			else
			{
				objects_.push_back(obj);
				++obj_count_;
			}
		}

		uint64 now_timestamp = timestamp();

		if (obj_count_ <= OBJECT_POOL_INIT_SIZE)
		{
			// 小于等于则刷新检查时间
			lastReducingCheckTime_ = now_timestamp;
		}
		else if (now_timestamp - lastReducingCheckTime_ > OBJECT_POOL_REDUCING_TIME_OUT)
		{
			// 长时间大于OBJECT_POOL_INIT_SIZE未使用的对象则开始做清理工作
			size_t reducing = std::min(objects_.size(), std::min((size_t)OBJECT_POOL_INIT_SIZE, (size_t)(obj_count_ - OBJECT_POOL_INIT_SIZE)));
			
			//printf("ObjectPool::reclaimObject_(): start reducing..., name=%s, currsize=%d, OBJECT_POOL_INIT_SIZE=%d\n", 
			//	name_.c_str(), (int)objects_.size(), OBJECT_POOL_INIT_SIZE);

			while (reducing-- > 0)
			{
				T* t = static_cast<T*>(*objects_.begin());
				objects_.pop_front();
				delete t;

				--obj_count_;
			}

			//printf("ObjectPool::reclaimObject_(): reducing over, name=%s, currsize=%d\n", 
			//	name_.c_str(), (int)objects_.size());

			lastReducingCheckTime_ = now_timestamp;
		}
	}

protected:
	OBJECTS objects_;

	size_t max_;

	bool isDestroyed_;

	// 一些原因导致锁还是有必要的
	// 例如：dbmgr任务线程中输出log，cellapp中加载navmesh后的线程回调导致的log输出
	THREADMUTEX* pMutex_;

	std::string name_;

	size_t total_allocs_;

	// Linux环境中，list.size()使用的是std::distance(begin(), end())方式来获得
	// 会对性能有影响，这里我们自己对size做一个记录
	size_t obj_count_;

	// 最后一次瘦身检查时间
	// 如果长达OBJECT_POOL_REDUCING_TIME_OUT大于OBJECT_POOL_INIT_SIZE，则最多瘦身OBJECT_POOL_INIT_SIZE个
	uint64 lastReducingCheckTime_;

	// 记录的创建位置信息，用于追踪泄露点
	std::map<std::string, ObjectPoolLogPoint> logPoints_;
};

/*
	池对象， 所有使用池的对象必须实现回收功能。
*/
class PoolObject
{
public:
	PoolObject() : 
		isEnabledPoolObject_(false)
	{

	}

	virtual ~PoolObject(){}
	virtual void onReclaimObject() = 0;
	virtual void onEabledPoolObject() {
	}

	virtual size_t getPoolObjectBytes()
	{ 
		return 0; 
	}

	/**
		池对象被析构前的通知
		某些对象可以在此做一些工作
	*/
	virtual bool destructorPoolObject()
	{
		return false;
	}

	bool isEnabledPoolObject() const
	{
		return isEnabledPoolObject_;
	}

	void isEnabledPoolObject(bool v)
	{
		isEnabledPoolObject_ = v;
	}

	void poolObjectCreatePoint(const std::string& logPoint)
	{
		poolObjectCreatePoint_ = logPoint;
	}

	const std::string& poolObjectCreatePoint() const
	{
		return poolObjectCreatePoint_;
	}

protected:

	// 池对象是否处于激活（从池中已经取出）状态
	bool isEnabledPoolObject_;

	// 记录对象创建的位置
	std::string poolObjectCreatePoint_;
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


#define NEW_POOL_OBJECT(TYPE) TYPE::createPoolObject(OBJECTPOOL_POINT);


}
#endif
