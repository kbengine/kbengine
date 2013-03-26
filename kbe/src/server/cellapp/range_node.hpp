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

class RangeNode
{
public:
	RangeNode();
	virtual ~RangeNode();

	/**
		x && z由不同的应用实现(从不同处获取)
	*/
	virtual float x()const = 0;
	virtual float z()const = 0;

	/**
		获取链表的前后端指针
	*/
	INLINE RangeNode* pPrevX()const;
	INLINE RangeNode* pNextX()const;
	INLINE RangeNode* pPrevZ()const;
	INLINE RangeNode* pNextZ()const;

	/**
		设置链表的前后端指针
	*/
	INLINE void pPrevX(RangeNode* pNode);
	INLINE void pNextX(RangeNode* pNode);
	INLINE void pPrevZ(RangeNode* pNode);
	INLINE void pNextZ(RangeNode* pNode);
protected:
	// 链表的前端和后端
	RangeNode* pPrevX_;
	RangeNode* pNextX_;
	RangeNode* pPrevZ_;
	RangeNode* pNextZ_;
};

}

#ifdef CODE_INLINE
#include "range_node.ipp"
#endif
#endif
