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

	class AEddOfSpadesGameState* GameState; 

	UPROPERTY()
	class UServerTCP* ServerTCP;

	UPROPERTY()
	class UWorldTransferProtocol* WorldTransfer;

	UPROPERTY()
	class UBlockPhysics* BlockPhysics;

public:
	UPROPERTY(BlueprintReadOnly)
	class UWorldLoader* WorldLoader;

public:
	AEddOfSpadesGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void StartPlay() override;

	void SendWorldToAllConnectedClients();

	void RespawnPlayer(AEddOfSpadesPlayerController* Player);

	UFUNCTION()
	void OnBlockChanged(const FIntVector& Block);

	UFUNCTION()
	void OnStartButtonPressed();

	UFUNCTION()
	void OnWorldLoaded();

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

};



