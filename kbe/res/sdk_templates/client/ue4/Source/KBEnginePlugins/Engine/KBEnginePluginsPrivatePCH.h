// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "Engine.h"
#include "KBEnginePlugins.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.


#define INFO_MSG(Format, ...) \
{ \
	SET_WARN_COLOR(COLOR_CYAN); \
	const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
	UE_LOG(LogKBEngine, Log, TEXT("%s"), *Msg); \
	CLEAR_WARN_COLOR(); \
}

#define DEBUG_MSG(Format, ...) \
{ \
	SET_WARN_COLOR(COLOR_CYAN); \
	const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
	UE_LOG(LogKBEngine, Log, TEXT("**DEBUG** %s"), *Msg); \
	CLEAR_WARN_COLOR(); \
}

#define WARNING_MSG(Format, ...) \
{ \
	SET_WARN_COLOR(COLOR_YELLOW); \
	const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
	UE_LOG(LogKBEngine, Log, TEXT("**WARNING** %s"), *Msg); \
	CLEAR_WARN_COLOR(); \
}

#define SCREEN_WARNING_MSG(Format, ...) \
{ \
	SET_WARN_COLOR(COLOR_YELLOW); \
	const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
	const FString NewMsg = FString::Printf(TEXT("**WARNING** %s"), *Msg); \
	UE_LOG(LogKBEngine, Log, TEXT("%s"), *NewMsg); \
	CLEAR_WARN_COLOR(); \
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, NewMsg); \
}

#define ERROR_MSG(Format, ...) \
{ \
	SET_WARN_COLOR(COLOR_RED); \
	const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
	UE_LOG(LogKBEngine, Log, TEXT("**ERROR** %s"), *Msg); \
	CLEAR_WARN_COLOR(); \
}

#define SCREEN_ERROR_MSG(Format, ...) \
{ \
	SET_WARN_COLOR(COLOR_RED); \
	const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
	const FString NewMsg = FString::Printf(TEXT("**ERROR** %s"), *Msg); \
	UE_LOG(LogKBEngine, Log, TEXT("%s"), *NewMsg); \
	CLEAR_WARN_COLOR(); \
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, NewMsg); \
}

#define SCREENDEBUG(Format, ...) \
{ \
	const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
	GEngine->AddOnScreenDebugMessage(-1, 10000.f, FColor::White, Msg); \
}