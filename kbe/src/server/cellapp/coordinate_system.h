// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_COORDINATE_SYSTEM_H
#define KBE_COORDINATE_SYSTEM_H

#include "helper/debug_helper.h"
#include "common/common.h"	

//#define DEBUG_COORDINATE_SYSTEM

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
	void releaseNodes();

	/**
		当某个节点有变动时，需要更新它在list中的
		相关位置等信息
	*/
	void update(CoordinateNode* pNode);

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

	INLINE void incUpdating();
	INLINE void decUpdating();

private:
	uint32 size_;

	// 链表的首尾指针
	CoordinateNode* first_x_coordinateNode_;
	CoordinateNode* first_y_coordinateNode_;
	CoordinateNode* first_z_coordinateNode_;

	std::list<CoordinateNode*> dels_;
	size_t dels_count_;

	int updating_;

	std::list<CoordinateNode*> releases_;
};

}

#ifdef CODE_INLINE
#include "coordinate_system.inl"
#endif
#endif
