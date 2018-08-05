// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Chunk.generated.h"

UCLASS()
class EDDOFSPADES_API AChunk : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* ProceduralMesh;

	FCriticalSection MeshMutex;

	int32 ChunkX;
	int32 ChunkY;

	UPROPERTY()
	class AEddOfSpadesGameState* GameState;

	//Arrays to describe mesh
	TArray<FVector> Positions;
	TArray<int32> Indices;
	TArray<FVector2D> UVs;
	TArray<FVector> Normals;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
	
public:	
	// Sets default values for this actor's properties
	AChunk();

	void GenerateMeshFromBlockData();

	void SetChunkPosition(int32 NewChunkX, int32 NewChunkY);
	void GetChunkPosition(int32& OutChunkX, int32& OutChunkY);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void AddBlockMeshVertices(int32 BlockX, int32 BlockY, int32 BlockZ, 
		TArray<FVector>& Positions,
		TArray<int32>& Indices,
		TArray<FVector2D>& UVs,
		TArray<FVector>& Normals,
		TArray<FLinearColor>& VertexColors,
		TArray<FProcMeshTangent>& Tangents);
};
