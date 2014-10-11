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

#ifndef KBE_CLIENTAPP_ENTITY_HPP
#define KBE_CLIENTAPP_ENTITY_HPP
	
// common include
#include "entity_aspect.hpp"
#include "client_lib/profile.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "pyscript/math.hpp"
#include "pyscript/scriptobject.hpp"
#include "entitydef/datatypes.hpp"	
#include "entitydef/entitydef.hpp"	
#include "entitydef/scriptdef_module.hpp"
#include "entitydef/entity_macro.hpp"	
#include "server/script_timers.hpp"	

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{
class EntityMailbox;
class ClientObjectBase;

namespace Mercury
{
class Channel;
}

namespace client
{

class Entity : public script::ScriptObject
{
	/** ���໯ ��һЩpy�������������� */
	BASE_SCRIPT_HREADER(Entity, ScriptObject)	
	ENTITY_HEADER(Entity)
public:
	Entity(ENTITY_ID id, const ScriptDefModule* scriptModule, EntityMailbox* base, EntityMailbox* cell);
	~Entity();
	
	/** 
		�����������ݱ��ı��� 
	*/
	void onDefDataChanged(const PropertyDescription* propertyDescription, 
			PyObject* pyData);
	
	/** 
		mailbox section
	*/
	INLINE EntityMailbox* baseMailbox()const;
	DECLARE_PY_GET_MOTHOD(pyGetBaseMailbox);
	INLINE void baseMailbox(EntityMailbox* mailbox);
	
	INLINE EntityMailbox* cellMailbox()const;
	DECLARE_PY_GET_MOTHOD(pyGetCellMailbox);
	INLINE void cellMailbox(EntityMailbox* mailbox);

	/** 
		�ű���ȡ������entity��position 
	*/
	INLINE Position3D& position();
	INLINE Position3D& serverPosition();
	INLINE void position(const Position3D& pos);
	INLINE void serverPosition(const Position3D& pos);
	void onPositionChanged();
	DECLARE_PY_GETSET_MOTHOD(pyGetPosition, pySetPosition);

	/** 
		�ű���ȡ������entity�ķ��� 
	*/
	INLINE Direction3D& direction();
	INLINE void direction(const Direction3D& dir);
	void onDirectionChanged();
	DECLARE_PY_GETSET_MOTHOD(pyGetDirection, pySetDirection);
	
	/**
		�ƶ��ٶ�
	*/
	INLINE void moveSpeed(float speed);
	INLINE float moveSpeed()const;
	void onMoveSpeedChanged();
	DECLARE_PY_GETSET_MOTHOD(pyGetMoveSpeed, pySetMoveSpeed);

	/** 
		pClientApp section
	*/
	DECLARE_PY_GET_MOTHOD(pyGetClientApp);
	INLINE void pClientApp(ClientObjectBase* p);
	INLINE ClientObjectBase* pClientApp()const;
	
	const EntityAspect* getAspect()const{ return &aspect_; }
	/** 
		�������entity 
	*/
	void onDestroy(bool callScript){};

	void onEnterWorld();
	void onLeaveWorld();

	void onEnterSpace();
	void onLeaveSpace();

	/**
		Զ�̺��б�entity�ķ��� 
	*/
	void onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s);

	/**
		����������entity����
	*/
	void onUpdatePropertys(MemoryStream& s);

	bool isEnterword()const{ return enterword_; }

	void onBecomePlayer();
	void onBecomeNonPlayer();
	
	bool isOnGound()const { return isOnGound_;}
	void isOnGound(bool v) { isOnGound_ = v;}
protected:
	EntityMailbox*							cellMailbox_;						// ���entity��cell-mailbox
	EntityMailbox*							baseMailbox_;						// ���entity��base-mailbox

	Position3D								position_, serverPosition_;			// entity�ĵ�ǰλ��
	Direction3D								direction_;							// entity�ĵ�ǰ����

	ClientObjectBase*						pClientApp_;

	EntityAspect							aspect_;

	float									velocity_;

	bool									enterword_;							// �Ƿ��Ѿ�enterworld�ˣ� restoreʱ����
	
	bool									isOnGound_;
};																										

}
}

#ifdef CODE_INLINE
#include "entity.ipp"
#endif
#endif
