// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "GameConstants.h"
#include "GameFramework/GameState.h"
#include "IGGameState.generated.h"

USTRUCT(BlueprintType)
struct FBlockColor {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	uint8 r;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	uint8 g;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	uint8 b;
};

USTRUCT(BlueprintType)
struct FBlockData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsAir;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FBlockColor Color;

};

USTRUCT(BlueprintType)
struct FChunkData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FBlockData> BlockData;

};

/**
 * 
 */
UCLASS()
class EDDOFSPADES_API AIGGameState : public AGameState
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FChunkData> Chunks;


	virtual void BeginPlay() override;

public:
	const FBlockColor& GetBlockColorAt(const FIntVector& Pos) const;
	const FBlockColor& GetBlockColorAt(int32 x, int32 y, int32 z) const;
	void SetBlockColorAt(const FIntVector& Pos, const FBlockColor& NewColor);
	void SetBlockColorAt(int32 x, int32 y, int32 z, const FBlockColor& NewColor);

	bool IsBlockAirAt(const FIntVector& Pos) const;
	bool IsBlockAirAt(int32 x, int32 y, int32 z) const;
	void SetBlockIsAirAt(const FIntVector& Pos, bool bIsAir);
	void SetBlockIsAirAt(int32 x, int32 y, int32 z, bool bIsAir);

	const FBlockData& GetBlockAt(const FIntVector& Pos) const;
	const FBlockData& GetBlockAt(int32 x, int32 y, int32 z) const;
	void SetBlockAt(const FIntVector& Pos, const FBlockData& NewBlock);
	void SetBlockAt(int32 x, int32 y, int32 z, const FBlockData& NewBlock);

	void SetChunk(const FChunkData& NewChunk, int32 ChunkX, int32 ChunkY);
	const FChunkData& GetChunk(int32 ChunkX, int32 ChunkY) const;

	const TArray<FChunkData>& GetAllChunks() const;

private:
	FChunkData& GetEditableChunk(int32 ChunkX, int32 ChunkY);

};
