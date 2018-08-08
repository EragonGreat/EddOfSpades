// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IGPlayerController.h"
#include "GameFramework/GameMode.h"
#include "IGGameMode.generated.h"

UCLASS()
class AIGGameMode : public AGameMode
{
	GENERATED_BODY()

private:
	TArray<AIGPlayerController*> PlayerControllers;

	UPROPERTY()
	AIGGameState* EddGameState; 

	UPROPERTY()
	class UServerTCP* ServerTCP;

	UPROPERTY()
	class UWorldTransferProtocol* WorldTransfer;

	UPROPERTY()
	class UBlockPhysics* BlockPhysics;

	UPROPERTY()
	class UWorldLoader* WorldLoader;

	UPROPERTY()
	class ABlockTransfer* BlockTransfer;

public:
	AIGGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void StartPlay() override;

	void SendWorldToAllConnectedClients();

	void RespawnPlayer(AIGPlayerController* Player);

	void UpdateBlock(const FIntVector& Position, const FBlockData& NewBlock);
	
	UFUNCTION()
	void OnStartButtonPressed();

	UFUNCTION()
	void OnWorldLoaded();

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

};



