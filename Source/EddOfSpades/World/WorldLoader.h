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
	class AIGGameState* GameState;
	class AIGHUD* HUD;

	FRunnableThread* LoadThread;

	bool bRunThread;
	bool bProcedural;

public:
	UPROPERTY(BlueprintAssignable, Category = "World")
	FOnWorldLoaded OnWorldLoaded;


	virtual void Exit() override;


	virtual bool Init() override;

public:
	~UWorldLoader();

	void LoadNewWorld(class AIGGameState* GameState, bool bProcedural);

	virtual uint32 Run() override;
	virtual void Stop() override;

};
