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
#define RANGENODE_FLAG_ENTITY				0x00000001		// һ��Entity�ڵ�
#define RANGENODE_FLAG_TRIGGER				0x00000002		// һ���������ڵ�
#define RANGENODE_FLAG_HIDE					0x00000004		// ���ؽڵ�(�����ڵ㲻�ɼ�)
#define RANGENODE_FLAG_REMOVE				0x00000008		// ɾ���ڵ�
#define RANGENODE_FLAG_PENDING				0x00000010		// ����ڵ㴦��update�����С�

#define RANGENODE_FLAG_HIDE_OR_REMOVE		(RANGENODE_FLAG_REMOVE | RANGENODE_FLAG_HIDE)

class RangeList;
class RangeNode
{
public:
	RangeNode(RangeList* pRangeList = NULL);
	virtual ~RangeNode();
	
	INLINE void flags(uint32 v);
	INLINE uint32 flags()const;

	/**
		x && z�ɲ�ͬ��Ӧ��ʵ��(�Ӳ�ͬ����ȡ)
	*/
	virtual float x()const { return x_; }
	virtual float y()const { return y_; }
	virtual float z()const { return z_; }

	virtual void x(float v){ x_ = v; }
	virtual void y(float v){ y_ = v; }
	virtual void z(float v){ z_ = v; }

	void old_x(float v) { old_x_ = v; }
	void old_y(float v) { old_y_ = v; }
	void old_z(float v) { old_z_ = v; }

	float old_x()const { return old_x_; }
	float old_y()const { return old_y_; }
	float old_z()const { return old_z_; }

	virtual void resetOld(){ 
		old_x_ = x();
		old_y_ = y();
		old_z_ = z();
	}

	void c_str();

	void debugX();
	void debugY();
	void debugZ();

	INLINE void pRangeList(RangeList* p);
	INLINE RangeList* pRangeList()const;

	/**
		��ȡ�����ǰ���ָ��
	*/
	INLINE RangeNode* pPrevX()const;
	INLINE RangeNode* pNextX()const;
	INLINE RangeNode* pPrevY()const;
	INLINE RangeNode* pNextY()const;
	INLINE RangeNode* pPrevZ()const;
	INLINE RangeNode* pNextZ()const;

	INLINE RangeNode* pPassPrevX(uint32 flags)const;
	INLINE RangeNode* pPassNextX(uint32 flags)const;
	INLINE RangeNode* pPassPrevY(uint32 flags)const;
	INLINE RangeNode* pPassNextY(uint32 flags)const;
	INLINE RangeNode* pPassPrevZ(uint32 flags)const;
	INLINE RangeNode* pPassNextZ(uint32 flags)const;

	/**
		���������ǰ���ָ��
	*/
	INLINE void pPrevX(RangeNode* pNode);
	INLINE void pNextX(RangeNode* pNode);
	INLINE void pPrevY(RangeNode* pNode);
	INLINE void pNextY(RangeNode* pNode);
	INLINE void pPrevZ(RangeNode* pNode);
	INLINE void pNextZ(RangeNode* pNode);

	/**
		ĳ���ڵ�䶯�����˱��ڵ�
		@isfront: ��ǰ�ƶ���������ƶ�
	*/
	virtual void onNodePassX(RangeNode* pNode, bool isfront);
	virtual void onNodePassY(RangeNode* pNode, bool isfront);
	virtual void onNodePassZ(RangeNode* pNode, bool isfront);

	virtual void onRemove();

	/**
		���ڵ�ɾ��
	*/
	virtual void onParentRemove(RangeNode* pParentNode){}

	/**
		���ڵ��б䶯ʱ����Ҫ��������list�е�
		���λ�õ���Ϣ
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
	// �����ǰ�˺ͺ��
	RangeNode* pPrevX_;
	RangeNode* pNextX_;
	RangeNode* pPrevY_;
	RangeNode* pNextY_;
	RangeNode* pPrevZ_;
	RangeNode* pNextZ_;

	RangeList* pRangeList_;

	float x_, y_, z_;
	float old_x_, old_y_, old_z_;

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
