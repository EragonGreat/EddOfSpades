// Fill out your copyright notice in the Description page of Project Settings.

#include "BlockTransfer.h"
#include "ChunkSpawner.h"

// Sets default values
ABlockTransfer::ABlockTransfer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bAlwaysRelevant = true;

	SetReplicates(true);

}

void ABlockTransfer::UpdateSeveralBlocks(const TMap<FIntVector, FBlockData>& Blocks)
{
	for(const auto& Block : Blocks)
	{
		MulticastBlockChangeWithoutRefresh(Block.Key, Block.Value);
	}
}

void ABlockTransfer::UpdateSingleBlock(const FIntVector& Position, const FBlockData& BlockData, bool bRefreshChunk /*= true*/)
{
	if(bRefreshChunk)
	{
		MulticastBlockChangeWithRefresh(Position, BlockData);
	}
	else
	{
		MulticastBlockChangeWithoutRefresh(Position, BlockData);
	}
}

void ABlockTransfer::DestroyedSeveralBlocks(const TArray<FIntVector>& DestroyedBlocks)
{
	for(const FIntVector& Block : DestroyedBlocks)
	{
		MulticastBlockDestructionWithoutRefresh(Block);
	}

	RefreshChunksConcerning(DestroyedBlocks);
}

void ABlockTransfer::RefreshChunksConcerning(const TArray<FIntVector>& Blocks)
{
	int32 MinChunkX = Blocks[0].X;
	int32 MinChunkY = Blocks[0].Y;
	int32 MaxChunkX = Blocks[0].X;
	int32 MaxChunkY = Blocks[0].Y;

	for(const FIntVector& Block : Blocks)
	{
		if(MinChunkX > Block.X)
		{
			MinChunkX = Block.X;
		}
		if(MinChunkY > Block.Y)
		{
			MinChunkY = Block.Y;
		}
		if(MaxChunkX < Block.X)
		{
			MaxChunkX = Block.X;
		}
		if(MaxChunkY < Block.Y)
		{
			MaxChunkY = Block.Y;
		}
	}

	MinChunkX /= GameConstants::ChunkSize;
	MinChunkY /= GameConstants::ChunkSize;
	MaxChunkX /= GameConstants::ChunkSize;
	MaxChunkY /= GameConstants::ChunkSize;

	MinChunkX--;
	MinChunkY--;
	MaxChunkX++;
	MaxChunkY++;

	MulticastChunkSectionRefresh(MinChunkX, MinChunkY, MaxChunkX, MaxChunkY);
}

void ABlockTransfer::MulticastBlockChangeWithRefresh_Implementation(const FIntVector& Position, const FBlockData& BlockData)
{

	// First update the game state
	GameState->SetBlockAt(Position, BlockData);

	// Then update the world mesh
	ChunkSpawner->BlockUpdatedInChunk(Position.X / GameConstants::ChunkSize, Position.Y / GameConstants::ChunkSize);

}

void ABlockTransfer::MulticastBlockChangeWithoutRefresh_Implementation(const FIntVector& Position, const FBlockData& BlockData)
{

	// Only update the game state
	GameState->SetBlockAt(Position, BlockData);

}

void ABlockTransfer::MulticastBlockDestructionWithRefresh_Implementation(const FIntVector& Position)
{

	GameState->SetBlockIsAirAt(Position, true);

	// Then update the world mesh
	ChunkSpawner->BlockUpdatedInChunk(Position.X / GameConstants::ChunkSize, Position.Y / GameConstants::ChunkSize);

}

void ABlockTransfer::MulticastBlockDestructionWithoutRefresh_Implementation(const FIntVector& Position)
{

	GameState->SetBlockIsAirAt(Position, true);

}


void ABlockTransfer::MulticastChunkSectionRefresh_Implementation(int32 MinChunkX, int32 MinChunkY, int32 MaxChunkX, int32 MaxChunkY)
{
	
	for(int32 X = MinChunkX; X <= MaxChunkX; X++)
	{
		for(int32 Y = MinChunkY; Y <= MaxChunkY; Y++)
		{

			ChunkSpawner->RebuildSingleChunk(X, Y);

		}
	}

}

class AChunkSpawner* ABlockTransfer::GetChunkSpawner() const
{
	return ChunkSpawner;
}


// Called when the game starts or when spawned
void ABlockTransfer::BeginPlay()
{
	Super::BeginPlay();

	GameState = GetWorld()->GetGameState<AIGGameState>();

	// Spawn the chunk spawner
	ChunkSpawner = GetWorld()->SpawnActor<AChunkSpawner>();

}