// Fill out your copyright notice in the Description page of Project Settings.

#include "KBETicker.h"
#include "Engine.h"
#include "KBDebug.h"
#include "Entity.h"
#include "KBEngine.h"


UKBETicker::UKBETicker()
{
}

UKBETicker::~UKBETicker()
{
}

void UKBETicker::Tick(float DeltaTime)
{
	KBEvent::processOutEvents();

	APawn* ue4_player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	Entity* kbe_player = KBEngineApp::getSingleton().player();

	// 每个tick将UE4的玩家坐标写入到KBE插件中的玩家实体坐标，插件会定期同步给服务器
	if (kbe_player && ue4_player)
	{
		UE4Pos2KBPos(kbe_player->position, ue4_player->GetActorLocation());
		UE4Dir2KBDir(kbe_player->direction, ue4_player->GetActorRotation());

		kbe_player->isOnGround(ue4_player->GetMovementComponent() && ue4_player->GetMovementComponent()->IsMovingOnGround());
	}

	KBEngineApp::getSingleton().process();
}

bool UKBETicker::IsTickable() const
{
	return true;
}

TStatId UKBETicker::GetStatId() const
{
	return TStatId();
}

UWorld* UKBETicker::GetWorld() const
{ 
	UWorld* World = (GetOuter() != nullptr) ? GetOuter()->GetWorld() : GWorld;	
	if (World == nullptr) 
	{
		World = GWorld; 
	}

	return World; 
}

bool UKBETicker::IsTickableWhenPaused() const
{
	return false;
}

bool UKBETicker::IsTickableInEditor() const
{
	return false;
}

UWorld* UKBETicker::GetTickableGameObjectWorld() const
{
	return GetWorld();
}