// Fill out your copyright notice in the Description page of Project Settings.

#include "ClientSDKUpdater.h"
#include "iostream"
#include <stdlib.h>
#include <fstream>
#include "Paths.h"
#include "PlatformFilemanager.h"
#include "FileHelper.h"
//#include "OutPutDeviceDebug.h"

namespace KBEngine
{

ClientSDKUpdater::ClientSDKUpdater()
{
	pSdkFileStream = nullptr;
	sdkPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectPluginsDir());
	sdkPath += "kbengine_ue4_plugins";
	FPaths::NormalizeDirectoryName(sdkPath);
	sdkTempPath = sdkPath + "_temp";
	sdkBakPath = sdkPath + "_bak";

	installEvents();
}

ClientSDKUpdater::~ClientSDKUpdater()
{
	if (pSdkFileStream != nullptr)
	{
		MemoryStream::reclaimObject(pSdkFileStream);
		pSdkFileStream = nullptr;
	}

	deregisterEvents();
}

void ClientSDKUpdater::onImportClientSDK(const UKBEventData* eventData)
{
	if (eventData == nullptr) return;

	const UKBEventData_onImportClientSDK* pEventData = dynamic_cast<const UKBEventData_onImportClientSDK*>(eventData);

	if (pSdkFileStream == nullptr)
		pSdkFileStream = MemoryStream::createObject();

	pSdkFileStream->append(pEventData->fileDatas.GetData(), pSdkFileStream->rpos(), (uint32)pEventData->fileDatas.Num());

	warnUpdateSDK = "Download:" + pEventData->fileName + " -> " + FString::FromInt(pSdkFileStream->length()) + "/" + FString::FromInt(pEventData->fileSize) + "bytes! " + FString::FromInt((int)(((float)downloadFiles / (float)(downloadFiles + pEventData->remainingFiles)) * 100)) + "%";

	if (pSdkFileStream->length() == pEventData->fileSize)
	{
		FString tempPath = sdkTempPath + "/" + pEventData->fileName;

		SCREEN_WARNING_MSG("DownloadSDKFileName ::: %s", *tempPath);

		createDirectory(FPaths::GetPath(tempPath));

		unsigned char* fileData = (unsigned char*)((uint8*)malloc(sizeof(uint8)*(pEventData->fileSize + 1)));
		memcpy(fileData, pSdkFileStream->data(), pEventData->fileSize);
		fileData[pEventData->fileSize] = '\0';
		
		if (!tempPath.EndsWith(".png"))
		{
			FString strData = FString(UTF8_TO_TCHAR((fileData)));
			FString filePath = FString(tempPath);
			FFileHelper::SaveStringToFile(strData, *filePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), 0);
		}
		else
		{
			uint8* pPNGData = (unsigned char*)((uint8*)malloc(sizeof(uint8)*(pEventData->fileSize)));
			memcpy(pPNGData, pSdkFileStream->data(), pEventData->fileSize);

			TArray<uint8> PNGData;
			for(int i = 0 ; i < pEventData->fileSize; i++)
			{
				PNGData.Add(pPNGData[i]);
			}

			FString strData = FString(UTF8_TO_TCHAR((fileData)));
			FString filePath = FString(tempPath);
			FFileHelper::SaveArrayToFile(PNGData, *tempPath);

			free(pPNGData);
			pPNGData = nullptr;
		}

		MemoryStream::reclaimObject(pSdkFileStream);
		pSdkFileStream = nullptr;
		
		free(fileData);
		fileData = nullptr;
		downloadFiles += 1;

		if (pEventData->remainingFiles == 0)
		{
			warnUpdateSDK = "";
			downloadFiles = 0;

			replaceNewSDK();
			UE_LOG(LogTemp, Warning, TEXT("End Update KBEnginePlugin!"));

			UKBEventData_onImportClientSDKSuccessfully* pEventData = NewObject<UKBEventData_onImportClientSDKSuccessfully>();

			KBENGINE_EVENT_FIRE(KBEventTypes::onImportClientSDKSuccessfully, pEventData);
		}
	}
}

void ClientSDKUpdater::downloadSDKFromServer()
{
	SCREEN_WARNING_MSG("%s", *(getWarnMsgOfUpdateSDK()));

	downloadFiles = 0;
	if (deleteDirectory(sdkTempPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("DownloadSDKFromServer Begin!"));
		
		createDirectory(sdkTempPath);

		if (pSdkFileStream != nullptr)
		{
			MemoryStream::reclaimObject(pSdkFileStream);
			pSdkFileStream = nullptr;
		}

		// kbcmd options
		FString tool_options = "ue4";
		FString callbackIP = "";
		uint16 callbackPort = 0;
		int clientWindowSize = (int)KBEngineApp::getSingleton().getInitArgs()->TCP_RECV_BUFFER_MAX;

		Bundle* bundle = Bundle::createObject();
		bundle->newMessage(Messages::messages["Loginapp_importClientSDK"]);
		bundle->writeString(tool_options);
		bundle->writeInt32(clientWindowSize);
		bundle->writeString(callbackIP);
		bundle->writeUint16(callbackPort);
		bundle->send(KBEngineApp::getSingleton().pNetworkInterface());

		UE_LOG(LogTemp, Warning, TEXT("DownloadSDKFromServer end!"));
	}
}

FString ClientSDKUpdater::getWarnMsgOfUpdateSDK()
{
	warnUpdateSDK = "Version does not match the server.\nClick to update KBEnginePlugin!\nPull from: " + KBEngineApp::getSingleton().getInitArgs()->ip + ":" + FString::FromInt(KBEngineApp::getSingleton().getInitArgs()->port);
	return warnUpdateSDK;
}

void ClientSDKUpdater::onDownloadSDKSuccessfully()
{
	SCREEN_WARNING_MSG("Update SDK Successfully !!");
}

void ClientSDKUpdater::installEvents()
{
	KBENGINE_REGISTER_EVENT("onImportClientSDK", onImportClientSDK);
}

void ClientSDKUpdater::deregisterEvents()
{
	KBENGINE_DEREGISTER_ALL_EVENT();
}

bool ClientSDKUpdater::deleteDirectory(FString fileName)
{
	FString filePath = FString(fileName);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (PlatformFile.DirectoryExists(*filePath))
	{
		if (PlatformFile.DeleteDirectoryRecursively(*filePath))
		{
			UE_LOG(LogTemp, Warning, TEXT("deleteDic: DeleteDic  successfully!"));
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("deleteDic: Not DeleteDic! %s "), *filePath);
			return false;
		}
	}

	return true;
}

bool ClientSDKUpdater::createDirectory(FString fileName)
{
	FString filePath = FString(fileName);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.CreateDirectory(*filePath);
}

bool ClientSDKUpdater::directoryExists(FString fileName)
{
	FString filePath = FString(fileName);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.DirectoryExists(*filePath);
}

void ClientSDKUpdater::replaceNewSDK()
{
	UE_LOG(LogTemp, Warning, TEXT("ReplaceNewSDK Begin!"));
	copyDirectory(sdkPath, sdkBakPath);

	FString tempScriptPath = "/Source/KBEnginePlugins/Scripts";
	copyDirectory((sdkBakPath + tempScriptPath), (sdkTempPath + tempScriptPath));
	copyDirectory(sdkTempPath, sdkPath);

	deleteDirectory(sdkBakPath);
	deleteDirectory(sdkTempPath);
	
	UE_LOG(LogTemp, Warning, TEXT("ReplaceNewSDK End!"));

	onDownloadSDKSuccessfully();
}

void ClientSDKUpdater::moveDirectory(FString fromDicPath, FString toDicPath)
{
	deleteDirectory(toDicPath);
	TArray<FString> fileNames = findFiles(fromDicPath);

	FString tempFileName;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	for (int i = 0; i < fileNames.Num(); i++)
	{
		tempFileName = fileNames[i];
		tempFileName.Replace(*fromDicPath, *toDicPath);
		PlatformFile.MoveFile(*tempFileName, *fileNames[i]);
	}
}

TArray<FString> ClientSDKUpdater::findFiles(FString directory)
{
	TArray<FString> FileNames;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.FindFilesRecursively(FileNames, *directory, nullptr);
	return FileNames;
}

void ClientSDKUpdater::moveToFile(FString fromFileName, FString toFileName)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.MoveFile(*toFileName, *fromFileName);
}

void ClientSDKUpdater::copyDirectory(FString fromDicPath, FString toDicPath)
{
	FPaths::NormalizeDirectoryName(fromDicPath);
	FPaths::NormalizeDirectoryName(toDicPath);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CopyDirectoryTree(*toDicPath, *fromDicPath, true);
}

}
