// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EddOfSpadesGameState.h"
#include "GameFramework/PlayerController.h"
#include "EddOfSpadesPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartButtonPressed);

/**
 * 
 */
UCLASS()
class EDDOFSPADES_API AEddOfSpadesPlayerController : public APlayerController
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	class AChunkSpawner* ChunkSpawner;

	UPROPERTY()
	class AEddOfSpadesGameMode* GameMode;

	UPROPERTY()
	AEddOfSpadesGameState* GameState;

	UPROPERTY()
	class UClientTCP* ClientTCP;

	UPROPERTY()
	class UWorldTransferProtocol* WorldTransfer;

public:
	UPROPERTY(BlueprintAssignable, Category = "Input")
	FOnStartButtonPressed OnStartButtonPressed;

	UPROPERTY(BlueprintReadWrite, Category = "Input")
	FBlockColor SelectedColor;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void OnWorldReceived();

	UFUNCTION()
	void OnWorldMeshBuilt();

	UFUNCTION()
	void RebuildWorldMesh();

	UFUNCTION()
	void ClientReceiveNewChunk(const FChunkData& Chunk, int32 ChunkX, int32 ChunkY);
	
	UFUNCTION(Client, Reliable)
	void ClientForceChunkRefresh(const int32& ChunkX, const int32& ChunkY);

	UFUNCTION(Client, Reliable)
	void ClientForceChunkSectionRefresh(const int32& ChunkXMin, const int32& ChunkYMin, const int32& ChunkXMax, const int32& ChunkYMax);

	UFUNCTION(Client, Reliable)
	void ClientBlockChanged(const FIntVector& BlockPos, const FBlockData& NewBlockData, bool bShouldRefreshChunk);

	UFUNCTION(Client, Unreliable)
	void ClientBlocksFellDown(const TArray<FIntVector>& FallingBlocks);

	UFUNCTION(BlueprintCallable, Category = "Input", Server, Reliable, WithValidation)
	void ServerDamageBlock(const FIntVector& BlockPosition);

	UFUNCTION(BlueprintCallable, Category = "Input", Server, Reliable, WithValidation)
	void ServerPlaceBlock(const FIntVector& BlockPosition, const FBlockColor BlockColor);

	UFUNCTION(BlueprintCallable, Category="Input", Server, Reliable, WithValidation)
	void ServerStartButonPressed();
	
	UFUNCTION(BlueprintCallable, Category = "Input", Server, Reliable, WithValidation)
	void ServerRequestSpawn();

};
