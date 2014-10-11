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
	IDAllocate(������)
		��������һ�����������������Ψһid�� ʹ����������������Լ���֤�� һ��Ӧ��ֻ��ʹ��
		ͬһ��id����������ȡid����Ψһ�ġ�
		
		�����һ�� unsigned int���ͣ� �����������һֱ���Ϸ��䣬 ���ﵽ���͵����ֵ֮���
		��תͷ�ִ�0��ʼ�����ۼӷ��䣬 �����list��Ѱ�ң� �����ǰҪ�����IDû����list���ҵ�
		��ô���id�������䡣

		�÷�:
		IDAllocate<ENTITY_ID>* m_IDAllocPtr = new IDAllocate<ENTITY_ID>;
		// ����һ��id 
		m_IDAllocPtr->alloc()
		// ����һ��id
		m_IDAllocPtr->reclaim()
		
	IDServer(������)
		�����Ҫ���ṩ������������֮���entityID�ķ��䣬 ����Ҫ��baseappmgrʹ�ã� ÿ��IDserver
		�����ȡID��ʱ�� ����������ͻ����һ��Ψһid�θ��ͻ��ˣ� ��ô�ͻ��˾Ϳ��Ը��������
		�������е�Ψһid���������ɵķ��ɡ�
		
		�÷�:
		IDServer<ENTITY_ID>* m_idServer = new IDServer<ENTITY_ID>(1, 400);
		// ��ȡһ��id�� �������IDClient
		std::pair< unsigned int, unsigned int > idRange = m_idServer->allocRange();
		g_socketStreamIDClient->send(idRange.first, idRange.second);
		
	IDClient(�ͻ���)
		���ģ�������IDServer����id����ͽ��յģ� ��
		
		�÷�:
		IDClient<ENTITY_ID>* m_idClient = new IDClient<ENTITY_ID>;
		// ���IDServer���͹�����id��
		m_idClient->onAddRange(idBegin, idEnd);
		// ����һ��id 
		m_idClient->alloc()
*/
#ifndef KBE_IDALLOCATE_HPP
#define KBE_IDALLOCATE_HPP

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

// ֱ��ʹ��һ���������� �������������ʹ�С�͹���������Ҫʹ���޷�������
// ��������ʱ����id�� ���Һܿ�黹�� �����Ų���ID��ͻ
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
	
	/** 
		����һ��id 
	*/
	T alloc(void)
	{
		T t = ++lastID_;
		if(t == 0)
			t = ++lastID_;

		return t;
	}
	
	/** 
		����һ��id 
	*/
	virtual void reclaim(T id)
	{
	}

	T lastID()const{ return lastID_; }
	void lastID(T v){ lastID_ = v; }
protected:
	T lastID_;													// ���һ�����뵽��ID
};

// �����id�����洢���б��У� �´�ʹ�û���л�ȡ
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
		����һ��id 
	*/
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
	
	/** 
		����һ��id 
	*/
	void reclaim(T id)
	{
		idList_.push(id);
	}
protected:
	typename std::queue< T > idList_;							// id�б� ����ID����������б���
};


template< typename T >
class IDServer
{
protected:
	T lastIDRange_begin_;										// ���һ�����뵽��ID�ε���ʼλ��
	T rangeStep_;												// id�ε�һ���γ���
public:
	IDServer(T idBegin, T rangeStep): 
	lastIDRange_begin_(idBegin), 
	rangeStep_(rangeStep)
	{
	}

	~IDServer()
	{
	}	
	
	/** 
		����һ��id�� 
	*/
	std::pair< T, T > allocRange(void)
	{
		INFO_MSG(fmt::format("IDServer::allocRange: {}-{}.\n", 
			lastIDRange_begin_, (lastIDRange_begin_ + rangeStep_)));
		
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
	requestedIDServerAlloc_(false)
	{
	}
	
	/** 
		����ʱ����֪ͨIDServer���л��գ� ��ʹ�����Լ������ⷽ���ά�� 
	*/
	virtual ~IDClient()
	{
	}	
	
	bool hasReqServerAlloc()const { 
		return requestedIDServerAlloc_; 
	}

	void setReqServerAllocFlag(bool has){ 
		requestedIDServerAlloc_ = has; 
	}

	size_t getSize()
	{ 
		size_t ncount = lastIDRange_end_ - lastIDRange_begin_; 
		if(ncount <= 0)
		{
			// �����Ƿ��л����ID�Σ�����id���þ�ʱ����������뻺�浽���
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
	
	/**
		���entityID�Ƿ��� 
		ע�⣺һ��tick��ʹ��ID������Ҫ����ID_ENOUGH_LIMIT
	*/
	virtual void onAlloc(void) {
	};
	
	/** 
		idserver ���������һ��id�� 
	*/
	void onAddRange(T idBegin, T idEnd)
	{
		INFO_MSG(fmt::format("IDClient::onAddRange: number of ids increased from {} to {}.\n", idBegin, idEnd));
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
	
	/** 
		����һ��id 
	*/
	T alloc(void)
	{
		KBE_ASSERT(getSize() > 0 && "IDClient:: alloc:no usable of the id.\n");

		T id = lastIDRange_begin_;
		lastIDRange_begin_ ++;

		if(lastIDRange_begin_ > lastIDRange_end_)
		{
			// �����Ƿ��л����ID�Σ�����id���þ�ʱ����������뻺�浽���
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
	
	/** 
		����һ��id
	*/
	void onReclaim(T id)
	{
	}
	
protected:
	typename std::queue< std::pair< T, T > > idList_;					// id�б� ����ID�ζ���������б���

	T lastIDRange_begin_;												// ���һ�����뵽��ID�ε���ʼλ��
	T lastIDRange_end_;	

	bool requestedIDServerAlloc_;										// �Ƿ��Ѿ�����ID����˷���ID
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

#endif // KBE_IDALLOCATE_HPP
