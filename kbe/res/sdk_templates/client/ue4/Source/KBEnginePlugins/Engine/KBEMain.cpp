// Fill out your copyright notice in the Description page of Project Settings.

#include "KBEMain.h"
#include "KBEngine.h"
#include "KBEngineArgs.h"
#include "MemoryStream.h"
#include "Bundle.h"
#include "Engine.h"
#include "KBDebug.h"
#include "Entity.h"

bool UKBEMain::hasUpdateSDK = false;

// Sets default values for this component's properties
UKBEMain::UKBEMain(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;

	// ...

	ip = TEXT("127.0.0.1");
	port = @{KBE_LOGIN_PORT};
	syncPlayerMS = 1000 / @{KBE_UPDATEHZ};
	useAliasEntityID = @{KBE_USE_ALIAS_ENTITYID};
	isOnInitCallPropertysSetMethods = true;
	forceDisableUDP = false;
	clientType = EKCLIENT_TYPE::CLIENT_TYPE_WIN;
	networkEncryptType = NETWORK_ENCRYPT_TYPE::ENCRYPT_TYPE_NONE;
	serverHeartbeatTick = @{KBE_SERVER_EXTERNAL_TIMEOUT};
	TCP_SEND_BUFFER_MAX = TCP_PACKET_MAX;
	TCP_RECV_BUFFER_MAX = TCP_PACKET_MAX;
	UDP_SEND_BUFFER_MAX = 128;
	UDP_RECV_BUFFER_MAX = 128;

	pUpdaterObj = NULL;
	automaticallyUpdateSDK = true;
}

void UKBEMain::InitializeComponent()
{
	Super::InitializeComponent();
}

void UKBEMain::UninitializeComponent()
{
	Super::UninitializeComponent();
}

// Called when the game starts
void UKBEMain::BeginPlay()
{
	Super::BeginPlay();
	KBEngine::KBEngineArgs* pArgs = new KBEngine::KBEngineArgs();
	pArgs->ip = ip;
	pArgs->port = port;
	pArgs->syncPlayerMS = syncPlayerMS;
	pArgs->useAliasEntityID = useAliasEntityID;
	pArgs->isOnInitCallPropertysSetMethods = isOnInitCallPropertysSetMethods;
	pArgs->forceDisableUDP = forceDisableUDP;
	pArgs->clientType = clientType;
	pArgs->networkEncryptType = networkEncryptType;
	pArgs->serverHeartbeatTick = serverHeartbeatTick / 2;
	pArgs->TCP_SEND_BUFFER_MAX = TCP_SEND_BUFFER_MAX;
	pArgs->TCP_RECV_BUFFER_MAX = TCP_RECV_BUFFER_MAX;
	pArgs->UDP_SEND_BUFFER_MAX = UDP_SEND_BUFFER_MAX;
	pArgs->UDP_RECV_BUFFER_MAX = UDP_RECV_BUFFER_MAX;

	if(!KBEngine::KBEngineApp::getSingleton().initialize(pArgs))
		delete pArgs;

	installEvents();
}

void UKBEMain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (pUpdaterObj != nullptr)
	{
		delete pUpdaterObj;
		pUpdaterObj = nullptr;
	}

	ClientSDKUpdateUI.Reset();
	deregisterEvents();
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UKBEMain::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	
}

void UKBEMain::installEvents()
{
	KBENGINE_REGISTER_EVENT(KBEngine::KBEventTypes::onScriptVersionNotMatch, onScriptVersionNotMatch);
	KBENGINE_REGISTER_EVENT(KBEngine::KBEventTypes::onVersionNotMatch, onVersionNotMatch);
	KBENGINE_REGISTER_EVENT(KBEngine::KBEventTypes::onImportClientSDKSuccessfully, onImportClientSDKSuccessfully);
	KBENGINE_REGISTER_EVENT(KBEngine::KBEventTypes::onDownloadSDK, onDownloadSDK);
}

void UKBEMain::deregisterEvents()
{
	KBENGINE_DEREGISTER_EVENT(KBEngine::KBEventTypes::onScriptVersionNotMatch);
	KBENGINE_DEREGISTER_EVENT(KBEngine::KBEventTypes::onVersionNotMatch);
	KBENGINE_DEREGISTER_EVENT(KBEngine::KBEventTypes::onImportClientSDKSuccessfully);
	KBENGINE_DEREGISTER_EVENT(KBEngine::KBEventTypes::onDownloadSDK);
}

void UKBEMain::onVersionNotMatch(const UKBEventData* pEventData)
{
	handVersionNotMatch();
}

void UKBEMain::onScriptVersionNotMatch(const UKBEventData* pEventData)
{
	handVersionNotMatch();
}

bool UKBEMain::isUpdateSDK()
{
#if WITH_EDITOR
	return automaticallyUpdateSDK;
#endif

	return false;
}

void UKBEMain::downloadSDKFromServer()
{
	if (GEngine->IsValidLowLevel())
	{
		GEngine->GameViewport->RemoveAllViewportWidgets();
	}

	if (isUpdateSDK())
	{
		ClientSDKUpdateUI = SNew(SClientSDKUpdateUI);

		if (GEngine->IsValidLowLevel())
		{
			GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ClientSDKUpdateUI.ToSharedRef()));
		}

		if (ClientSDKUpdateUI.IsValid())
		{
			ClientSDKUpdateUI->SetVisibility(EVisibility::Visible);
		}

		hasUpdateSDK = true;
	}
}

void UKBEMain::onImportClientSDKSuccessfully(const UKBEventData* pEventData)
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}

void UKBEMain::onDownloadSDK(const UKBEventData* pEventData)
{
	ClientSDKUpdateUI.Reset();
	if (GEngine->IsValidLowLevel())
	{
		GEngine->GameViewport->RemoveAllViewportWidgets();
	}

	const UKBEventData_onDownloadSDK* pData = Cast<UKBEventData_onDownloadSDK>(pEventData);
	if(pData->isDownload)
	{
		if (pUpdaterObj == nullptr)
		{
			pUpdaterObj = new KBEngine::ClientSDKUpdater();
		}

		pUpdaterObj->downloadSDKFromServer();
	}
	else
	{
		if (pUpdaterObj != nullptr)
		{
			delete pUpdaterObj;
			pUpdaterObj = nullptr;
		}
	}
}

FString UKBEMain::getClientVersion()
{
	if (!KBEngine::KBEngineApp::getSingleton().isInitialized())
		return TEXT("");

	return KBEngine::KBEngineApp::getSingleton().clientVersion();
}

FString UKBEMain::getClientScriptVersion()
{
	if (!KBEngine::KBEngineApp::getSingleton().isInitialized())
		return TEXT("");

	return KBEngine::KBEngineApp::getSingleton().clientScriptVersion();
}

FString UKBEMain::getServerVersion()
{
	if (!KBEngine::KBEngineApp::getSingleton().isInitialized())
		return TEXT("");

	return KBEngine::KBEngineApp::getSingleton().serverVersion();
}

FString UKBEMain::getServerScriptVersion()
{
	if (!KBEngine::KBEngineApp::getSingleton().isInitialized())
		return TEXT("");

	return KBEngine::KBEngineApp::getSingleton().serverScriptVersion();
}

FString UKBEMain::getComponentName()
{
	if (!KBEngine::KBEngineApp::getSingleton().isInitialized())
		return TEXT("");

	return KBEngine::KBEngineApp::getSingleton().component();
}

bool UKBEMain::destroyKBEngine()
{
	if (!KBEngine::KBEngineApp::getSingleton().isInitialized())
		return false;

	KBEngine::KBEngineApp::getSingleton().destroy();
	KBENGINE_EVENT_CLEAR();
	return true;
}

bool UKBEMain::login(FString username, FString password, TArray<uint8> datas)
{
	if (!KBEngine::KBEngineApp::getSingleton().isInitialized())
	{
		return false;
	}

	KBEngine::KBEngineApp::getSingleton().reset();

	UKBEventData_login* pEventData = NewObject<UKBEventData_login>();
	pEventData->username = username;
	pEventData->password = password;
	pEventData->datas = datas;
	KBENGINE_EVENT_FIRE(KBEngine::KBEventTypes::login, pEventData);
	return true;
}

bool UKBEMain::createAccount(FString username, FString password, const TArray<uint8>& datas)
{
	if (!KBEngine::KBEngineApp::getSingleton().isInitialized())
	{
		return false;
	}

	KBEngine::KBEngineApp::getSingleton().reset();

	UKBEventData_createAccount* pEventData = NewObject<UKBEventData_createAccount>();
	pEventData->username = username;
	pEventData->password = password;
	pEventData->datas = datas;
	KBENGINE_EVENT_FIRE(KBEngine::KBEventTypes::createAccount, pEventData);
	return true;
}

void UKBEMain::handVersionNotMatch()
{
	if (hasUpdateSDK) 
	{
		showPromptMessageOfCompile();
	}
	else
	{
		downloadSDKFromServer();
	}
}

void UKBEMain::showPromptMessageOfCompile()
{
	if (GEngine->IsValidLowLevel())
	{
		GEngine->GameViewport->RemoveAllViewportWidgets();
	}
	
	if (hasUpdateSDK) 
	{
		ShowPromptMessageUI = SNew(SShowPromptMessageUI);

		if (GEngine->IsValidLowLevel())
		{
			GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ShowPromptMessageUI.ToSharedRef()));
		}

		if (ShowPromptMessageUI.IsValid())
		{
			ShowPromptMessageUI->SetVisibility(EVisibility::Visible);
		}
	}
}