// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chunk.h"
#include "ChunkSpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldMeshBuilt);

UCLASS()
class EDDOFSPADES_API AChunkSpawner : public AActor
{
	GENERATED_BODY()
	
private:
	TArray<AChunk*> SpawnedChunks;

public:
	UPROPERTY()
	FOnWorldMeshBuilt OnWorldMeshBuilt;

public:	
	// Sets default values for this actor's properties
	AChunkSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void RebuildWorldMesh();
	
	void RebuildSingleChunk(int32 ChunkX, int32 ChunkY);

	void BlockUpdatedInChunk(int32 ChunkX, int32 ChunkY);

};
