// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WorldLoader.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldLoaded);

/**
 * 
 */
UCLASS()
class EDDOFSPADES_API UWorldLoader : public UObject, public FRunnable
{
	GENERATED_BODY()

private:
	class AEddOfSpadesGameState* GameState;
	class AEddOfSpadesHUD* HUD;

	FRunnableThread* LoadThread;

	bool bProcedural;

public:
	UPROPERTY(BlueprintAssignable, Category = "World")
	FOnWorldLoaded OnWorldLoaded;


	virtual void Exit() override;

public:
	void LoadNewWorld(class AEddOfSpadesGameState* GameState, bool bProcedural);

	virtual uint32 Run() override;
	virtual void Stop() override;

};
