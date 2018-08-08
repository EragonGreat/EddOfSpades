// Fill out your copyright notice in the Description page of Project Settings.

#include "IGPlayerController.h"
#include "IGGameState.h"
#include "IGGameMode.h"
#include "Network/ClientTCP.h"
#include "IGCharacter.h"
#include "ChunkSpawner.h"
#include "World/FallingBlocks.h"
#include "Network/WorldTransferProtocol.h"

void AIGPlayerController::BeginPlay()
{

	Super::BeginPlay();
	
	SelectedColor.r = 0;
	SelectedColor.g = 0;
	SelectedColor.b = 0;

	// Get Game class references
	GameState = GetWorld()->GetGameState<AIGGameState>();
	GameMode = Cast<AIGGameMode>(GetWorld()->GetAuthGameMode());

	// Create the client TCP only if this is on remote machine
	if (IsNetMode(ENetMode::NM_Client))
	{
		ClientTCP = NewObject<UClientTCP>();
		ClientTCP->StartTCPSocket(this);

		// Wait for the world to be received
		WorldTransfer = NewObject<UWorldTransferProtocol>();
		WorldTransfer->ReceiveWorldFromServer(ClientTCP, GameState, this);
		WorldTransfer->OnWorldReceived.AddDynamic(this, &AIGPlayerController::OnWorldReceived);
	}

}

void AIGPlayerController::OnWorldReceived()
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

void AIGPlayerController::OnWorldMeshBuilt()
{

	ServerRequestSpawn();

}

void AIGPlayerController::ClientReceiveNewChunk(const FChunkData& Chunk, int32 ChunkX, int32 ChunkY)
{

	GameState->SetChunk(Chunk, ChunkX, ChunkY);

}

void AIGPlayerController::ClientBlockChanged_Implementation(const FIntVector& BlockPos, const FBlockData& NewBlockData)
{

	GameState->SetBlockAt(BlockPos, NewBlockData);

}

void AIGPlayerController::ServerPlaceBlock_Implementation(const FIntVector& BlockPosition, const FBlockColor BlockColor)
{
	FBlockData PlacedBlock;
	PlacedBlock.bIsAir = false;
	PlacedBlock.Color = BlockColor;

	GameMode->UpdateBlock(BlockPosition, PlacedBlock);

}

bool AIGPlayerController::ServerPlaceBlock_Validate(const FIntVector& BlockPosition, const FBlockColor BlockColor)
{
	return true;
}

void AIGPlayerController::ServerDamageBlock_Implementation(const FIntVector& BlockPosition)
{
	FBlockData DestroyedBlock;
	DestroyedBlock.bIsAir = true;
	DestroyedBlock.Color.r = 0;
	DestroyedBlock.Color.g = 0;
	DestroyedBlock.Color.b = 0;

	GameMode->UpdateBlock(BlockPosition, DestroyedBlock);
}

bool AIGPlayerController::ServerDamageBlock_Validate(const FIntVector& BlockPosition)
{
	return true;
}

bool AIGPlayerController::ServerStartButonPressed_Validate()
{
	return true;
}

void AIGPlayerController::ServerStartButonPressed_Implementation()
{

	if(GameMode)
	{
		GameMode->OnStartButtonPressed();
	}

}

void AIGPlayerController::ServerRequestSpawn_Implementation()
{

	// Possess the default pawn for the player that finished loading
	GameMode->RespawnPlayer(this);

}

bool AIGPlayerController::ServerRequestSpawn_Validate()
{

	return true;

}

