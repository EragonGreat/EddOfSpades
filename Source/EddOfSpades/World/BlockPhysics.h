// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BlockPhysics.generated.h"


USTRUCT()
struct EDDOFSPADES_API FBlockNode
{
	GENERATED_BODY()

public:
	FIntVector Position;

	int32 StartCost;
	int32 GroundCost;

	

};

bool operator<(const FBlockNode& A, const FBlockNode& B);

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
	TArray<FBlockNode> BlockNodes;

	UPROPERTY()
	class AEddOfSpadesGameState* GameState;
	
public:
	UBlockPhysics();

	bool CheckIfBlocksWillFall(AEddOfSpadesGameState* GameState, const FIntVector& BlockPosition, FBlockPhysicsResult& OutResult);

private:
	void AnalyzeChangedBlock(const FIntVector& Block, FFallingBlocksList& OutFallingBlocks);
	void AddNeigbourIfValid(int32 x, int32 y, int32 z, FBlockNode* Origin, TArray<FBlockNode*>& Closed, TArray<FBlockNode*>& Open);

	void SetUpNodeArray();

	FBlockNode* GetNode(int32 X, int32 Y, int32 Z);

};
