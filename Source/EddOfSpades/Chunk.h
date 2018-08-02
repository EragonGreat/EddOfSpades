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

	int32 ChunkX, ChunkY;

	class AEddOfSpadesGameState* GameState;

public:	
	// Sets default values for this actor's properties
	AChunk();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GenerateMeshFromBlockData();

	void SetChunkPosition(int32 NewChunkX, int32 NewChunkY);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	void AddBlockMeshVertices(int32 BlockX, int32 BlockY, int32 BlockZ, TArray<FVector>& Positions, TArray<int32>& Indices, TArray<FVector2D>& UVs, TArray<FVector>& Normals, TArray<FLinearColor>& VertexColors, TArray<FProcMeshTangent>& Tangents);

};
