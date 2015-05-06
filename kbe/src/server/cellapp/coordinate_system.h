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

#ifndef KBE_COORDINATE_SYSTEM_H
#define KBE_COORDINATE_SYSTEM_H

#include "helper/debug_helper.h"
#include "common/common.h"	

namespace KBEngine{

class CoordinateNode;

class CoordinateSystem
{
public:
	CoordinateSystem();
	~CoordinateSystem();

	/**
		向list中插入节点
	*/
	bool insert(CoordinateNode* pNode);

	/**
		将节点从list中移除
	*/
	bool remove(CoordinateNode* pNode);
	bool removeReal(CoordinateNode* pNode);
	void removeDelNodes();
	
	/**
		当某个节点有变动时，需要更新它在list中的
		相关位置等信息
	*/
	void update(CoordinateNode* pNode);
	void update(CoordinateNode* pNode, CoordinateNode& tmpNode, bool isMove);

	/**
		移动节点
	*/
	void moveNodeX(CoordinateNode* pNode, float px, CoordinateNode* pCurrNode);
	void moveNodeY(CoordinateNode* pNode, float py, CoordinateNode* pCurrNode);
	void moveNodeZ(CoordinateNode* pNode, float pz, CoordinateNode* pCurrNode);

	INLINE CoordinateNode * pFirstXNode() const;
	INLINE CoordinateNode * pFirstYNode() const;
	INLINE CoordinateNode * pFirstZNode() const;

	INLINE bool isEmpty() const;

	INLINE uint32 size() const;

	static bool hasY;

private:
	uint32 size_;

	// 链表的首尾指针
	CoordinateNode* first_x_coordinateNode_;
	CoordinateNode* first_y_coordinateNode_;
	CoordinateNode* first_z_coordinateNode_;

	std::list<CoordinateNode*> dels_;
	size_t dels_count_;

	int updating_;
};

}

#ifdef CODE_INLINE
#include "coordinate_system.inl"
#endif
#endif
