// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBVar.h"
#include "KBECommon.h"

class EntityCall;


/*
	KBEngine逻辑层的实体基础类
	所有扩展出的游戏实体都应该继承于该模块

	要实现一个KBE对应的实体必须经过以下几步
	1: 在服务器entity_defs中entities.xml中注册实体并实现实体的def定义
	2: 在服务器的相关位置如：assets/scripts目录的cell（ 取决于实体拥有该部分）或base（ 取决于实体拥有该部分）文件夹下实现实体的服务器部分py脚本模块
	3: 在UE4客户端中kbe_scripts文件夹下实现实体的客户端部分脚本模块（这里统一称为实体脚本，虽然是C++实现），UE4实现实体后必须有如下几步
		A：在实体的头文件中按照格式定义ENTITYDEF_DECLARE_模块名（用于声明实体def相关属性和方法用于后续自动化协议绑定）， 具体看demo
		B：在实体CPP文件中ENTITYDEF_CLASS_REGISTER将实体注册（用于告诉插件客户端具体实现了哪些实体），具体看demo
*/
class KBENGINEPLUGINS_API Entity
{
public:
	Entity();
	virtual ~Entity();

	virtual void __init__();

	static void clear();

public:
	const FString& className() const {
		return className_;
	}

	void className(const FString& v) {
		className_ = v;
	}

	bool inWorld() const {
		return inWorld_;
	}

	void inWorld(bool v) {
		inWorld_ = v;
	}

	bool isControlled() const {
		return isControlled_;
	}

	void isControlled(bool v) {
		isControlled_ = v;
	}

	bool inited() const {
		return inited_;
	}

	void inited(bool v) {
		inited_ = v;
	}

	ENTITY_ID id() const {
		return id_;
	}

	void id(ENTITY_ID v) {
		id_ = v;
	}

	EntityCall* base() const {
		return base_;
	}

	void base(EntityCall* v);

	EntityCall* cell() const {
		return cell_;
	}

	void cell(EntityCall* v);

	float velocity() const {
		return velocity_;
	}

	void velocity(float v) {
		velocity_ = v;
	}

	bool isPlayer();

	void isOnGround(bool v) {
		isOnGround_ = v;
	}

	bool isOnGround() const {
		return isOnGround_;
	}

public:
	virtual void callPropertysSetMethods();

	virtual void onDestroy()
	{
	}

	void enterWorld();
	virtual void onEnterWorld();
	void leaveWorld();
	virtual void onLeaveWorld();

	virtual void enterSpace();
	virtual void onEnterSpace();
	virtual void leaveSpace();
	virtual void onLeaveSpace();

	// This callback method is called when the local entity control by the client has been enabled or disabled. 
	// See the Entity.controlledBy() method in the CellApp server code for more infomation.
	// 对于玩家自身来说，它表示是否自己被其它玩家控制了；
	// 对于其它entity来说，表示我本机是否控制了这个entity
	virtual void onControlled(bool isControlled)
	{

	}

	virtual void onUpdateVolatileData()
	{
	}

	virtual void set_position(const FVector& old);
	virtual void set_direction(const FVector& old);

public:
	void baseCall(FString methodName, const TArray<KBVar*>& arguments = TArray<KBVar*>());
	void baseCall(FString methodName, KBVar arg1);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18, KBVar arg19);
	void baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10, 
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18, KBVar arg19, KBVar arg20);

	void cellCall(FString methodName, const TArray<KBVar*>& arguments = TArray<KBVar*>());
	void cellCall(FString methodName, KBVar arg1);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18, KBVar arg19);
	void cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
		KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18, KBVar arg19, KBVar arg20);

protected:
	ENTITY_ID id_;
	FString className_;

	bool isOnGround_;

	EntityCall* base_;
	EntityCall* cell_;

	// enterworld之后设置为true
	bool inWorld_;

	// 对于玩家自身来说，它表示是否自己被其它玩家控制了；
	// 对于其它entity来说，表示我本机是否控制了这个entity
	bool isControlled_;

	bool inited_;

	float velocity_;

public:
	FVector position;
	FVector direction;
	uint32 spaceID;

	// 当前玩家最后一次同步到服务端的位置与朝向
	// 这两个属性是给引擎KBEngine.cs用的，别的地方不要修改
	FVector entityLastLocalPos;
	FVector entityLastLocalDir;
};

/*
	用于在需要创建某个实体时被插件底层调用创建。
	每添加一个新的实体必须在源代码中加入宏 KBENGINE_ENTITY_CLASS_REGISTER(XEntity)，以便底层注册一个相关的EntityCreator
*/
class KBENGINEPLUGINS_API EntityCreator
{
public:
	EntityCreator(const FString& scriptName);

	virtual ~EntityCreator();

	virtual Entity* create() = 0;

	virtual FString parentClasses()
	{	
		return FString(TEXT("Entity"));	
	}	
};

/*
	实体创建工厂类
*/
class KBENGINEPLUGINS_API EntityFactory
{
public:
	EntityFactory();
	~EntityFactory();

	static void initialize();
	static void finishDefs(const FString& scriptName, const FString& parentScriptName);

	static Entity* create(const FString& scriptName);
	EntityCreator* addEntityCreator(const FString& scriptName, EntityCreator* pEntityCreator);

	static EntityCreator* findCreator(const FString& scriptName);

	static EntityFactory& getSingleton() {
		if (!pEntityFactory)
			pEntityFactory = new EntityFactory();
		return *pEntityFactory;
	}

	TMap<FString, EntityCreator*> creators;

	static EntityFactory* pEntityFactory;
};

/*
	包装实体的defined方法
*/
class KBENGINEPLUGINS_API EntityDefMethodHandle
{
public:
	EntityDefMethodHandle(const FString& scriptName, const FString& defMethodName);

	virtual ~EntityDefMethodHandle();

	virtual void callMethod(void* pEntity, const KBVar& arg)
	{

	}

	virtual void callMethod(void* pEntity, const TArray<KBVar*>& args) = 0;
};

class KBENGINEPLUGINS_API EntityDefMethodHandles
{
public:
	EntityDefMethodHandles();
	~EntityDefMethodHandles();

	EntityDefMethodHandle* add(const FString& scriptName, const FString& defMethodName, EntityDefMethodHandle* pEntityDefMethodHandle);
	static EntityDefMethodHandle* find(const FString& scriptName, const FString& defMethodName);

	static EntityDefMethodHandles& getSingleton() {
		if (!pEntityDefMethodHandles)
			pEntityDefMethodHandles = new EntityDefMethodHandles();
		return *pEntityDefMethodHandles;
	}

	static EntityDefMethodHandles* pEntityDefMethodHandles;

	TMap<FString, TMap<FString, EntityDefMethodHandle*>> defMethodHandles;
};

/*
	包装实体的defined属性
*/
class KBENGINEPLUGINS_API EntityDefPropertyHandle
{
public:
	EntityDefPropertyHandle(const FString& scriptName, const FString& defPropertyName);

	virtual ~EntityDefPropertyHandle();

	virtual void setPropertyValue(void* pEntity, KBVar* val) = 0;
	virtual KBVar* getPropertyValue(void* pEntity) = 0;

protected:
	void setPropertyValue_(int8* out, KBVar* val) { *out = (int8)(*val); }
	void setPropertyValue_(int16* out, KBVar* val) { *out = (int16)(*val); }
	void setPropertyValue_(int32* out, KBVar* val) { *out = (int32)(*val); }
	void setPropertyValue_(int64* out, KBVar* val) { *out = (int64)(*val); }
	void setPropertyValue_(uint8* out, KBVar* val) { *out = (uint8)(*val); }
	void setPropertyValue_(uint16* out, KBVar* val) { *out = (uint16)(*val); }
	void setPropertyValue_(uint32* out, KBVar* val) { *out = (uint32)(*val); }
	void setPropertyValue_(uint64* out, KBVar* val) { *out = (uint64)(*val); }
	void setPropertyValue_(FString* out, KBVar* val) { *out = val->GetValue<FString>(); }
	void setPropertyValue_(float* out, KBVar* val) { *out = (float)(*val); }
	void setPropertyValue_(double* out, KBVar* val) { *out = (double)(*val); }
	void setPropertyValue_(bool* out, KBVar* val) { *out = (bool)(*val); }
	void setPropertyValue_(FVector* out, KBVar* val) { *out = (*val); }
	void setPropertyValue_(FVector2D* out, KBVar* val) { *out = (*val); }
	void setPropertyValue_(FVector4* out, KBVar* val) { *out = (*val); }
	void setPropertyValue_(ByteArray* out, KBVar* val) { *out = val->GetValue<ByteArray>(); }
	void setPropertyValue_(KBVar::KBVarMap* out, KBVar* val) { *out = (*val); }
	void setPropertyValue_(KBVar::KBVarArray* out, KBVar* val) { *out = val->GetValue<KBVar::KBVarArray>(); }
	void setPropertyValue_(KBVar* out, KBVar* val) { *out = (*val); }
};

class KBENGINEPLUGINS_API EntityDefPropertyHandles
{
public:
	EntityDefPropertyHandles();
	~EntityDefPropertyHandles();

	EntityDefPropertyHandle* add(const FString& scriptName, const FString& defPropertyName, EntityDefPropertyHandle* pEntityDefPropertyHandle);
	static EntityDefPropertyHandle* find(const FString& scriptName, const FString& defPropertyName);

	static EntityDefPropertyHandles& getSingleton() {
		if (!pEntityDefPropertyHandles)
			pEntityDefPropertyHandles = new EntityDefPropertyHandles();
		return *pEntityDefPropertyHandles;
	}

	static EntityDefPropertyHandles* pEntityDefPropertyHandles;

	TMap<FString, TMap<FString, EntityDefPropertyHandle*>> defPropertyHandles;
};



#if PLATFORM_WINDOWS
	#define K_BRACKET_L (
	#define K_BRACKET_R )

	#define K_PP_RSEQ_N \
		63,62,61,60, \
		59,58,57,56,55,54,53,52,51,50, \
		49,48,47,46,45,44,43,42,41,40, \
		39,38,37,36,35,34,33,32,31,30, \
		29,28,27,26,25,24,23,22,21,20, \
		19,18,17,16,15,14,13,12,11,10, \
		9,8,7,6,5,4,3,2,1,0

	#define K_PP_ARG_N( \
		_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
		_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
		_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
		_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
		_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
		_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
		_61,_62,_63,N,...) N

	#define K_PP_NARG(...) \
		K_PP_ARG_N K_BRACKET_L __VA_ARGS__, K_PP_RSEQ_N K_BRACKET_R

	#define K_CONNECT1(a, b) a##b
	#define K_CONNECT(a, b) K_CONNECT1(a, b)

	// 将name与参数个数组为一个，如K_EXPAND_NARG(name, 5, 6, 7, 8, 9)为name5
	#define K_EXPAND_NARG(name, ...) \
		K_CONNECT K_BRACKET_L name, K_PP_NARG(__VA_ARGS__) K_BRACKET_R (__VA_ARGS__)

#else

	#define K_PP_ARG_N( \
			_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
			_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
			_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
			_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
			_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
			_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
			_61,_62,_63,N,...) N

	#define K_PP_RSEQ_N() \
			63,62,61,60, \
			59,58,57,56,55,54,53,52,51,50, \
			49,48,47,46,45,44,43,42,41,40, \
			39,38,37,36,35,34,33,32,31,30, \
			29,28,27,26,25,24,23,22,21,20, \
			19,18,17,16,15,14,13,12,11,10, \
			9,8,7,6,5,4,3,2,1,0

	#define K_CONNECT(a, b) K_CONNECT1(a, b)
	#define K_CONNECT1(a, b) K_CONNECT2(a, b)
	#define K_CONNECT2(a, b) a##b

	#define K_PP_NARG(...) \
		K_PP_NARG_(__VA_ARGS__,K_PP_RSEQ_N())

	#define K_PP_NARG_(...) \
		K_PP_ARG_N(__VA_ARGS__)

	// 将name与参数个数组为一个，如K_EXPAND_NARG(name, 5, 6, 7, 8, 9)为name5
	#define K_EXPAND_NARG(name, ...) \
		K_CONNECT(name, K_PP_NARG(__VA_ARGS__))(__VA_ARGS__)

#endif

#define __XXX_ENTITYDEF_DECLARE__(...) K_EXPAND_NARG(ENTITYDEF_DECLARE_XXX, __VA_ARGS__)

#define ENTITYDEF_DECLARE_XXX2(ENTITY_SCRIPTMODULE_NAME, module1, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME)

#define ENTITYDEF_DECLARE_XXX3(ENTITY_SCRIPTMODULE_NAME, module1, module2, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 

#define ENTITYDEF_DECLARE_XXX4(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 

#define ENTITYDEF_DECLARE_XXX5(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 

#define ENTITYDEF_DECLARE_XXX6(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 

#define ENTITYDEF_DECLARE_XXX7(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 

#define ENTITYDEF_DECLARE_XXX8(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 

#define ENTITYDEF_DECLARE_XXX9(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 

#define ENTITYDEF_DECLARE_XXX10(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 

#define ENTITYDEF_DECLARE_XXX11(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, module10, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module10 (ENTITY_SCRIPTMODULE_NAME) 

#define ENTITYDEF_DECLARE_XXX12(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, module10, module11, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module10 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module11 (ENTITY_SCRIPTMODULE_NAME)

#define ENTITYDEF_DECLARE_XXX13(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, module10, module11, module12, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module10 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module11 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module12 (ENTITY_SCRIPTMODULE_NAME)

#define ENTITYDEF_DECLARE_XXX14(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, module10, module11, module12, module13, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module10 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module11 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module12 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module13 (ENTITY_SCRIPTMODULE_NAME)

#define ENTITYDEF_DECLARE_XXX15(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, module10, module11, module12, module13, module14, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module10 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module11 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module12 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module13 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module14 (ENTITY_SCRIPTMODULE_NAME)

#define ENTITYDEF_DECLARE_XXX16(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, module10, module11, module12, module13, module14, module15, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module10 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module11 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module12 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module13 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module14 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module15 (ENTITY_SCRIPTMODULE_NAME)

#define ENTITYDEF_DECLARE_XXX17(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, module10, module11, module12, module13, module14, module15, module16, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module10 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module11 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module12 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module13 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module14 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module15 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module16 (ENTITY_SCRIPTMODULE_NAME)

#define ENTITYDEF_DECLARE_XXX18(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, module10, module11, module12, module13, module14, module15, module16, module17, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module10 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module11 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module12 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module13 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module14 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module15 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module16 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module17 (ENTITY_SCRIPTMODULE_NAME)

#define ENTITYDEF_DECLARE_XXX19(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, module10, module11, module12, module13, module14, module15, module16, module17, module18, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module10 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module11 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module12 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module13 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module14 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module15 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module16 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module17 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module18 (ENTITY_SCRIPTMODULE_NAME)

#define ENTITYDEF_DECLARE_XXX20(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, module10, module11, module12, module13, module14, module15, module16, module17, module18, module19, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module10 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module11 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module12 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module13 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module14 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module15 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module16 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module17 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module18 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module19 (ENTITY_SCRIPTMODULE_NAME) 

#define ENTITYDEF_DECLARE_XXX21(ENTITY_SCRIPTMODULE_NAME, module1, module2, module3, module4, module5, module6, module7, module8, module9, module10, module11, module12, module13, module14, module15, module16, module17, module18, module19, module20, ...)	\
		ENTITYDEF_DECLARE_##module1 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module2 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module3 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module4 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module5 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module6 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module7 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module8 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module9 (ENTITY_SCRIPTMODULE_NAME) 	\
		ENTITYDEF_DECLARE_##module10 (ENTITY_SCRIPTMODULE_NAME)	\
		ENTITYDEF_DECLARE_##module11 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module12 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module13 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module14 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module15 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module16 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module17 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module18 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module19 (ENTITY_SCRIPTMODULE_NAME) \
		ENTITYDEF_DECLARE_##module20 (ENTITY_SCRIPTMODULE_NAME)

// 注册定义的实体类，当实体类没有继承任何def中的类时使用该宏
#define ENTITYDEF_CLASS_NO_DEFPARENT_REGISTER(ENTITY_SCRIPTMODULE_NAME)	\
	ENTITYDEF_DECLARE_Entity(ENTITY_SCRIPTMODULE_NAME)	\
	ENTITYDEF_DECLARE_##ENTITY_SCRIPTMODULE_NAME(ENTITY_SCRIPTMODULE_NAME)	\
	class _##ENTITY_SCRIPTMODULE_NAME##Creator : public EntityCreator {	\
		public:	\
			_##ENTITY_SCRIPTMODULE_NAME##Creator(const FString& scriptName):	\
			EntityCreator(scriptName)	\
			{	\
			}	\
			virtual ~_##ENTITY_SCRIPTMODULE_NAME##Creator()	\
			{	\
			}	\
			virtual Entity* create() override	\
			{	\
				return (Entity*)(new ENTITY_SCRIPTMODULE_NAME());	\
			}	\
	};\
	_##ENTITY_SCRIPTMODULE_NAME##Creator g_##ENTITY_SCRIPTMODULE_NAME##Creator(FString(TEXT(#ENTITY_SCRIPTMODULE_NAME)));	\


// 注册定义的实体类
#define ENTITYDEF_CLASS_REGISTER(ENTITY_SCRIPTMODULE_NAME, .../*The name of the parent classes*/)	\
	ENTITYDEF_DECLARE_Entity(ENTITY_SCRIPTMODULE_NAME)	\
	ENTITYDEF_DECLARE_##ENTITY_SCRIPTMODULE_NAME(ENTITY_SCRIPTMODULE_NAME)	\
	__XXX_ENTITYDEF_DECLARE__(ENTITY_SCRIPTMODULE_NAME, __VA_ARGS__)	\
	class _##ENTITY_SCRIPTMODULE_NAME##Creator : public EntityCreator {	\
		public:	\
			_##ENTITY_SCRIPTMODULE_NAME##Creator(const FString& scriptName):	\
			EntityCreator(scriptName)	\
			{	\
			}	\
			virtual ~_##ENTITY_SCRIPTMODULE_NAME##Creator()	\
			{	\
			}	\
			virtual Entity* create() override	\
			{	\
				return (Entity*)(new ENTITY_SCRIPTMODULE_NAME());	\
			}	\
			virtual FString parentClasses() override	\
			{	\
				return FString(#__VA_ARGS__);	\
			}	\
	};\
	_##ENTITY_SCRIPTMODULE_NAME##Creator g_##ENTITY_SCRIPTMODULE_NAME##Creator(FString(TEXT(#ENTITY_SCRIPTMODULE_NAME)));	\


// 注册定义的方法
#define ENTITYDEF_METHOD_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	class _##ENTITY_SCRIPTMODULE_NAME##_##DEF_METHOD_NAME##DefMethodHandle : public EntityDefMethodHandle {	\
		public:	\
			_##ENTITY_SCRIPTMODULE_NAME##_##DEF_METHOD_NAME##DefMethodHandle(const FString& scriptName, const FString& defMethodName):	\
			EntityDefMethodHandle(scriptName, defMethodName)	\
			{	\
			}	\
			virtual ~_##ENTITY_SCRIPTMODULE_NAME##_##DEF_METHOD_NAME##DefMethodHandle()	\
			{	\
			}	\
			virtual void callMethod(void* pEntity, const KBVar& arg) override	\
			{	\
				reinterpret_cast<ENTITY_SCRIPTMODULE_NAME*>(pEntity)->DEF_METHOD_NAME(arg);	\
				if (arg.GetError() > 0)	\
					arg.ErrorLog(TEXT(#ENTITY_SCRIPTMODULE_NAME":"#DEF_METHOD_NAME"(arg1) error!"));	\
			}	\
			virtual void callMethod(void* pEntity, const TArray<KBVar*>& args) override	\
			{	\
				reinterpret_cast<ENTITY_SCRIPTMODULE_NAME*>(pEntity)->DEF_METHOD_NAME(*args[0]);	\
				const KBVar& arg = *args[0];	\
				if (arg.GetError() > 0)	\
					arg.ErrorLog(TEXT(#ENTITY_SCRIPTMODULE_NAME":"#DEF_METHOD_NAME"(arg1) error!"));	\
			}	\
	};\
	_##ENTITY_SCRIPTMODULE_NAME##_##DEF_METHOD_NAME##DefMethodHandle g_##ENTITY_SCRIPTMODULE_NAME##_##DEF_METHOD_NAME##DefMethodHandle(FString(TEXT(#ENTITY_SCRIPTMODULE_NAME)), FString(TEXT(#DEF_METHOD_NAME)));	\


#define ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, CALLMETHOD)	\
	class _##ENTITY_SCRIPTMODULE_NAME##_##DEF_METHOD_NAME##DefMethodHandle : public EntityDefMethodHandle {	\
		public:	\
			_##ENTITY_SCRIPTMODULE_NAME##_##DEF_METHOD_NAME##DefMethodHandle(const FString& scriptName, const FString& defMethodName):	\
			EntityDefMethodHandle(scriptName, defMethodName)	\
			{	\
			}	\
			virtual ~_##ENTITY_SCRIPTMODULE_NAME##_##DEF_METHOD_NAME##DefMethodHandle()	\
			{	\
			}	\
			virtual void callMethod(void* pEntity, const TArray<KBVar*>& args) override	\
			{	\
				reinterpret_cast<ENTITY_SCRIPTMODULE_NAME*>(pEntity)->DEF_METHOD_NAME CALLMETHOD;	\
				for(int i=0; i<args.Num(); ++i)	\
				{	\
					const KBVar& arg = *args[0];	\
					if (arg.GetError() > 0)	\
						arg.ErrorLog(FString::Printf(TEXT(#ENTITY_SCRIPTMODULE_NAME":"#DEF_METHOD_NAME"(arg%d) error!"), i));	\
				}	\
			}	\
	};\
	_##ENTITY_SCRIPTMODULE_NAME##_##DEF_METHOD_NAME##DefMethodHandle g_##ENTITY_SCRIPTMODULE_NAME##_##DEF_METHOD_NAME##DefMethodHandle(FString(TEXT(#ENTITY_SCRIPTMODULE_NAME)), FString(TEXT(#DEF_METHOD_NAME)));	\

#define ENTITYDEF_METHOD_ARGS0_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, ())

#define ENTITYDEF_METHOD_ARGS1_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0]))

#define ENTITYDEF_METHOD_ARGS2_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1]))

#define ENTITYDEF_METHOD_ARGS3_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2]))

#define ENTITYDEF_METHOD_ARGS4_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3]))

#define ENTITYDEF_METHOD_ARGS5_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4]))

#define ENTITYDEF_METHOD_ARGS6_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5]))

#define ENTITYDEF_METHOD_ARGS7_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6]))

#define ENTITYDEF_METHOD_ARGS8_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7]))

#define ENTITYDEF_METHOD_ARGS9_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8]))

#define ENTITYDEF_METHOD_ARGS10_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8], *args[9]))

#define ENTITYDEF_METHOD_ARGS11_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8], *args[9], *args[10]))

#define ENTITYDEF_METHOD_ARGS12_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8], *args[9], *args[10], *args[11]))

#define ENTITYDEF_METHOD_ARGS13_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8], *args[9], *args[10], *args[11], *args[12]))

#define ENTITYDEF_METHOD_ARGS14_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8], *args[9], *args[10], *args[11], *args[12], *args[13]))

#define ENTITYDEF_METHOD_ARGS15_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8], *args[9], *args[10], *args[11], *args[12], *args[13], *args[14]))

#define ENTITYDEF_METHOD_ARGS16_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8], *args[9], *args[10], *args[11], *args[12], *args[13], *args[14], *args[15]))

#define ENTITYDEF_METHOD_ARGS17_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8], *args[9], *args[10], *args[11], *args[12], *args[13], *args[14], *args[15], *args[16]))

#define ENTITYDEF_METHOD_ARGS18_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8], *args[9], *args[10], *args[11], *args[12], *args[13], *args[14], *args[15], *args[16], *args[17]))

#define ENTITYDEF_METHOD_ARGS19_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8], *args[9], *args[10], *args[11], *args[12], *args[13], *args[14], *args[15], *args[16], *args[17], *args[18]))

#define ENTITYDEF_METHOD_ARGS20_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME)	\
	ENTITYDEF_METHOD_ARGS_TEMPLATE_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_METHOD_NAME, (*args[0], *args[1], *args[2], *args[3], *args[4], *args[5], *args[6], *args[7], *args[8], *args[9], *args[10], *args[11], *args[12], *args[13], *args[14], *args[15], *args[16], *args[17], *args[18], *args[19]))


// 注册定义的属性
#define ENTITYDEF_PROPERTY_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_PROPERTY_NAME)	\
	class _##ENTITY_SCRIPTMODULE_NAME##_##DEF_PROPERTY_NAME##DefPropertyHandle : public EntityDefPropertyHandle {	\
		public:	\
			_##ENTITY_SCRIPTMODULE_NAME##_##DEF_PROPERTY_NAME##DefPropertyHandle(const FString& scriptName, const FString& defPropertyName):	\
			EntityDefPropertyHandle(scriptName, defPropertyName)	\
			{	\
			}	\
			virtual ~_##ENTITY_SCRIPTMODULE_NAME##_##DEF_PROPERTY_NAME##DefPropertyHandle()	\
			{	\
			}	\
			virtual void setPropertyValue(void* pEntity, KBVar* val) override	\
			{	\
				setPropertyValue_(&reinterpret_cast<ENTITY_SCRIPTMODULE_NAME*>(pEntity)->DEF_PROPERTY_NAME, val);	\
			}	\
			virtual KBVar* getPropertyValue(void* pEntity) override	\
			{	\
				return new KBVar(reinterpret_cast<ENTITY_SCRIPTMODULE_NAME*>(pEntity)->DEF_PROPERTY_NAME);	\
			}	\
	};\
	_##ENTITY_SCRIPTMODULE_NAME##_##DEF_PROPERTY_NAME##DefPropertyHandle g_##ENTITY_SCRIPTMODULE_NAME##_##DEF_PROPERTY_NAME##DefPropertyHandle(FString(TEXT(#ENTITY_SCRIPTMODULE_NAME)), FString(TEXT(#DEF_PROPERTY_NAME)));	\

// 注册定义的属性，并且属性包含一个set_*方法，属性被更新后改方法被调用
#define ENTITYDEF_PROPERTY_WITH_SETMETHOD_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_PROPERTY_NAME)	\
	ENTITYDEF_PROPERTY_REGISTER(ENTITY_SCRIPTMODULE_NAME, DEF_PROPERTY_NAME)	\
	ENTITYDEF_METHOD_REGISTER(ENTITY_SCRIPTMODULE_NAME, set_##DEF_PROPERTY_NAME)	\



#define ENTITYDEF_DECLARE_Entity(PARENT_MODULE)	\
	ENTITYDEF_PROPERTY_WITH_SETMETHOD_REGISTER(PARENT_MODULE, position)	\
	ENTITYDEF_PROPERTY_WITH_SETMETHOD_REGISTER(PARENT_MODULE, direction)	\
	ENTITYDEF_PROPERTY_REGISTER(PARENT_MODULE, spaceID)







