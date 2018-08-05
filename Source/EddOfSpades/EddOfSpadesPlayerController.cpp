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

	// Rebuild the world mesh
	TArray<AActor*> ChunkSpawners;
	UGameplayStatics::GetAllActorsOfClass(this, AChunkSpawner::StaticClass(), ChunkSpawners);

	if(ChunkSpawners.Num() > 0)
	{
		AChunkSpawner* ChunkSpawner = Cast<AChunkSpawner>(ChunkSpawners[0]);
		ChunkSpawner->RebuildWorldMesh();
	}

}

void AEddOfSpadesPlayerController::OnWorldMeshBuilt()
{

	ServerRequestSpawn();

}

void AEddOfSpadesPlayerController::ClientReceiveNewChunk(const FChunkData& Chunk, int32 ChunkX, int32 ChunkY)
{

	GameState->SetChunk(Chunk, ChunkX, ChunkY);

}

void AEddOfSpadesPlayerController::ClientBlockChanged_Implementation(const FIntVector& BlockPos, const FBlockData& NewBlockData)
{

	GameState->SetBlockAt(BlockPos, NewBlockData);

}

void AEddOfSpadesPlayerController::ServerPlaceBlock_Implementation(const FIntVector& BlockPosition, const FBlockColor BlockColor)
{
	FBlockData PlacedBlock;
	PlacedBlock.bIsAir = false;
	PlacedBlock.Color = BlockColor;

	GameMode->UpdateBlock(BlockPosition, PlacedBlock);

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

	GameMode->UpdateBlock(BlockPosition, DestroyedBlock);
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

