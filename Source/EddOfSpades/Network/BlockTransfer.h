// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EddOfSpadesGameState.h"
#include "GameFramework/Actor.h"
#include "BlockTransfer.generated.h"

UCLASS()
class EDDOFSPADES_API ABlockTransfer : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	class AChunkSpawner* ChunkSpawner;

	UPROPERTY()
	AEddOfSpadesGameState* GameState;

public:	
	// Sets default values for this actor's properties
	ABlockTransfer();

	void UpdateSeveralBlocks(const TMap<FIntVector, FBlockData>& Blocks);
	void UpdateSingleBlock(const FIntVector& Position, const FBlockData& BlockData, bool bRefreshChunk = true);
	void DestroyedSeveralBlocks(const TArray<FIntVector>& DestroyedBlocks);
	
	class AChunkSpawner* GetChunkSpawner() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void RefreshChunksConcerning(const TArray<FIntVector>& Blocks);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastChunkSectionRefresh(int32 MinChunkX, int32 MinChunkY, int32 MaxChunkX, int32 MaxChunkY);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBlockChangeWithRefresh(const FIntVector& Position, const FBlockData& BlockData);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBlockChangeWithoutRefresh(const FIntVector& Position, const FBlockData& BlockData);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBlockDestructionWithoutRefresh(const FIntVector& Position);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBlockDestructionWithRefresh(const FIntVector& Position);

};
