// Fill out your copyright notice in the Description page of Project Settings.

#include "KBEMain.h"
#include "KBEngine.h"
#include "KBEngineArgs.h"
#include "MemoryStream.h"
#include "Bundle.h"
#include "Engine.h"
#include "KBDebug.h"
#include "Entity.h"

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

	KBEngineArgs* pArgs = new KBEngineArgs();
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

	KBEngineApp::destroyKBEngineApp();
	if (!KBEngineApp::getSingleton().initialize(pArgs))
		delete pArgs;

	installEvents();
	
#ifdef KBENGINE_NO_CRYPTO
	if (pArgs->networkEncryptType == NETWORK_ENCRYPT_TYPE::ENCRYPT_TYPE_BLOWFISH)
	{
		pArgs->networkEncryptType = NETWORK_ENCRYPT_TYPE::ENCRYPT_TYPE_NONE;
		ERROR_MSG("No module CryptoPP! Please use unreal engine source code to install");
	}
#endif
}

void UKBEMain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (pUpdaterObj != nullptr)
	{
		delete pUpdaterObj;
		pUpdaterObj = nullptr;
	}

	deregisterEvents();
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UKBEMain::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	
}

void UKBEMain::installEvents()
{
	KBENGINE_REGISTER_EVENT(KBEventTypes::onScriptVersionNotMatch, onScriptVersionNotMatch);
	KBENGINE_REGISTER_EVENT(KBEventTypes::onVersionNotMatch, onVersionNotMatch);
	KBENGINE_REGISTER_EVENT(KBEventTypes::onImportClientSDKSuccessfully, onImportClientSDKSuccessfully);
}

void UKBEMain::deregisterEvents()
{
	KBENGINE_DEREGISTER_EVENT(KBEventTypes::onScriptVersionNotMatch);
	KBENGINE_DEREGISTER_EVENT(KBEventTypes::onVersionNotMatch);
	KBENGINE_DEREGISTER_EVENT(KBEventTypes::onImportClientSDKSuccessfully);
}

void UKBEMain::onVersionNotMatch(const UKBEventData* pEventData)
{
	downloadSDKFromServer();
}

void UKBEMain::onScriptVersionNotMatch(const UKBEventData* pEventData)
{
	downloadSDKFromServer();
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
	if (isUpdateSDK())
	{
		if (pUpdaterObj == nullptr)
		{
			pUpdaterObj = new ClientSDKUpdater();
		}

		pUpdaterObj->downloadSDKFromServer();
	}
	else
	{
		if(pUpdaterObj != nullptr)
		{
			delete pUpdaterObj;
			pUpdaterObj = nullptr;
		}
	}
}

void UKBEMain::onImportClientSDKSuccessfully(const UKBEventData* pEventData)
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}

FString UKBEMain::getClientVersion()
{
	if (!KBEngineApp::getSingleton().isInitialized())
		return TEXT("");

	return KBEngineApp::getSingleton().clientVersion();
}

FString UKBEMain::getClientScriptVersion()
{
	if (!KBEngineApp::getSingleton().isInitialized())
		return TEXT("");

	return KBEngineApp::getSingleton().clientScriptVersion();
}

FString UKBEMain::getServerVersion()
{
	if (!KBEngineApp::getSingleton().isInitialized())
		return TEXT("");

	return KBEngineApp::getSingleton().serverVersion();
}

FString UKBEMain::getServerScriptVersion()
{
	if (!KBEngineApp::getSingleton().isInitialized())
		return TEXT("");

	return KBEngineApp::getSingleton().serverScriptVersion();
}

FString UKBEMain::getComponentName()
{
	if (!KBEngineApp::getSingleton().isInitialized())
		return TEXT("");

	return KBEngineApp::getSingleton().component();
}

bool UKBEMain::destroyKBEngine()
{
	if (!KBEngineApp::getSingleton().isInitialized())
		return false;

	KBEngineApp::getSingleton().destroy();
	KBENGINE_EVENT_CLEAR();
	return true;
}

bool UKBEMain::login(FString username, FString password, TArray<uint8> datas)
{
	if (!KBEngineApp::getSingleton().isInitialized())
	{
		return false;
	}

	KBEngineApp::getSingleton().reset();

	UKBEventData_login* pEventData = NewObject<UKBEventData_login>();
	pEventData->username = username;
	pEventData->password = password;
	pEventData->datas = datas;
	KBENGINE_EVENT_FIRE(KBEventTypes::login, pEventData);
	return true;
}

bool UKBEMain::createAccount(FString username, FString password, const TArray<uint8>& datas)
{
	if (!KBEngineApp::getSingleton().isInitialized())
	{
		return false;
	}

	KBEngineApp::getSingleton().reset();

	UKBEventData_createAccount* pEventData = NewObject<UKBEventData_createAccount>();
	pEventData->username = username;
	pEventData->password = password;
	pEventData->datas = datas;
	KBENGINE_EVENT_FIRE(KBEventTypes::createAccount, pEventData);
	return true;
}