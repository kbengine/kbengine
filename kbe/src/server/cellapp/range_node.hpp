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

#ifndef __KBE_RANGE_NODE_HPP__
#define __KBE_RANGE_NODE_HPP__

#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"	

namespace KBEngine{

#define RANGENODE_FLAG_UNKNOWN				0x00000000
#define RANGENODE_FLAG_ENTITY				0x00000001		// 一个Entity节点
#define RANGENODE_FLAG_TRIGGER				0x00000002		// 一个触发器节点
#define RANGENODE_FLAG_HIDE					0x00000004		// 隐藏节点(其他节点不可见)
#define RANGENODE_FLAG_REMOVEING			0x00000008		// 删除节点
#define RANGENODE_FLAG_REMOVED				0x00000010		// 删除节点
#define RANGENODE_FLAG_PENDING				0x00000020		// 这类节点处于update操作中。

#define RANGENODE_FLAG_HIDE_OR_REMOVED		(RANGENODE_FLAG_REMOVED | RANGENODE_FLAG_HIDE)

class RangeList;
class RangeNode
{
public:
	RangeNode(RangeList* pRangeList = NULL);
	virtual ~RangeNode();
	
	INLINE void flags(uint32 v);
	INLINE uint32 flags()const;

	/**
		(节点本身的坐标)
		x && z由不同的应用实现(从不同处获取)
	*/
	virtual float x()const { return x_; }
	virtual float y()const { return y_; }
	virtual float z()const { return z_; }

	virtual void x(float v){ x_ = v; }
	virtual void y(float v){ y_ = v; }
	virtual void z(float v){ z_ = v; }

	/**
		(扩展坐标)
		x && z由不同的应用实现(从不同处获取)
	*/
	virtual float xx()const { return 0.f; }
	virtual float yy()const { return 0.f; }
	virtual float zz()const { return 0.f; }

	void old_xx(float v) { old_xx_ = v; }
	void old_yy(float v) { old_yy_ = v; }
	void old_zz(float v) { old_zz_ = v; }

	float old_xx()const { return old_xx_; }
	float old_yy()const { return old_yy_; }
	float old_zz()const { return old_zz_; }

	virtual void resetOld(){ 
		old_xx_ = xx();
		old_yy_ = yy();
		old_zz_ = zz();
	}

	void c_str();

	void debugX();
	void debugY();
	void debugZ();

	INLINE void pRangeList(RangeList* p);
	INLINE RangeList* pRangeList()const;

	/**
		获取链表的前后端指针
	*/
	INLINE RangeNode* pPrevX()const;
	INLINE RangeNode* pNextX()const;
	INLINE RangeNode* pPrevY()const;
	INLINE RangeNode* pNextY()const;
	INLINE RangeNode* pPrevZ()const;
	INLINE RangeNode* pNextZ()const;

	/**
		设置链表的前后端指针
	*/
	INLINE void pPrevX(RangeNode* pNode);
	INLINE void pNextX(RangeNode* pNode);
	INLINE void pPrevY(RangeNode* pNode);
	INLINE void pNextY(RangeNode* pNode);
	INLINE void pPrevZ(RangeNode* pNode);
	INLINE void pNextZ(RangeNode* pNode);

	/**
		某个节点变动经过了本节点
		@isfront: 向前移动还是向后移动
	*/
	virtual void onNodePassX(RangeNode* pNode, bool isfront);
	virtual void onNodePassY(RangeNode* pNode, bool isfront);
	virtual void onNodePassZ(RangeNode* pNode, bool isfront);

	virtual void onRemove();

	/**
		父节点删除
	*/
	virtual void onParentRemove(RangeNode* pParentNode){}

	/**
		当节点有变动时，需要更新它在list中的
		相关位置等信息
	*/
	virtual void update();

#ifdef _DEBUG
	void descr(const std::string& str){ descr_ = str; }
	virtual const char* descr(){ return descr_.c_str(); }
#else
	void descr(const std::string& str){}
	virtual const char* descr(){ return ""; }
#endif

protected:
	// 链表的前端和后端
	RangeNode* pPrevX_;
	RangeNode* pNextX_;
	RangeNode* pPrevY_;
	RangeNode* pNextY_;
	RangeNode* pPrevZ_;
	RangeNode* pNextZ_;

	RangeList* pRangeList_;

	float x_, y_, z_;
	float old_xx_, old_yy_, old_zz_;

#ifdef _DEBUG
	std::string descr_;
#endif

	uint32 flags_;
};

}

#ifdef CODE_INLINE
#include "range_node.ipp"
#endif
#endif
