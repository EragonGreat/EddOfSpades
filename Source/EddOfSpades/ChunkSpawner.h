// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chunk.h"
#include "ChunkSpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldMeshBuilt);

UCLASS()
class EDDOFSPADES_API AChunkSpawner : public AActor, public FRunnable
{
	GENERATED_BODY()
	
private:
	class AEddOfSpadesHUD* HUD;

	TArray<AChunk*> SpawnedChunks;

	bool bRunThread;
	bool bStartWorldReload;

	FRunnableThread* ChunkThread;
	
	TQueue<AChunk*, EQueueMode::Spsc> ChunksToBuild;

public:
	UPROPERTY()
	FOnWorldMeshBuilt OnWorldMeshBuilt;

public:
	// Sets default values for this actor's properties
	AChunkSpawner();
	~AChunkSpawner();

public:
	void RebuildWorldMesh();

	void RebuildSingleChunk(int32 ChunkX, int32 ChunkY);

	void BlockUpdatedInChunk(int32 ChunkX, int32 ChunkY);

	virtual uint32 Run() override;
	virtual void Stop() override;

};
