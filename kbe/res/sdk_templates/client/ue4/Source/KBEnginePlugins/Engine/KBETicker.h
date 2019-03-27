// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "KBECommon.h"
#include "KBEvent.h"
#include "Tickable.h"
#include "KBETicker.generated.h"
/**
 * 
 */
UCLASS()
class KBENGINEPLUGINS_API UKBETicker : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
public:
	UKBETicker();
	~UKBETicker();

	UWorld* GetWorld() const override;

	virtual bool IsTickableWhenPaused() const override;	
	virtual bool IsTickableInEditor() const override;	
	virtual UWorld* GetTickableGameObjectWorld() const override;	

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	void OnEndPIE(const bool);
};
