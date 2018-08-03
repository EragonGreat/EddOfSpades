// Fill out your copyright notice in the Description page of Project Settings.

#include "BlockPhysics.h"
#include "EddOfSpadesGameState.h"

UBlockPhysics::UBlockPhysics()
{
	SetUpNodeArray();
}

bool UBlockPhysics::CheckIfBlocksWillFall(AEddOfSpadesGameState* GameState, const FIntVector& Block, FBlockPhysicsResult& OutResult)
{
	this->GameState = GameState;

	// Make sure that the out result is clean, this could be performed in a loop
	OutResult.BlocksToFall.Empty();
	OutResult.MaxBlock = Block;
	OutResult.MinBlock = Block;

	// If the block became air, analyze the blocks around it
	if(GameState->IsBlockAirAt(Block))
	{
		AnalyzeChangedBlock({Block.X + 1, Block.Y, Block.Z}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
		AnalyzeChangedBlock({Block.X - 1, Block.Y, Block.Z}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
		AnalyzeChangedBlock({Block.X, Block.Y + 1, Block.Z}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
		AnalyzeChangedBlock({Block.X, Block.Y - 1, Block.Z}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
		AnalyzeChangedBlock({Block.X, Block.Y, Block.Z + 1}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
		AnalyzeChangedBlock({Block.X, Block.Y, Block.Z - 1}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
	}

	// Fin the min and max blocks, this is used for chunk refreshes later on
	bool bBlocksFell = false;
	for(const FFallingBlocksList& List : OutResult.BlocksToFall)
	{
		for(const FIntVector& FallingBlock : List.Blocks)
		{
			if(OutResult.MinBlock.X > FallingBlock.X)
			{
				OutResult.MinBlock.X = FallingBlock.X;
			}
			if(OutResult.MinBlock.Y > FallingBlock.Y)
			{
				OutResult.MinBlock.Y = FallingBlock.Y;
			}
			if(OutResult.MaxBlock.X > FallingBlock.X)
			{
				OutResult.MaxBlock.X = FallingBlock.X;
			}
			if(OutResult.MaxBlock.Y > FallingBlock.Y)
			{
				OutResult.MaxBlock.Y = FallingBlock.Y;
			}

			bBlocksFell = true;
		}
	}

	// No blocks will fall, return false
	return bBlocksFell;
}

void UBlockPhysics::AnalyzeChangedBlock(const FIntVector& Block, FFallingBlocksList& OutFallingBlocks)
{
	// Check bounds
	if(Block.X >= 0 && Block.Y >= 0 && Block.Z >= 0 &&
		Block.X < GameConstants::WorldBlockCount && Block.Y < GameConstants::WorldBlockCount && Block.Z < GameConstants::WorldHeight)
	{
		// Ignore air blocks
		if(GameState->IsBlockAirAt(Block.X, Block.Y, Block.Z))
		{
			return;
		}

		bool bBlocksWillFall = true;

		TArray<FBlockNode*> ClosedSet;
		TArray<FBlockNode*> OpenSet;

		GetNode(Block.X, Block.Y, Block.Z)->StartCost = 0;
		OpenSet.Add(GetNode(Block.X, Block.Y, Block.Z));

		while(OpenSet.Num() > 0)
		{
			// Get the node with the lowest cost in OpenSet
			OpenSet.Sort();
			FBlockNode* CurrentBlock = OpenSet[0];
			OpenSet.RemoveAt(0, 1, false);

			// Add current to closed set, to prevent it from getting checked again
			ClosedSet.Add(CurrentBlock);

			// Check if this is the ground
			if(CurrentBlock->Position.Z == 0)
			{
				bBlocksWillFall = false;
				break;
			}

			// Go through each neighbour and evaluate them
			AddNeigbourIfValid(CurrentBlock->Position.X + 1, CurrentBlock->Position.Y, CurrentBlock->Position.Z, CurrentBlock, ClosedSet, OpenSet);
			AddNeigbourIfValid(CurrentBlock->Position.X - 1, CurrentBlock->Position.Y, CurrentBlock->Position.Z, CurrentBlock, ClosedSet, OpenSet);
			AddNeigbourIfValid(CurrentBlock->Position.X, CurrentBlock->Position.Y + 1, CurrentBlock->Position.Z, CurrentBlock, ClosedSet, OpenSet);
			AddNeigbourIfValid(CurrentBlock->Position.X, CurrentBlock->Position.Y - 1, CurrentBlock->Position.Z, CurrentBlock, ClosedSet, OpenSet);
			AddNeigbourIfValid(CurrentBlock->Position.X, CurrentBlock->Position.Y, CurrentBlock->Position.Z + 1, CurrentBlock, ClosedSet, OpenSet);
			AddNeigbourIfValid(CurrentBlock->Position.X, CurrentBlock->Position.Y, CurrentBlock->Position.Z - 1, CurrentBlock, ClosedSet, OpenSet);

		}

		if(bBlocksWillFall)
		{
			for(const FBlockNode* Node : ClosedSet)
			{
				OutFallingBlocks.Blocks.Add(Node->Position);
			}
		}
	}
}

void UBlockPhysics::AddNeigbourIfValid(int32 x, int32 y, int32 z, FBlockNode* Origin, TArray<FBlockNode*>& Closed, TArray<FBlockNode*>& Open)
{
	// Check bounds first
	if(x >= 0 && y >= 0 && z >= 0 &&
		x < GameConstants::WorldBlockCount && y < GameConstants::WorldBlockCount && z < GameConstants::WorldHeight)
	{
		// Only add solid blocks
		if(!GameState->IsBlockAirAt(x, y, z))
		{
			// Make sure it hasn't already beeen checked
			if(!Closed.Contains(GetNode(x, y, z)) && !Open.Contains(GetNode(x, y, z)))
			{
				FIntVector Offset = FIntVector(x, y, z) - Origin->Position;
				GetNode(x, y, z)->StartCost = abs(Offset.X) + abs(Offset.Y) + abs(Offset.Z);
				Open.Add(GetNode(x, y, z));

			}

		}

	}

}

void UBlockPhysics::SetUpNodeArray()
{
	BlockNodes.Empty();
	BlockNodes.AddDefaulted(GameConstants::TotalBlocks);

	for(int32 x = 0; x < GameConstants::WorldBlockCount; x++)
	{
		for(int32 y = 0; y < GameConstants::WorldBlockCount; y++)
		{
			for(int32 z = 0; z < GameConstants::WorldHeight; z++)
			{
				GetNode(x, y, z)->Position = FIntVector(x, y, z);
				GetNode(x, y, z)->GroundCost = z;
			}
		}
	}
}

FBlockNode* UBlockPhysics::GetNode(int32 X, int32 Y, int32 Z)
{
	return &BlockNodes[X + GameConstants::WorldBlockCount * (Y + Z * GameConstants::WorldBlockCount)];
}

bool operator<(const FBlockNode& A, const FBlockNode& B)
{
	
	return A.StartCost + A.GroundCost < B.StartCost + B.GroundCost;
	
}
