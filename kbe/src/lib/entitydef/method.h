// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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
#include "entitycallabstract.h"
#include "pyscript/scriptobject.h"	


namespace KBEngine{

class MethodDescription
{
public:
	// ��¶����������
	enum EXPOSED_TYPE
	{
		// Ĭ�ϣ��Ǳ�¶����
		NO_EXPOSED = 0,

		// Ĭ�ϣ��ű��������Բ��ӵ����߲���
		EXPOSED = 1,

		// �ű�������һ������Ϊ������ID���ṩ�ű��������ߺϷ���
		EXPOSED_AND_CALLER_CHECK = 2
	};

public:	
	MethodDescription(ENTITY_METHOD_UID utype, COMPONENT_ID domain,
		std::string name,
		EXPOSED_TYPE exposedType = NO_EXPOSED);

	virtual ~MethodDescription();
	
	INLINE const char* getName(void) const;

	INLINE ENTITY_METHOD_UID getUType(void) const;
	INLINE void setUType(ENTITY_METHOD_UID muid);

	static uint32 getDescriptionCount(void){ return methodDescriptionCount_; }
	static void resetDescriptionCount(void){ methodDescriptionCount_ = 0; }

	INLINE EXPOSED_TYPE isExposed(void) const;

	void setExposed(EXPOSED_TYPE type = EXPOSED);

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

	EXPOSED_TYPE							exposedType_;								// �Ƿ���һ����¶����

	int16									aliasID_;									// ����id�� ����¶�ķ������߹㲥�������ܸ���С��255ʱ�� ���ǲ�ʹ��utype��ʹ��1�ֽڵ�aliasID������
};

}

#ifdef CODE_INLINE
#include "method.inl"
#endif
#endif // KBENGINE_DEF_METHOD_H
