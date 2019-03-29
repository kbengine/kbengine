// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "KBVar.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "KBECommon.generated.h"

namespace KBEngine
{

DECLARE_LOG_CATEGORY_EXTERN(LogKBEngine, Log, All);

}
#define KBE_ASSERT check

typedef uint16 MessageID;
typedef uint16 MessageLength;
typedef uint32 MessageLengthEx;
typedef int32 ENTITY_ID;
typedef uint32 SPACE_ID;
typedef uint64 DBID;
typedef TArray<uint8> ByteArray;
typedef TMap<FString, KBEngine::KBVar> KB_FIXED_DICT;
typedef TArray<KBEngine::KBVar> KB_ARRAY;

#define KBE_FLT_MAX FLT_MAX

/** 安全的释放一个指针内存 */
#define KBE_SAFE_RELEASE(i)									\
	if (i)													\
		{													\
			delete i;										\
			i = NULL;										\
		}

/** 安全的释放一个指针数组内存 */
#define KBE_SAFE_RELEASE_ARRAY(i)							\
	if (i)													\
		{													\
			delete[] i;										\
			i = NULL;										\
		}

USTRUCT(BlueprintType)
struct FKServerErr
{
	GENERATED_USTRUCT_BODY()

	FKServerErr()
	: name()
	, descr()
	, id(0)
	{
	}

	UPROPERTY(Category = ServerErr, BlueprintReadWrite, EditAnywhere)
	FString name;

	UPROPERTY(Category = ServerErr, BlueprintReadWrite, EditAnywhere)
	FString descr;

	UPROPERTY(Category = ServerErr, BlueprintReadWrite, EditAnywhere)
	int32 id;
};

// 客户端的类别
// http://www.kbengine.org/docs/programming/clientsdkprogramming.html
// http://www.kbengine.org/cn/docs/programming/clientsdkprogramming.html
UENUM(BlueprintType)
enum class EKCLIENT_TYPE : uint8
{
	CLIENT_TYPE_UNKNOWN		UMETA(DisplayName = "unknown"),
	CLIENT_TYPE_MOBILE		UMETA(DisplayName = "Mobile"),
	CLIENT_TYPE_WIN			UMETA(DisplayName = "Windows"),
	CLIENT_TYPE_LINUX		UMETA(DisplayName = "Linux"),
	CLIENT_TYPE_MAC			UMETA(DisplayName = "Mac"),
	CLIENT_TYPE_BROWSER		UMETA(DisplayName = "Browser"),
	CLIENT_TYPE_BOTS		UMETA(DisplayName = "Bots"),
	CLIENT_TYPE_MINI		UMETA(DisplayName = "Mini"),
};

//加密通讯的类别
UENUM(BlueprintType)
enum class NETWORK_ENCRYPT_TYPE : uint8
{
	ENCRYPT_TYPE_NONE			UMETA(DisplayName = "None"),
	ENCRYPT_TYPE_BLOWFISH		UMETA(DisplayName = "Blowfish"),
};

// 网络消息类别
enum NETWORK_MESSAGE_TYPE
{
	NETWORK_MESSAGE_TYPE_COMPONENT = 0,		// 组件消息
	NETWORK_MESSAGE_TYPE_ENTITY = 1,		// entity消息
};

enum ProtocolType
{
	PROTOCOL_TCP = 0,
	PROTOCOL_UDP = 1,
};

enum EntityDataFlags
{
	ED_FLAG_UNKOWN = 0x00000000,			// 未定义
	ED_FLAG_CELL_PUBLIC = 0x00000001,		// 相关所有cell广播
	ED_FLAG_CELL_PRIVATE = 0x00000002,		// 当前cell
	ED_FLAG_ALL_CLIENTS = 0x00000004,		// cell广播与所有客户端
	ED_FLAG_CELL_PUBLIC_AND_OWN = 0x00000008, // cell广播与自己的客户端
	ED_FLAG_OWN_CLIENT = 0x00000010,		// 当前cell和客户端
	ED_FLAG_BASE_AND_CLIENT = 0x00000020,	// base和客户端
	ED_FLAG_BASE = 0x00000040,				// 当前base
	ED_FLAG_OTHER_CLIENTS = 0x00000080,		// cell广播和其他客户端
};

// 加密额外存储的信息占用字节(长度+填充)
#define ENCRYPTTION_WASTAGE_SIZE			(1 + 7)

#define PACKET_MAX_SIZE						1500
#ifndef PACKET_MAX_SIZE_TCP
#define PACKET_MAX_SIZE_TCP					1460
#endif
#define PACKET_MAX_SIZE_UDP					1472

typedef uint16								PacketLength;				// 最大65535
#define PACKET_LENGTH_SIZE					sizeof(PacketLength)

#define NETWORK_MESSAGE_ID_SIZE				sizeof(MessageID)
#define NETWORK_MESSAGE_LENGTH_SIZE			sizeof(MessageLength)
#define NETWORK_MESSAGE_LENGTH1_SIZE		sizeof(MessageLengthEx)
#define NETWORK_MESSAGE_MAX_SIZE			65535
#define NETWORK_MESSAGE_MAX_SIZE1			4294967295

// 游戏内容可用包大小
#define GAME_PACKET_MAX_SIZE_TCP			PACKET_MAX_SIZE_TCP - NETWORK_MESSAGE_ID_SIZE - \
											NETWORK_MESSAGE_LENGTH_SIZE - ENCRYPTTION_WASTAGE_SIZE

/*
	网络消息类型， 定长或者变长。
	如果需要自定义长度则在NETWORK_INTERFACE_DECLARE_BEGIN中声明时填入长度即可。
*/
#ifndef NETWORK_FIXED_MESSAGE
#define NETWORK_FIXED_MESSAGE 0
#endif

#ifndef NETWORK_VARIABLE_MESSAGE
#define NETWORK_VARIABLE_MESSAGE -1
#endif

/*
	网络MTU最大值
*/

#define TCP_PACKET_MAX 1460

double getTimeSeconds();

inline float int82angle(int8 angle, bool half)
{
	float halfv = 128.f;
	if (half == true)
		halfv = 254.f;

	halfv = ((float)angle) * ((float)PI / halfv);
	return halfv;
}

inline bool almostEqual(float f1, float f2, float epsilon)
{
	return FMath::Abs(f1 - f2) < epsilon;
}

inline bool isNumeric(KBEngine::KBVar& v)
{
	return v.GetType() == EKBVarTypes::Bool || 
		v.GetType() == EKBVarTypes::Double ||
		v.GetType() == EKBVarTypes::Float ||
		v.GetType() == EKBVarTypes::Int8 ||
		v.GetType() == EKBVarTypes::Int16 ||
		v.GetType() == EKBVarTypes::Int32 ||
		v.GetType() == EKBVarTypes::Int64 ||
		v.GetType() == EKBVarTypes::UInt8 ||
		v.GetType() == EKBVarTypes::UInt16 ||
		v.GetType() == EKBVarTypes::UInt32 ||
		v.GetType() == EKBVarTypes::UInt64;
}

// UE4的尺度单位转化为米
#define UE4_SCALE_UNIT_TO_METER 100.f

// 将KBE坐标系的position(Vector3)转换为UE4坐标系的位置
inline void KBPos2UE4Pos(FVector& UE4_POSITION, const FVector& KBE_POSITION)
{	
	// UE4坐标单位为厘米， KBE单位为米， 因此转化需要常量
	UE4_POSITION.Y = KBE_POSITION.X * UE4_SCALE_UNIT_TO_METER;
	UE4_POSITION.Z = KBE_POSITION.Y * UE4_SCALE_UNIT_TO_METER;
	UE4_POSITION.X = KBE_POSITION.Z * UE4_SCALE_UNIT_TO_METER;
}	

// 将UE4坐标系的position(Vector3)转换为KBE坐标系的位置
inline void UE4Pos2KBPos(FVector& KBE_POSITION, const FVector& UE4_POSITION)
{
	// UE4坐标单位为厘米， KBE单位为米， 因此转化需要常量
	KBE_POSITION.X = UE4_POSITION.Y / UE4_SCALE_UNIT_TO_METER;
	KBE_POSITION.Y = UE4_POSITION.Z / UE4_SCALE_UNIT_TO_METER;
	KBE_POSITION.Z = UE4_POSITION.X / UE4_SCALE_UNIT_TO_METER;
}

// 将KBE方向转换为UE4方向
inline void KBDir2UE4Dir(FRotator& UE4_DIRECTION, const FVector& KBE_DIRECTION)
{
	UE4_DIRECTION.Pitch = FMath::RadiansToDegrees<float>(KBE_DIRECTION.Y);
	UE4_DIRECTION.Yaw = FMath::RadiansToDegrees<float>(KBE_DIRECTION.Z);
	UE4_DIRECTION.Roll = FMath::RadiansToDegrees<float>(KBE_DIRECTION.X);
}

// 将UE4方向转换为KBE方向
inline void UE4Dir2KBDir(FVector& KBE_DIRECTION, const FRotator& UE4_DIRECTION)
{
	KBE_DIRECTION.Y = FMath::DegreesToRadians<float>(UE4_DIRECTION.Pitch);
	KBE_DIRECTION.Z = FMath::DegreesToRadians<float>(UE4_DIRECTION.Yaw);
	KBE_DIRECTION.X = FMath::DegreesToRadians<float>(UE4_DIRECTION.Roll);
}

UCLASS()
class KBENGINEPLUGINS_API AKBECommon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKBECommon();
	
};


