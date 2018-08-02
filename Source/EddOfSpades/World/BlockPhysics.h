// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BlockPhysics.generated.h"


USTRUCT()
struct EDDOFSPADES_API FTensionNode
{
	GENERATED_BODY()

public:
	FIntVector Position;
	int8 DistanceFromOrigin;

};

bool operator==(const FTensionNode& A, const FTensionNode& B);

USTRUCT()
struct EDDOFSPADES_API FFallingBlocksList
{
	GENERATED_BODY()

public:
	TArray<FIntVector> Blocks;
};

USTRUCT()
struct EDDOFSPADES_API FBlockPhysicsResult
{
	GENERATED_BODY()

public:
	FIntVector MinBlock;
	FIntVector MaxBlock;
	TArray<FFallingBlocksList> BlocksToFall;

};

/**
 * 
 */
UCLASS()
class EDDOFSPADES_API UBlockPhysics : public UObject
{
	GENERATED_BODY()
	
private:
	const int8 MaxTensionPerBlock = 5;
	const int8 BreakOffTension = 3;
	const int32 MaxFallingBlocksAtATime = 16;

	UPROPERTY()
	class AEddOfSpadesGameState* GameState;
	
public:
	bool CheckIfBlocksWillFall(AEddOfSpadesGameState* GameState, const FIntVector& BlockPosition, FBlockPhysicsResult& OutResult);

private:
	bool AnalyzeChangedBlock(const FIntVector& Block, FFallingBlocksList& OutFallingBlocks);
	bool BlockIsSupport(const FIntVector& Block);
	void AddNeigbourIfValid(FTensionNode& Current, const FIntVector& Offset, TArray<FTensionNode>& List);
	void BreakOfBlocksUnderTension(const FIntVector& Block, int32 DistanceFromOrigin, int32 MaxTension, FFallingBlocksList& BlocksToFall);

};
