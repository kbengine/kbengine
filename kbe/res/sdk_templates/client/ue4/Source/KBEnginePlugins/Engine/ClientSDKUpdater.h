// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KBECommon.h"
#include "KBEvent.h"
#include "KBEngine.h"
#include "KBEngineArgs.h"
#include "MemoryStream.h"
#include "Messages.h"
#include "Bundle.h"
#include "Engine.h"
#include "KBDebug.h"
#include "Entity.h"
#include "Components/ActorComponent.h"

/**
 * 
 */
namespace KBEngine
{

class KBENGINEPLUGINS_API ClientSDKUpdater
{
public:
	ClientSDKUpdater();
	~ClientSDKUpdater();

	FString getWarnMsgOfUpdateSDK();

	void onImportClientSDK(const UKBEventData* pEventData);

	void replaceNewSDK();
	void downloadSDKFromServer();

	void onDownloadSDKSuccessfully();

	void installEvents();
	void deregisterEvents();

	bool deleteDirectory(FString fileName);
	bool createDirectory(FString fileName);
	bool directoryExists(FString fileName);
	void moveDirectory(FString fromDicPath, FString toDicPath);
	void moveToFile(FString fromFileName, FString toFileName);
	void copyDirectory(FString fromDicPath, FString toDicPath);
	TArray<FString> findFiles(FString directory);

	FString sdkPath;
	FString sdkTempPath;
	FString sdkBakPath;
	FString warnUpdateSDK;

	int downloadFiles = 0;

	MemoryStream* pSdkFileStream = nullptr;
};

}