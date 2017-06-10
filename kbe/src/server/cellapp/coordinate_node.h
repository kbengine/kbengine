/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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

#ifndef KBE_COORDINATE_NODE_H
#define KBE_COORDINATE_NODE_H

#include "helper/debug_helper.h"
#include "common/common.h"	

namespace KBEngine{

#define COORDINATE_NODE_FLAG_UNKNOWN				0x00000000
#define COORDINATE_NODE_FLAG_ENTITY					0x00000001		// һ��Entity�ڵ�
#define COORDINATE_NODE_FLAG_TRIGGER				0x00000002		// һ���������ڵ�
#define COORDINATE_NODE_FLAG_HIDE					0x00000004		// ���ؽڵ�(�����ڵ㲻�ɼ�)
#define COORDINATE_NODE_FLAG_REMOVING				0x00000008		// ɾ���еĽڵ�
#define COORDINATE_NODE_FLAG_REMOVED				0x00000010		// ɾ���ڵ�
#define COORDINATE_NODE_FLAG_PENDING				0x00000020		// ����ڵ㴦��update�����С�
#define COORDINATE_NODE_FLAG_ENTITY_NODE_UPDATING	0x00000040		// entity�ڵ�����ִ��update����
#define COORDINATE_NODE_FLAG_INSTALLING				0x00000080		// �ڵ����ڰ�װ����
#define COORDINATE_NODE_FLAG_POSITIVE_BOUNDARY		0x00000100		// �ڵ��Ǵ����������߽�
#define COORDINATE_NODE_FLAG_NEGATIVE_BOUNDARY		0x00000200		// �ڵ��Ǵ������ĸ��߽�

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
		(�ڵ㱾�������)
		x && z�ɲ�ͬ��Ӧ��ʵ��(�Ӳ�ͬ����ȡ)
	*/
	virtual float x() const { return x_; }
	virtual float y() const { return y_; }
	virtual float z() const { return z_; }

	virtual void x(float v) { x_ = v; }
	virtual void y(float v) { y_ = v; }
	virtual void z(float v) { z_ = v; }

	/**
		(��չ����)
		x && z�ɲ�ͬ��Ӧ��ʵ��(�Ӳ�ͬ����ȡ)
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
		��ȡ�����ǰ���ָ��
	*/
	INLINE CoordinateNode* pPrevX() const;
	INLINE CoordinateNode* pNextX() const;
	INLINE CoordinateNode* pPrevY() const;
	INLINE CoordinateNode* pNextY() const;
	INLINE CoordinateNode* pPrevZ() const;
	INLINE CoordinateNode* pNextZ() const;

	/**
		���������ǰ���ָ��
	*/
	INLINE void pPrevX(CoordinateNode* pNode);
	INLINE void pNextX(CoordinateNode* pNode);
	INLINE void pPrevY(CoordinateNode* pNode);
	INLINE void pNextY(CoordinateNode* pNode);
	INLINE void pPrevZ(CoordinateNode* pNode);
	INLINE void pNextZ(CoordinateNode* pNode);

	/**
		ĳ���ڵ�䶯�����˱��ڵ�
		@isfront: ��ǰ�ƶ���������ƶ�
	*/
	virtual void onNodePassX(CoordinateNode* pNode, bool isfront);
	virtual void onNodePassY(CoordinateNode* pNode, bool isfront);
	virtual void onNodePassZ(CoordinateNode* pNode, bool isfront);

	virtual void onRemove();

	/**
		���ڵ�ɾ��
	*/
	virtual void onParentRemove(CoordinateNode* pParentNode) {
	}

	/**
		���ڵ��б䶯ʱ����Ҫ��������list�е�
		���λ�õ���Ϣ
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
	// �����ǰ�˺ͺ��
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
