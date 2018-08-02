// Fill out your copyright notice in the Description page of Project Settings.

#include "BlockPhysics.h"
#include "EddOfSpadesGameState.h"

bool UBlockPhysics::CheckIfBlocksWillFall(AEddOfSpadesGameState* GameState, const FIntVector& Block, FBlockPhysicsResult& OutResult)
{
	this->GameState = GameState;

	// Make sure that the out result is clean, this could be performed in a loop
	OutResult.BlocksToFall.Empty();
	OutResult.MaxBlock = Block;
	OutResult.MinBlock = Block;

	if(GameState->IsBlockAirAt(Block))
	{
		// If the block became air, analyze the blocks around it
		AnalyzeChangedBlock({Block.X + 1, Block.Y, Block.Z}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
		AnalyzeChangedBlock({Block.X - 1, Block.Y, Block.Z}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
		AnalyzeChangedBlock({Block.X, Block.Y + 1, Block.Z}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
		AnalyzeChangedBlock({Block.X, Block.Y - 1, Block.Z}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
		AnalyzeChangedBlock({Block.X, Block.Y, Block.Z + 1}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
		AnalyzeChangedBlock({Block.X, Block.Y, Block.Z - 1}, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
	}
	else
	{
		// If a block was placed, analyze that block
		AnalyzeChangedBlock(Block, OutResult.BlocksToFall.Add_GetRef(FFallingBlocksList()));
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

bool UBlockPhysics::AnalyzeChangedBlock(const FIntVector& Block, FFallingBlocksList& OutFallingBlocks)
{
	// Check bounds
	if(Block.X >= 0 && Block.Y >= 0 && Block.Z >= 0 &&
		Block.X < GameConstants::WorldBlockCount && Block.Y < GameConstants::WorldBlockCount && Block.Z < GameConstants::WorldHeight)
	{
		TArray<FTensionNode> BlocksToCheck;

		// Create the origin
		FTensionNode Origin;
		Origin.DistanceFromOrigin = 0;
		Origin.Position = Block;

		BlocksToCheck.Add(Origin);

		bool bBlocksWillFall = true;
		bool bNoSupports = true;
		// Loop through all blocks to check, ordered after distance from origin
		for(int i = 0; i < BlocksToCheck.Num(); i++)
		{
			FTensionNode& Current = BlocksToCheck[i];

			// Check if this is a stable block
			if(BlockIsSupport(Current.Position))
			{
				// Check if tension is within bounds
				if(Current.DistanceFromOrigin < MaxTensionPerBlock)
				{
					bBlocksWillFall = false;
				}
				bNoSupports = false;
			}
			else
			{
				// Get neighbours
				AddNeigbourIfValid(Current, {-1, 0, 0}, BlocksToCheck);
				AddNeigbourIfValid(Current, {0, -1, 0}, BlocksToCheck);
				AddNeigbourIfValid(Current, {0, 0, -1}, BlocksToCheck);
				AddNeigbourIfValid(Current, {1, 0, 0}, BlocksToCheck);
				AddNeigbourIfValid(Current, {0, 1, 0}, BlocksToCheck);
				AddNeigbourIfValid(Current, {0, 0, 1}, BlocksToCheck);
			}
		}

		if(bNoSupports)
		{

			BreakOfBlocksUnderTension(Origin.Position, 0, MaxFallingBlocksAtATime, OutFallingBlocks);

		}
		else if(bBlocksWillFall)
		{

			// Tension is too big! Break of blocks from the origin
			BreakOfBlocksUnderTension(Origin.Position, 0, BreakOffTension, OutFallingBlocks);

		}
	}



	return false;
}

bool UBlockPhysics::BlockIsSupport(const FIntVector& Block)
{

	for(int32 Z = Block.Z - 1; Z >= 0; Z--)
	{
		if(GameState->IsBlockAirAt({Block.X, Block.Y, Z}))
		{
			// Block is not a support, it has air below it
			return false;
		}
	}

	// Block is support
	return true;
}

void UBlockPhysics::AddNeigbourIfValid(FTensionNode& Current, const FIntVector& Offset, TArray<FTensionNode>& List)
{
	FIntVector Block = Current.Position + Offset;

	// Check bounds
	if(Block.X >= 0 && Block.Y >= 0 && Block.Z >= 0 &&
		Block.X < GameConstants::WorldBlockCount && Block.Y < GameConstants::WorldBlockCount && Block.Z < GameConstants::WorldHeight)
	{
		// Don't consider air blocks
		if(!GameState->IsBlockAirAt(Block))
		{
			FTensionNode Neighbour;
			Neighbour.DistanceFromOrigin = Current.DistanceFromOrigin + 1;
			Neighbour.Position = Block;

			// Prevent looping
			int32 Old = 0;
			if(List.Find(Neighbour, Old))
			{
				if(List[Old].DistanceFromOrigin > Neighbour.DistanceFromOrigin)
				{
					List[Old].DistanceFromOrigin = Neighbour.DistanceFromOrigin;
				}
			}
			else
			{
				List.Add(Neighbour);
			}
		}
	}

}

void UBlockPhysics::BreakOfBlocksUnderTension(const FIntVector& Block, int32 DistanceFromOrigin, int32 MaxTension, FFallingBlocksList& BlocksToFall)
{
	// Make sure it has not already been checked
	if(!BlocksToFall.Blocks.Contains(Block))
	{
		// Check bounds
		if(Block.X >= 0 && Block.Y >= 0 && Block.Z >= 0 &&
			Block.X < GameConstants::WorldBlockCount && Block.Y < GameConstants::WorldBlockCount && Block.Z < GameConstants::WorldHeight)
		{
			// Make sure not do make an air block fall
			if(!GameState->IsBlockAirAt(Block))
			{
				// Make this block fall
				BlocksToFall.Blocks.Add(Block);

				// Check if the tension level of this block is withing breakof tension
				if(DistanceFromOrigin < MaxTension)
				{
					BreakOfBlocksUnderTension({Block.X - 1, Block.Y, Block.Z}, DistanceFromOrigin + 1, MaxTension, BlocksToFall);
					BreakOfBlocksUnderTension({Block.X, Block.Y - 1, Block.Z}, DistanceFromOrigin + 1, MaxTension, BlocksToFall);
					BreakOfBlocksUnderTension({Block.X, Block.Y, Block.Z - 1}, DistanceFromOrigin + 1, MaxTension, BlocksToFall);
					BreakOfBlocksUnderTension({Block.X + 1, Block.Y, Block.Z}, DistanceFromOrigin + 1, MaxTension, BlocksToFall);
					BreakOfBlocksUnderTension({Block.X, Block.Y + 1, Block.Z}, DistanceFromOrigin + 1, MaxTension, BlocksToFall);
					BreakOfBlocksUnderTension({Block.X, Block.Y, Block.Z + 1}, DistanceFromOrigin + 1, MaxTension, BlocksToFall);
				}
			}
		}
	}
}


bool operator==(const FTensionNode& A, const FTensionNode& B)
{
	
	return A.Position == B.Position;
	
}
