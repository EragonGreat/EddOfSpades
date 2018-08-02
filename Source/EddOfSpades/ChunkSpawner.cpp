// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkSpawner.h"
#include "EngineUtils.h"
#include "GameConstants.h"

// Sets default values
AChunkSpawner::AChunkSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AChunkSpawner::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AChunkSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChunkSpawner::RebuildWorldMesh()
{
	// Delete all the old chunks, if any
	for (AChunk* Chunk : SpawnedChunks) 
	{
		Chunk->Destroy();
	}
	SpawnedChunks.Empty();

	// Spawn all chunks
	for(int y = 0; y < GameConstants::WorldChunkCount; y++)
	{
		for(int x = 0; x < GameConstants::WorldChunkCount; x++)
		{
			FVector Location = FVector(x * GameConstants::ChunkSize * GameConstants::BlockSize, y * GameConstants::ChunkSize * GameConstants::BlockSize, 0.f);

			AChunk* Chunk = GetWorld()->SpawnActor<AChunk>(Location, FRotator(0.f, 0.f, 0.f));

			Chunk->SetChunkPosition(x, y);
			Chunk->GenerateMeshFromBlockData();

			SpawnedChunks.Push(Chunk);
		}
	}

	OnWorldMeshBuilt.Broadcast();
}

void AChunkSpawner::RebuildSingleChunk(int32 ChunkX, int32 ChunkY)
{
	if(ChunkX < 0 || ChunkY < 0 || ChunkX >= GameConstants::WorldChunkCount || ChunkY >= GameConstants::WorldChunkCount)
	{
		return;
	}

	SpawnedChunks[ChunkX + ChunkY * GameConstants::WorldChunkCount]->GenerateMeshFromBlockData();
}

void AChunkSpawner::BlockUpdatedInChunk(int32 ChunkX, int32 ChunkY)
{
	RebuildSingleChunk(ChunkX, ChunkY);
	RebuildSingleChunk(ChunkX + 1, ChunkY);
	RebuildSingleChunk(ChunkX, ChunkY + 1);
	RebuildSingleChunk(ChunkX - 1, ChunkY);
	RebuildSingleChunk(ChunkX, ChunkY - 1);
}

