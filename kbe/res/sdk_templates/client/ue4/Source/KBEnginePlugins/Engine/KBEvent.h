// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "KBECommon.h"
#include "KBEventTypes.h"
#include "KBEvent.generated.h"


/*
事件模块
事件的数据基础类， 不同事件需要实现不同的数据类
*/
UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData : public UObject
{
	GENERATED_BODY()

public:
	// 事件名称，可用于对事件类型进行判断，该名称由事件触发时事件系统进行填充
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString eventName;
};

/*
	事件模块
	KBEngine插件层与Unity3D表现层通过事件来交互
*/
class KBENGINEPLUGINS_API KBEvent
{
public:
	KBEvent();
	virtual ~KBEvent();
	
public:
		static bool registerEvent(const FString& eventName, const FString& funcName, TFunction<void(const UKBEventData*)> func, void* objPtr);
		static bool deregister(void* objPtr, const FString& eventName, const FString& funcName);
		static bool deregister(void* objPtr);

		static void fire(const FString& eventName, UKBEventData* pEventData);

		static void clear();
		static void clearFiredEvents();

		static void processInEvents() {}
		static void processOutEvents() {}

		static void pause();
		static void resume();

		static void removeFiredEvent(void* objPtr, const FString& eventName = TEXT(""), const FString& funcName = TEXT(""));

protected:
	struct EventObj
	{
		TFunction<void(const UKBEventData*)> method;
		FString funcName;
		void* objPtr;
	};

	struct FiredEvent
	{
		EventObj evt;
		FString eventName;
		UKBEventData* args;
	};

	static TMap<FString, TArray<EventObj>> events_;
	static TArray<FiredEvent*> firedEvents_;
	static bool isPause_;
};


// 注册事件
#define KBENGINE_REGISTER_EVENT(EVENT_NAME, EVENT_FUNC) \
	KBEvent::registerEvent(EVENT_NAME, #EVENT_FUNC, [this](const UKBEventData* pEventData) {	EVENT_FUNC(pEventData);	}, (void*)this);

// 注册事件，可重写事件函数
#define KBENGINE_REGISTER_EVENT_OVERRIDE_FUNC(EVENT_NAME, FUNC_NAME, EVENT_FUNC) \
	KBEvent::registerEvent(EVENT_NAME, FUNC_NAME, EVENT_FUNC, (void*)this);

// 注销这个对象某个事件
#define KBENGINE_DEREGISTER_EVENT_BY_FUNCNAME(EVENT_NAME, FUNC_NAME) KBEvent::deregister((void*)this, EVENT_NAME, FUNC_NAME);
#define KBENGINE_DEREGISTER_EVENT(EVENT_NAME) KBEvent::deregister((void*)this, EVENT_NAME, TEXT(""));

// 注销这个对象所有的事件
#define KBENGINE_DEREGISTER_ALL_EVENT()	KBEvent::deregister((void*)this);

// fire event
#define KBENGINE_EVENT_FIRE(EVENT_NAME, EVENT_DATA) KBEvent::fire(EVENT_NAME, EVENT_DATA);

// 暂停事件
#define KBENGINE_EVENT_PAUSE() KBEvent::pause();

// 恢复事件
#define KBENGINE_EVENT_RESUME() KBEvent::resume();

// 清除所有的事件
#define KBENGINE_EVENT_CLEAR() KBEvent::clear();

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_Baseapp_importClientMessages : public UKBEventData
{
	GENERATED_BODY()

public:
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onKicked : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int32 failedcode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString errorStr;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_createAccount : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString username;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	TArray<uint8> datas;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_login : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString username;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	TArray<uint8> datas;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_logout : public UKBEventData
{
	GENERATED_BODY()

public:
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onLoginFailed : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int32 failedcode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString errorStr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	TArray<uint8> serverdatas;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onLoginBaseapp : public UKBEventData
{
	GENERATED_BODY()

public:
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onLoginSuccessfully : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, /*BlueprintReadWrite, No support*/ Category = KBEngine)
	uint64  entity_uuid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int32 entity_id;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onReloginBaseapp : public UKBEventData
{
	GENERATED_BODY()

public:
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onLoginBaseappFailed : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int32 failedcode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString errorStr;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onReloginBaseappFailed : public UKBEventData
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int32 failedcode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString errorStr;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onReloginBaseappSuccessfully : public UKBEventData
{
	GENERATED_BODY()

public:
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onVersionNotMatch : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString clientVersion;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString serverVersion;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onScriptVersionNotMatch : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString clientScriptVersion;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString serverScriptVersion;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_Loginapp_importClientMessages : public UKBEventData
{
	GENERATED_BODY()

public:
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_Baseapp_importClientEntityDef : public UKBEventData
{
	GENERATED_BODY()

public:
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onControlled : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int entityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool isControlled;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onLoseControlledEntity : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int entityID;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_updatePosition : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FVector position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FRotator direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int entityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	float moveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool isOnGround;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_set_position : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FVector position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int entityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	float moveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool isOnGround;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_set_direction : public UKBEventData
{
	GENERATED_BODY()

public:
	// roll, pitch, yaw
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FRotator direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int entityID;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onCreateAccountResult : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = KBEngine, BlueprintReadWrite, EditAnywhere)
	int errorCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString errorStr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	TArray<uint8> datas;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_addSpaceGeometryMapping : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString spaceResPath;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onSetSpaceData : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int spaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString value;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onDelSpaceData : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int spaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString key;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onDisconnected : public UKBEventData
{
	GENERATED_BODY()

public:
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onConnectionState : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool success;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString address;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onEnterWorld : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString entityClassName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int spaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int entityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString res;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FVector position;

	// roll, pitch, yaw
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FVector direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	float moveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool isOnGround;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool isPlayer;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onLeaveWorld : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int spaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int entityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool isPlayer;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onEnterSpace : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString entityClassName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int spaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int entityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString res;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FVector position;

	// roll, pitch, yaw
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FVector direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	float moveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool isOnGround;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool isPlayer;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onLeaveSpace : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int spaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int entityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool isPlayer;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_resetPassword : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString username;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onResetPassword : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int32 failedcode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString errorStr;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_bindAccountEmail : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString email;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onBindAccountEmail : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int32 failedcode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString errorStr;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_newPassword : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString old_password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString new_password;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onNewPassword : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int32 failedcode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString errorStr;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onStreamDataStarted : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int resID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int dataSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString dataDescr;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onStreamDataRecv : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int resID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	TArray<uint8> data;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onStreamDataCompleted : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int resID;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onImportClientSDK : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int remainingFiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int fileSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString fileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	TArray<uint8> fileDatas;
};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onImportClientSDKSuccessfully : public UKBEventData
{
	GENERATED_BODY()

public:

};

UCLASS(Blueprintable, BlueprintType)
class KBENGINEPLUGINS_API UKBEventData_onDownloadSDK : public UKBEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool isDownload;
};

