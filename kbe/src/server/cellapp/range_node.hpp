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

class RangeList;
class RangeNode
{
public:
	RangeNode(RangeList* pRangeList = NULL);
	virtual ~RangeNode();

	/**
		x && z由不同的应用实现(从不同处获取)
	*/
	virtual float x()const { return x_; }
	virtual float y()const { return y_; }
	virtual float z()const { return z_; }

	virtual void x(float v){ x_ = v; }
	virtual void y(float v){ y_ = v; }
	virtual void z(float v){ z_ = v; }

	float old_x()const { return old_x_; }
	float old_y()const { return old_y_; }
	float old_z()const { return old_z_; }

	void resetOld(){ 
		old_x_ = x_;
		old_y_ = y_;
		old_z_ = z_;
	}

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

	/**
		当节点有变动时，需要更新它在list中的
		相关位置等信息
	*/
	void update();
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
	float old_x_, old_y_, old_z_;
};

}

#ifdef CODE_INLINE
#include "range_node.ipp"
#endif
#endif
