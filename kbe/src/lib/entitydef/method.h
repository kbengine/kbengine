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


#ifndef KBENGINE_DEF_METHOD_H
#define KBENGINE_DEF_METHOD_H

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif

#include "datatype.h"
#include "datatypes.h"
#include "helper/debug_helper.h"
#include "network/packet.h"
#include "entitymailboxabstract.h"
#include "pyscript/scriptobject.h"	


namespace KBEngine{

class MethodDescription
{
public:	
	MethodDescription(ENTITY_METHOD_UID utype, COMPONENT_ID domain,
		std::string name, 
		bool isExposed = false);

	virtual ~MethodDescription();
	
	INLINE const char* getName(void) const;

	INLINE ENTITY_METHOD_UID getUType(void) const;
	INLINE void setUType(ENTITY_METHOD_UID muid);

	static uint32 getDescriptionCount(void){ return methodDescriptionCount_; }
	static void resetDescriptionCount(void){ methodDescriptionCount_ = 0; }

	INLINE bool isExposed(void) const;

	void setExposed(void);

	bool pushArgType(DataType* dataType);

	INLINE std::vector<DataType*>& getArgTypes(void);

	size_t getArgSize(void);
	
	/** 
		���һ��call�Ƿ�Ϸ� 
	*/
	bool checkArgs(PyObject* args);		
	
	/** 
		��ÿ�����������ӵ����� 
		��������������Ϣ����������ڽű�������ʱ�ﴫ��Ĳ��� 
	*/
	void addToStream(MemoryStream* mstream, PyObject* args);

	/** 
		��һ��call����� ������һ��PyObject���͵�args 
	*/
	PyObject* createFromStream(MemoryStream* mstream);
	
	/** 
		����һ������ 
	*/
	PyObject* call(PyObject* func, PyObject* args);	

	INLINE void currCallerID(ENTITY_ID eid);

	INLINE COMPONENT_ID domain() const;

	INLINE bool isClient() const;
	INLINE bool isCell() const;
	INLINE bool isBase() const;

	/** 
		����id�� ����¶�ķ������߹㲥�������ܸ���С��255ʱ
		���ǲ�ʹ��utype��ʹ��1�ֽڵ�aliasID������
	*/
	INLINE int16 aliasID() const;
	INLINE uint8 aliasIDAsUint8() const;
	INLINE void aliasID(int16 v);
	
protected:
	static uint32							methodDescriptionCount_;					// ���е���������������

	COMPONENT_ID							methodDomain_;

	std::string								name_;										// �������������
	ENTITY_METHOD_UID						utype_;										// ���������������� ���������ϴ���ʶ��

	std::vector<DataType*>					argTypes_;									// ������ԵĲ�������б�

	bool									isExposed_;									// �Ƿ���һ����¶����

	ENTITY_ID								currCallerID_;								// ��ǰ������������ĵ�����ID, �ṩ��¶��������ʱ���ű��жϵ���Դ��ֹ����

	int16									aliasID_;									// ����id�� ����¶�ķ������߹㲥�������ܸ���С��255ʱ�� ���ǲ�ʹ��utype��ʹ��1�ֽڵ�aliasID������
};

}

#ifdef CODE_INLINE
#include "method.inl"
#endif
#endif // KBENGINE_DEF_METHOD_H
