// Fill out your copyright notice in the Description page of Project Settings.

#include "EddOfSpadesPlayerController.h"
#include "EddOfSpadesGameState.h"
#include "EddOfSpadesGameMode.h"
#include "Network/ClientTCP.h"
#include "EddOfSpadesCharacter.h"
#include "ChunkSpawner.h"
#include "World/FallingBlocks.h"
#include "Network/WorldTransferProtocol.h"

void AEddOfSpadesPlayerController::BeginPlay()
{

	Super::BeginPlay();
	
	SelectedColor.r = 0;
	SelectedColor.g = 0;
	SelectedColor.b = 0;

	// Get Game class references
	GameState = GetWorld()->GetGameState<AEddOfSpadesGameState>();
	GameMode = Cast<AEddOfSpadesGameMode>(GetWorld()->GetAuthGameMode());

	if(IsLocalPlayerController())
	{
		// If this player controller is on the owning machine then spawn actors accordingly
		ChunkSpawner = GetWorld()->SpawnActor<AChunkSpawner>();
		ChunkSpawner->OnWorldMeshBuilt.AddDynamic(this, &AEddOfSpadesPlayerController::OnWorldMeshBuilt);

	}

	// Create the client TCP only if this is on remote machine
	if (IsNetMode(ENetMode::NM_Client))
	{
		ClientTCP = NewObject<UClientTCP>();
		ClientTCP->StartTCPSocket(this);

		// Wait for the world to be received
		WorldTransfer = NewObject<UWorldTransferProtocol>();
		WorldTransfer->ReceiveWorldFromServer(ClientTCP, GameState, this);
		WorldTransfer->OnWorldReceived.AddDynamic(this, &AEddOfSpadesPlayerController::OnWorldReceived);
	}

}

void AEddOfSpadesPlayerController::OnWorldReceived()
{

	RebuildWorldMesh();

}

void AEddOfSpadesPlayerController::OnWorldMeshBuilt()
{

	ServerRequestSpawn();

}

void AEddOfSpadesPlayerController::RebuildWorldMesh()
{

	ChunkSpawner->RebuildWorldMesh();

}

void AEddOfSpadesPlayerController::ClientReceiveNewChunk(const FChunkData& Chunk, int32 ChunkX, int32 ChunkY)
{

	GameState->SetChunk(Chunk, ChunkX, ChunkY);

}

void AEddOfSpadesPlayerController::ClientBlocksFellDown_Implementation(const TArray<FIntVector>& FallingBlocks)
{


}

void AEddOfSpadesPlayerController::ClientForceChunkSectionRefresh_Implementation(const int32& ChunkXMin, const int32& ChunkYMin, const int32& ChunkXMax, const int32& ChunkYMax)
{

	if(ChunkSpawner)
	{
		for(int32 i = ChunkXMin; i <= ChunkXMax; i++)
		{
			for(int32 j = ChunkYMin; j <= ChunkYMax; j++)
			{

				ChunkSpawner->RebuildSingleChunk(i, j);

			}
		}
	}

}

void AEddOfSpadesPlayerController::ClientForceChunkRefresh_Implementation(const int32& ChunkX, const int32& ChunkY)
{

	if(ChunkSpawner)
	{
		ChunkSpawner->RebuildSingleChunk(ChunkX, ChunkY);
	}

}

void AEddOfSpadesPlayerController::ClientBlockChanged_Implementation(const FIntVector& BlockPos, const FBlockData& NewBlockData, bool bShouldRefreshChunk)
{
	if(ChunkSpawner)
	{
		GameState->SetBlockAt(BlockPos, NewBlockData);

		if(bShouldRefreshChunk)
		{
			ChunkSpawner->BlockUpdatedInChunk(BlockPos.X / GameConstants::ChunkSize, BlockPos.Y / GameConstants::ChunkSize);
		}
	}
}

void AEddOfSpadesPlayerController::ServerPlaceBlock_Implementation(const FIntVector& BlockPosition, const FBlockColor BlockColor)
{
	FBlockData PlacedBlock;
	PlacedBlock.bIsAir = false;
	PlacedBlock.Color = BlockColor;

	// Update the server block first
	GameState->SetBlockAt(BlockPosition, PlacedBlock);

	// Update the clients
	for(auto It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{

		AEddOfSpadesPlayerController* Player = Cast<AEddOfSpadesPlayerController>(*It);

		Player->ClientBlockChanged(BlockPosition, PlacedBlock, true);

	}

	GameMode->OnBlockChanged(BlockPosition);
}

bool AEddOfSpadesPlayerController::ServerPlaceBlock_Validate(const FIntVector& BlockPosition, const FBlockColor BlockColor)
{
	return true;
}

void AEddOfSpadesPlayerController::ServerDamageBlock_Implementation(const FIntVector& BlockPosition)
{
	FBlockData DestroyedBlock;
	DestroyedBlock.bIsAir = true;
	DestroyedBlock.Color.r = 0;
	DestroyedBlock.Color.g = 0;
	DestroyedBlock.Color.b = 0;

	// Update the server block first
	GameState->SetBlockAt(BlockPosition, DestroyedBlock);

	for(auto It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{

		AEddOfSpadesPlayerController* Player = Cast<AEddOfSpadesPlayerController>(*It);

		Player->ClientBlockChanged(BlockPosition, DestroyedBlock, true);
		
	}

	GameMode->OnBlockChanged(BlockPosition);
}

bool AEddOfSpadesPlayerController::ServerDamageBlock_Validate(const FIntVector& BlockPosition)
{
	return true;
}

bool AEddOfSpadesPlayerController::ServerStartButonPressed_Validate()
{
	return true;
}

void AEddOfSpadesPlayerController::ServerStartButonPressed_Implementation()
{

	if(GameMode)
	{
		GameMode->OnStartButtonPressed();
	}

}

void AEddOfSpadesPlayerController::ServerRequestSpawn_Implementation()
{

	// Possess the default pawn for the player that finished loading
	GameMode->RespawnPlayer(this);

}

bool AEddOfSpadesPlayerController::ServerRequestSpawn_Validate()
{

	return true;

}

