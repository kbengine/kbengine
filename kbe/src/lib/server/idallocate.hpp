/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
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
#include "log/debug_helper.hpp"
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

template< typename T >
class IDAllocate
{
protected:
	typename std::queue< T > m_idList_;						// id列表， 所有ID都存在这个列表里
	T m_lastID_;											// 最后一次申请到的ID
public:
	IDAllocate(): m_lastID_(0)
	{
	}

	~IDAllocate()
	{
	}	
	
	/** 分配一个id */
	T alloc(void)
	{
		if(m_idList_.size() > 0)
		{
			T n = m_idList_.front();
			m_idList_.pop();
			return n;
		}
		
		return ++m_lastID_;
	}
	
	/** 回收一个id */
	void reclaim(T id)
	{
		m_idList_.push(id);
	}
	
};


template< typename T >
class IDServer
{
protected:
	T m_lastIDRange_begin_;										// 最后一次申请到的ID段的起始位置
	T m_rangeStep_;												// id段的一个段长度
public:
	IDServer(T idBegin, T rangeStep): 
	m_lastIDRange_begin_(idBegin), 
	m_rangeStep_(rangeStep)
	{
	}

	~IDServer()
	{
	}	
	
	/** 分配一个id段 */
	std::pair< T, T > allocRange(void)
	{
		DEBUG_MSG("IDServer::allocRange: %d-%d.\n", m_lastIDRange_begin_, m_lastIDRange_begin_ + m_rangeStep_);
		std::pair< T, T > p = std::make_pair(m_lastIDRange_begin_, m_lastIDRange_begin_ + m_rangeStep_);
		m_lastIDRange_begin_ += m_rangeStep_;
		return p;
	}
};

template< typename T >
class IDClient
{
protected:
	typename std::queue< std::pair< T, T > > m_idList_;					// id列表， 所有ID都存在这个列表里
	T m_lastIDRange_begin_;												// 最后一次申请到的ID段的起始位置
	T m_lastIDRange_end_;												
public:
	IDClient():m_lastIDRange_begin_(0), m_lastIDRange_end_(0)
	{
	}
	
	/** 析构时不会通知IDServer进行回收， 请使用者自己进行这方面的维护 */
	~IDClient()
	{
	}	
	
	size_t getSize()const{ return m_lastIDRange_end_ - m_lastIDRange_begin_; }
	
	/** idserver 分配过来的一个id段 */
	void onAddRange(T idBegin, T idEnd)
	{
		DEBUG_MSG("IDClient::onAddRange: number of ids increased from %d to %d.\n", idBegin, idEnd);
		if(getSize() <= 0)
		{
			m_lastIDRange_begin_ = idBegin;
			m_lastIDRange_end_ = idEnd;
		}
		else
		{
			m_idList_.push(std::make_pair(idBegin, idEnd));
		}
	}
	
	/** 分配一个id */
	T alloc(void)
	{
		assert(getSize() > 0 && "IDClient:: alloc:no usable of the id.\n");
		T id = m_lastIDRange_begin_;
		m_lastIDRange_begin_ ++;
		if(m_lastIDRange_begin_ > m_lastIDRange_end_)
		{
			if(m_idList_.size() > 0)
			{
				std::pair< T, T > n = m_idList_.front();
				m_lastIDRange_begin_ = n.first;
				m_lastIDRange_end_ = n.second;
				m_idList_.pop();
			}
			else
			{
				m_lastIDRange_begin_ = m_lastIDRange_end_ = 0;
			}
		}
		return id;
	}
	
	/** 回收一个id */
	void onReclaim(T id)
	{
	}
};


}
#endif
