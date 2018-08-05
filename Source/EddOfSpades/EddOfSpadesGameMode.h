// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EddOfSpadesPlayerController.h"
#include "GameFramework/GameMode.h"
#include "EddOfSpadesGameMode.generated.h"

UCLASS()
class AEddOfSpadesGameMode : public AGameMode
{
	GENERATED_BODY()

private:
	TArray<AEddOfSpadesPlayerController*> PlayerControllers;

	UPROPERTY()
	AEddOfSpadesGameState* EddGameState; 

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
	AEddOfSpadesGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void StartPlay() override;

	void SendWorldToAllConnectedClients();

	void RespawnPlayer(AEddOfSpadesPlayerController* Player);

	void UpdateBlock(const FIntVector& Position, const FBlockData& NewBlock);
	
	UFUNCTION()
	void OnStartButtonPressed();

	UFUNCTION()
	void OnWorldLoaded();

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

};



