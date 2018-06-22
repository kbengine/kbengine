// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
		m_idClient->onAddRange(id_begin, id_end);
		// 分配一个id 
		m_idClient->alloc()
*/
#ifndef KBE_IDALLOCATE_H
#define KBE_IDALLOCATE_H

#include "helper/debug_helper.h"
#include "common/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <queue>	

	
namespace KBEngine{

class ServerApp;

// 直接使用一个迭代数， 如果数溢出了类型大小就归零所以需要使用无符号类型
// 适用于临时分配id， 并且很快归还， 这样才不会ID冲突
template<typename T>
class IDAllocate
{
public:
	IDAllocate(): last_id_(0)
	{
	}

	virtual ~IDAllocate()
	{
	}	
	
	/** 
		分配一个id 
	*/
	T alloc(void)
	{
		T t = ++last_id_;
		if(t == 0)
			t = ++last_id_;

		return t;
	}
	
	/** 
		回收一个id 
	*/
	virtual void reclaim(T id)
	{
	}

	T lastID() const{ return last_id_; }
	void lastID(T v){ last_id_ = v; }

protected:
	// 最后一次申请到的ID
	T last_id_;
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
	
	/** 
		分配一个id 
	*/
	T alloc(void)
	{
		if(id_list_.size() > 0)
		{
			T n = id_list_.front();
			id_list_.pop();
			return n;
		}
		
		T t = ++IDAllocate<T>::last_id_;
		if(t == 0)
			t = ++IDAllocate<T>::last_id_;

		return t;
	}
	
	/** 
		回收一个id 
	*/
	void reclaim(T id)
	{
		id_list_.push(id);
	}

protected:
	// id列表， 所有ID都存在这个列表里
	typename std::queue< T > id_list_;
};


template< typename T >
class IDServer
{
public:
	IDServer(T id_begin, T range_step): 
	last_id_range_begin_(id_begin), 
	range_step_(range_step)
	{
	}

	~IDServer()
	{
	}
	
	/** 
		分配一个id段 
	*/
	std::pair< T, T > allocRange(void)
	{
		INFO_MSG(fmt::format("IDServer::allocRange: {}-{}.\n", 
			last_id_range_begin_, (last_id_range_begin_ + range_step_)));
		
		std::pair< T, T > p = std::make_pair(last_id_range_begin_, last_id_range_begin_ + range_step_);
		last_id_range_begin_ += range_step_;
		return p;
	}

	void set_range_step(T range_step) {
		range_step_ = range_step;
	}

protected:
	// 最后一次申请到的ID段的起始位置
	T last_id_range_begin_;
	
	// id段的一个段长度
	T range_step_;	
};

template< typename T >
class IDClient
{										
public:
	IDClient():
	  last_id_range_begin_(0), 
	last_id_range_end_(0),
	requested_idserver_alloc_(false)
	{
	}
	
	/** 
		析构时不会通知IDServer进行回收， 请使用者自己进行这方面的维护 
	*/
	virtual ~IDClient()
	{
	}	
	
	bool hasReqServerAlloc() const { 
		return requested_idserver_alloc_; 
	}

	void setReqServerAllocFlag(bool has){ 
		requested_idserver_alloc_ = has; 
	}

	size_t size()
	{ 
		size_t nCount = last_id_range_end_ - last_id_range_begin_; 
		if(nCount <= 0)
		{
			// 看看是否有缓存的ID段（会在id快用尽时向服务器申请缓存到这里）
			if(id_list_.size() > 0)
			{
				std::pair< T, T > n = id_list_.front();
				last_id_range_begin_ = n.first;
				last_id_range_end_ = n.second;
				id_list_.pop();
				nCount = last_id_range_end_ - last_id_range_begin_; 
			}
		}

		return nCount;
	}
	
	/**
		检查entityID是否够用 
		注意：一个tick内使用ID数量不要超过ID_ENOUGH_LIMIT
	*/
	virtual void onAlloc(void) {
	};
	
	/** 
		idserver 分配过来的一个id段 
	*/
	void onAddRange(T id_begin, T id_end)
	{
		INFO_MSG(fmt::format("IDClient::onAddRange: number of ids increased from {} to {}.\n", id_begin, id_end));
		if(size() <= 0)
		{
			last_id_range_begin_ = id_begin;
			last_id_range_end_ = id_end;
		}
		else
		{
			id_list_.push(std::make_pair(id_begin, id_end));
		}
	}
	
	/** 
		分配一个id 
	*/
	T alloc(void)
	{
		KBE_ASSERT(size() > 0 && "IDClient:: alloc:no usable of the id.\n");

		T id = last_id_range_begin_++;

		if(last_id_range_begin_ > last_id_range_end_)
		{
			// 看看是否有缓存的ID段（会在id快用尽时向服务器申请缓存到这里）
			if(id_list_.size() > 0)
			{
				std::pair< T, T > n = id_list_.front();
				last_id_range_begin_ = n.first;
				last_id_range_end_ = n.second;
				id_list_.pop();
			}
			else
			{
				last_id_range_begin_ = last_id_range_end_ = 0;
			}
		}
		
		onAlloc();
		return id;
	}
	
	/** 
		回收一个id
	*/
	void onReclaim(T id)
	{
	}
	
protected:
	// id列表， 所有ID段都存在这个列表里
	typename std::queue< std::pair< T, T > > id_list_;

	// 最后一次申请到的ID段的起始位置
	T last_id_range_begin_;
	T last_id_range_end_;

	// 是否已经请求ID服务端分配ID
	bool requested_idserver_alloc_;	
};

class EntityIDClient : public IDClient<ENTITY_ID>
{
public:
	EntityIDClient();
	
	virtual ~EntityIDClient()
	{
	}	

	virtual void onAlloc(void);
	
	void pApp(ServerApp* pApp){ 
		pApp_ = pApp; 
	}

protected:
	ServerApp* pApp_;
};

}

#endif // KBE_IDALLOCATE_H
