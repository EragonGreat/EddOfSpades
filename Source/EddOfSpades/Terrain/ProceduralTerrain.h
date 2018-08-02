// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralTerrain.generated.h"

UCLASS()
class EDDOFSPADES_API AProceduralTerrain : public AActor
{
	GENERATED_BODY()
	
private:
	class UProceduralMeshComponent* TerrainMesh;

	int32 Quads, VertexCount;
	float QuadSize;

public:	
	// Sets default values for this actor's properties
	AProceduralTerrain();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void GenerateTerrainMesh(int32 Quads, float QuadSize);
	
private:
	int32 ToIndex(int32 x, int32 y) const;

};
