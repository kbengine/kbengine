// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "KBECommon.h"
#include "KBEvent.h"
#include "ClientSDKUpdater.h"
#include "ClientSDKUpdateUI.h"
#include "ShowPromptMessageUI.h"
#include "Components/ActorComponent.h"
#include "KBEMain.generated.h"

/*
可以理解为插件的入口模块
在这个入口中安装了需要监听的事件(installEvents)，同时初始化KBEngine(initKBEngine)
*/
class KBEngineApp;


UCLASS(ClassGroup = "KBEngine", blueprintable, editinlinenew, hidecategories = (Object, LOD, Lighting, TextureStreaming), meta = (DisplayName = "KBEngine Main", BlueprintSpawnableComponent))
class KBENGINEPLUGINS_API UKBEMain : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:	
	// Sets default values for this component's properties
	UKBEMain();

	/**
	* Initializes the component.  Occurs at level startup. This is before BeginPlay (Actor or Component).
	* All Components in the level will be Initialized on load before any Actor/Component gets BeginPlay
	* Requires component to be registered, and bWantsInitializeComponent to be true.
	*/
	virtual void InitializeComponent() override;

	// Called when the game starts
	virtual void BeginPlay() override;

	/**
	* Ends gameplay for this component.
	* Called from AActor::EndPlay only if bHasBegunPlay is true
	*/
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	* Handle this component being Uninitialized.
	* Called from AActor::EndPlay only if bHasBeenInitialized is true
	*/
	virtual void UninitializeComponent() override;

	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;


	void installEvents();
	void deregisterEvents();

	void onVersionNotMatch(const UKBEventData* pEventData);
	void onScriptVersionNotMatch(const UKBEventData* pEventData);

	bool isUpdateSDK();
	void downloadSDKFromServer();
	void onDownloadSDK(const UKBEventData* pEventData);
	void onImportClientSDKSuccessfully(const UKBEventData* pEventData);

	void handVersionNotMatch();
	void showPromptMessageOfCompile();

	UFUNCTION(BlueprintCallable, Category = "KBEngine")
	static FString getClientVersion();

	UFUNCTION(BlueprintCallable, Category = "KBEngine")
	static FString getClientScriptVersion();

	UFUNCTION(BlueprintCallable, Category = "KBEngine")
	static FString getServerVersion();

	UFUNCTION(BlueprintCallable, Category = "KBEngine")
	static FString getServerScriptVersion();

	/*
		客户端属于KBE框架中的一个功能组件，这里获取将固定返回client
	*/
	UFUNCTION(BlueprintCallable, Category = "KBEngine")
	static FString getComponentName();

	/**
		在程序关闭时需要主动调用, 彻底销毁KBEngine
	*/
	UFUNCTION(BlueprintCallable, Category = "KBEngine")
	bool destroyKBEngine();

	UFUNCTION(BlueprintCallable, Category = "KBEngine")
	bool login(FString username, FString password, TArray<uint8> datas);

	/*
		创建账号
	*/
	UFUNCTION(BlueprintCallable, Category = "KBEngine")
	bool createAccount(FString username, FString password, const TArray<uint8>& datas);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	FString ip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int port;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	EKCLIENT_TYPE clientType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	NETWORK_ENCRYPT_TYPE networkEncryptType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int syncPlayerMS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool useAliasEntityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool isOnInitCallPropertysSetMethods;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool forceDisableUDP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int serverHeartbeatTick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int TCP_SEND_BUFFER_MAX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int TCP_RECV_BUFFER_MAX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int UDP_SEND_BUFFER_MAX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	int UDP_RECV_BUFFER_MAX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
	bool automaticallyUpdateSDK;

	KBEngine::ClientSDKUpdater* pUpdaterObj;

	static bool hasUpdateSDK;
	
	TSharedPtr<class SClientSDKUpdateUI> ClientSDKUpdateUI;
	TSharedPtr<class SShowPromptMessageUI> ShowPromptMessageUI;

};
