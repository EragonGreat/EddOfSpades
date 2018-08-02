// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "FallingBlocks.generated.h"

UCLASS()
class EDDOFSPADES_API AFallingBlocks : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleDefaultsOnly)
	class UProceduralMeshComponent* Mesh;

	UPROPERTY(ReplicatedUsing="OnRep_BlocksThatFell")
	TArray<FIntVector> BlocksThatFell;
	
public:	
	// Sets default values for this actor's properties
	AFallingBlocks();

	void SetBlocksThatWillFall(const TArray<FIntVector>& Blocks);

protected:
	UFUNCTION()
	void OnRep_BlocksThatFell();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastUpdateTransform(const FVector& Location, const FRotator Rotation);

	virtual void BeginPlay() override;

private:
	void AddBlockMesh(class AEddOfSpadesGameState* GameState, const FIntVector& Block, TArray<FVector>& Positions, TArray<int32>& Indices, TArray<FLinearColor>& VertexColors);
	
	void BuildMesh();

public:
	virtual void Tick(float DeltaSeconds) override;

};
