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
	IDAllocate(分配器)
		用来分配一个本分配器所管理的唯一id。 使用这个分配器必须自己保证， 一个应用只能使用
		同一个id分配器来获取id才是唯一的。
		
		如果是一个 unsigned int类型， 这个分配器会一直向上分配， 当达到类型的最大值之后会
		从转头又从0开始向上累加分配， 它会从list中寻找， 如果当前要分配的ID没有在list中找到
		那么这个id将被分配。

		用法:
		IDAllocate<ENTITY_ID>* m_IDAllocPtr = new IDAllocate<ENTITY_ID>;
		// 分配一个id 
		m_IDAllocPtr->alloc()
		// 回收一个id
		m_IDAllocPtr->reclaim()
		
	IDServer(服务器)
		这个主要是提供整个服务器组之间的entityID的分配， 他主要被baseappmgr使用， 每个IDserver
		请求获取ID的时候， 这个服务器就会分配一个唯一id段给客户端， 那么客户端就可以根据这个段
		产生所有的唯一id并进行自由的分派。
		
		用法:
		IDServer<ENTITY_ID>* m_idServer = new IDServer<ENTITY_ID>(1, 400);
		// 获取一个id段 并传输给IDClient
		std::pair< unsigned int, unsigned int > idRange = m_idServer->allocRange();
		g_socketStreamIDClient->send(idRange.first, idRange.second);
		
	IDClient(客户端)
		这个模块是配合IDServer进行id申请和接收的， 。
		
		用法:
		IDClient<ENTITY_ID>* m_idClient = new IDClient<ENTITY_ID>;
		// 添加IDServer发送过来的id段
		m_idClient->onAddRange(idBegin, idEnd);
		// 分配一个id 
		m_idClient->alloc()
*/
#ifndef __IDAllocate_H__
#define __IDAllocate_H__

// common include	
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <queue>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

class ServerApp;

// 直接使用一个迭代数， 如果数溢出了类型大小就归零所以需要使用无符号类型
// 适用于临时分配id， 并且很快归还， 这样才不会ID冲突
template<typename T>
class IDAllocate
{
public:
	IDAllocate(): lastID_(0)
	{
	}

	virtual ~IDAllocate()
	{
	}	
	
	/** 分配一个id */
	T alloc(void)
	{
		T t = ++lastID_;
		if(t == 0)
			t = ++lastID_;

		return t;
	}
	
	/** 回收一个id */
	virtual void reclaim(T id)
	{
	}
protected:
	T lastID_;													// 最后一次申请到的ID
};

// 分配的id用完会存储在列表中， 下次使用会从中获取
template< typename T >
class IDAllocateFromList : public IDAllocate<T>
{
public:
	IDAllocateFromList(): IDAllocate<T>()
	{
	}

	~IDAllocateFromList()
	{
	}	
	
	/** 分配一个id */
	T alloc(void)
	{
		if(idList_.size() > 0)
		{
			T n = idList_.front();
			idList_.pop();
			return n;
		}
		
		T t = ++IDAllocate<T>::lastID_;
		if(t == 0)
			t = ++IDAllocate<T>::lastID_;

		return t;
	}
	
	/** 回收一个id */
	void reclaim(T id)
	{
		idList_.push(id);
	}
protected:
	typename std::queue< T > idList_;							// id列表， 所有ID都存在这个列表里
};


template< typename T >
class IDServer
{
protected:
	T lastIDRange_begin_;										// 最后一次申请到的ID段的起始位置
	T rangeStep_;												// id段的一个段长度
public:
	IDServer(T idBegin, T rangeStep): 
	lastIDRange_begin_(idBegin), 
	rangeStep_(rangeStep)
	{
	}

	~IDServer()
	{
	}	
	
	/** 分配一个id段 */
	std::pair< T, T > allocRange(void)
	{
		INFO_MSG(boost::format("IDServer::allocRange: %1%-%2%.\n") % lastIDRange_begin_ % (lastIDRange_begin_ + rangeStep_));
		std::pair< T, T > p = std::make_pair(lastIDRange_begin_, lastIDRange_begin_ + rangeStep_);
		lastIDRange_begin_ += rangeStep_;
		return p;
	}
};

template< typename T >
class IDClient
{										
public:
	IDClient():
	  lastIDRange_begin_(0), 
	lastIDRange_end_(0),
	m_hasRequestedIDServerAlloc_(false)
	{
	}
	
	/** 析构时不会通知IDServer进行回收， 请使用者自己进行这方面的维护 */
	virtual ~IDClient()
	{
	}	
	
	bool hasReqServerAlloc()const { return m_hasRequestedIDServerAlloc_; }
	void setReqServerAllocFlag(bool has){ m_hasRequestedIDServerAlloc_ = has; }

	size_t getSize()
	{ 
		size_t ncount = lastIDRange_end_ - lastIDRange_begin_; 
		if(ncount <= 0)
		{
			// 看看是否有缓存的ID段（会在id快用尽时向服务器申请缓存到这里）
			if(idList_.size() > 0)
			{
				std::pair< T, T > n = idList_.front();
				lastIDRange_begin_ = n.first;
				lastIDRange_end_ = n.second;
				idList_.pop();
				ncount = lastIDRange_end_ - lastIDRange_begin_; 
			}
		}

		return ncount;
	}
	
	/* 检查entityID是否够用 
		注意：一个tick内使用ID数量不要超过ID_ENOUGH_LIMIT
	*/
	virtual void onAlloc(void) {};
	
	/** idserver 分配过来的一个id段 */
	void onAddRange(T idBegin, T idEnd)
	{
		INFO_MSG(boost::format("IDClient::onAddRange: number of ids increased from %1% to %2%.\n") % idBegin % idEnd);
		if(getSize() <= 0)
		{
			lastIDRange_begin_ = idBegin;
			lastIDRange_end_ = idEnd;
		}
		else
		{
			idList_.push(std::make_pair(idBegin, idEnd));
		}
	}
	
	/** 分配一个id */
	T alloc(void)
	{
		if(getSize() <= 0)
			return 0;

		KBE_ASSERT(getSize() > 0 && "IDClient:: alloc:no usable of the id.\n");
		T id = lastIDRange_begin_;
		lastIDRange_begin_ ++;

		if(lastIDRange_begin_ > lastIDRange_end_)
		{
			// 看看是否有缓存的ID段（会在id快用尽时向服务器申请缓存到这里）
			if(idList_.size() > 0)
			{
				std::pair< T, T > n = idList_.front();
				lastIDRange_begin_ = n.first;
				lastIDRange_end_ = n.second;
				idList_.pop();
			}
			else
			{
				lastIDRange_begin_ = lastIDRange_end_ = 0;
			}
		}
		
		onAlloc();
		return id;
	}
	
	/** 回收一个id */
	void onReclaim(T id)
	{
	}
	
protected:
	typename std::queue< std::pair< T, T > > idList_;					// id列表， 所有ID段都存在这个列表里
	T lastIDRange_begin_;												// 最后一次申请到的ID段的起始位置
	T lastIDRange_end_;		
	bool m_hasRequestedIDServerAlloc_;									// 是否已经请求ID服务端分配ID
};

class EntityIDClient : public IDClient<ENTITY_ID>
{
public:
	EntityIDClient():
	IDClient<ENTITY_ID>(),
	pApp_(NULL)
	{
	}
	
	virtual ~EntityIDClient()
	{
	}	

	virtual void onAlloc(void);
	
	void pApp(ServerApp* pApp){ pApp_ = pApp; }
protected:
	ServerApp* pApp_;
};

}
#endif
