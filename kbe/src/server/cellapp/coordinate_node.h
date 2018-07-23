// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_COORDINATE_NODE_H
#define KBE_COORDINATE_NODE_H

#include "helper/debug_helper.h"
#include "common/common.h"	

namespace KBEngine{

#define COORDINATE_NODE_FLAG_UNKNOWN				0x00000000
#define COORDINATE_NODE_FLAG_ENTITY					0x00000001		// 一个Entity节点
#define COORDINATE_NODE_FLAG_TRIGGER				0x00000002		// 一个触发器节点
#define COORDINATE_NODE_FLAG_HIDE					0x00000004		// 隐藏节点(其他节点不可见)
#define COORDINATE_NODE_FLAG_REMOVING				0x00000008		// 删除中的节点
#define COORDINATE_NODE_FLAG_REMOVED				0x00000010		// 删除节点
#define COORDINATE_NODE_FLAG_PENDING				0x00000020		// 这类节点处于update操作中。
#define COORDINATE_NODE_FLAG_ENTITY_NODE_UPDATING	0x00000040		// entity节点正在执行update操作
#define COORDINATE_NODE_FLAG_INSTALLING				0x00000080		// 节点正在安装操作
#define COORDINATE_NODE_FLAG_POSITIVE_BOUNDARY		0x00000100		// 节点是触发器的正边界
#define COORDINATE_NODE_FLAG_NEGATIVE_BOUNDARY		0x00000200		// 节点是触发器的负边界

#define COORDINATE_NODE_FLAG_HIDE_OR_REMOVED		(COORDINATE_NODE_FLAG_REMOVED | COORDINATE_NODE_FLAG_HIDE)

class CoordinateSystem;
class CoordinateNode
{
public:
	CoordinateNode(CoordinateSystem* pCoordinateSystem = NULL);
	virtual ~CoordinateNode();
	
	INLINE void flags(uint32 v);
	INLINE uint32 flags() const;
	INLINE void addFlags(uint32 v);
	INLINE void removeFlags(uint32 v);
	INLINE bool hasFlags(uint32 v) const;

	/**
		(节点本身的坐标)
		x && z由不同的应用实现(从不同处获取)
	*/
	virtual float x() const { return x_; }
	virtual float y() const { return y_; }
	virtual float z() const { return z_; }

	virtual void x(float v) { x_ = v; }
	virtual void y(float v) { y_ = v; }
	virtual void z(float v) { z_ = v; }

	/**
		(扩展坐标)
		x && z由不同的应用实现(从不同处获取)
	*/
	virtual float xx() const { return 0.f; }
	virtual float yy() const { return 0.f; }
	virtual float zz() const { return 0.f; }

	void old_xx(float v) { old_xx_ = v; }
	void old_yy(float v) { old_yy_ = v; }
	void old_zz(float v) { old_zz_ = v; }

	float old_xx() const { return old_xx_; }
	float old_yy() const { return old_yy_; }
	float old_zz() const { return old_zz_; }

	int8 weight() const { return weight_; }

	virtual void resetOld() { 
		old_xx_ = xx();
		old_yy_ = yy();
		old_zz_ = zz();
	}

	std::string c_str();

	void debugX();
	void debugY();
	void debugZ();

	INLINE void pCoordinateSystem(CoordinateSystem* p);
	INLINE CoordinateSystem* pCoordinateSystem() const;

	INLINE bool isDestroying() const {
		return hasFlags(COORDINATE_NODE_FLAG_REMOVING);
	}

	INLINE bool isDestroyed() const {
		return hasFlags(COORDINATE_NODE_FLAG_REMOVED);
	}

	/**
		获取链表的前后端指针
	*/
	INLINE CoordinateNode* pPrevX() const;
	INLINE CoordinateNode* pNextX() const;
	INLINE CoordinateNode* pPrevY() const;
	INLINE CoordinateNode* pNextY() const;
	INLINE CoordinateNode* pPrevZ() const;
	INLINE CoordinateNode* pNextZ() const;

	/**
		设置链表的前后端指针
	*/
	INLINE void pPrevX(CoordinateNode* pNode);
	INLINE void pNextX(CoordinateNode* pNode);
	INLINE void pPrevY(CoordinateNode* pNode);
	INLINE void pNextY(CoordinateNode* pNode);
	INLINE void pPrevZ(CoordinateNode* pNode);
	INLINE void pNextZ(CoordinateNode* pNode);

	/**
		某个节点变动经过了本节点
		@isfront: 向前移动还是向后移动
	*/
	virtual void onNodePassX(CoordinateNode* pNode, bool isfront);
	virtual void onNodePassY(CoordinateNode* pNode, bool isfront);
	virtual void onNodePassZ(CoordinateNode* pNode, bool isfront);

	virtual void onRemove();

	/**
		父节点删除
	*/
	virtual void onParentRemove(CoordinateNode* pParentNode) {
	}

	/**
		当节点有变动时，需要更新它在list中的
		相关位置等信息
	*/
	virtual void update();

#ifdef _DEBUG
	void descr(const std::string& str) { descr_ = str; }
	virtual const char* descr() { return descr_.c_str(); }
#else
	void descr(const std::string& str){}
	virtual const char* descr(){ return ""; }
#endif

protected:
	// 链表的前端和后端
	CoordinateNode* pPrevX_;
	CoordinateNode* pNextX_;
	CoordinateNode* pPrevY_;
	CoordinateNode* pNextY_;
	CoordinateNode* pPrevZ_;
	CoordinateNode* pNextZ_;

	CoordinateSystem* pCoordinateSystem_;

	float x_, y_, z_;
	float old_xx_, old_yy_, old_zz_;

	int8 weight_;

#ifdef _DEBUG
	std::string descr_;
#endif

	uint32 flags_;
};

}

#ifdef CODE_INLINE
#include "coordinate_node.inl"
#endif
#endif
