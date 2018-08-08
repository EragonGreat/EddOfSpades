// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "IGGameMode.h"
#include "IGHUD.h"
#include "Network/ServerTCP.h"
#include "IGCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "World/WorldLoader.h"
#include "World/BlockPhysics.h"
#include "World/FallingBlocks.h"
#include "ChunkSpawner.h"
#include "Network/BlockTransfer.h"
#include "GameConstants.h"
#include "Network/WorldTransferProtocol.h"

AIGGameMode::AIGGameMode()
	: Super()
	, ServerTCP(nullptr)
	, BlockTransfer(nullptr)
{

	// Setup defaults
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/InGame/BP_EddOfSpadesCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	static ConstructorHelpers::FClassFinder<ASpectatorPawn> SpectatorPawnClassFinder(TEXT("/Game/Blueprints/InGame/BP_EddOfSpadesSpectatorPawn"));
	SpectatorClass = SpectatorPawnClassFinder.Class;

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassFinder(TEXT("/Game/Blueprints/InGame/BP_EddOfSpadesPlayerController"));
	PlayerControllerClass = PlayerControllerClassFinder.Class;

	static ConstructorHelpers::FClassFinder<AIGGameState> GameStateClassFinder(TEXT("/Game/Blueprints/InGame/BP_EddOfSpadesGameState"));
	GameStateClass = GameStateClassFinder.Class;

	static ConstructorHelpers::FClassFinder<AIGHUD> HUDClassFinder(TEXT("/Game/Blueprints/InGame/BP_EddOfSpadesHUD"));
	HUDClass = HUDClassFinder.Class;

	bStartPlayersAsSpectators = true;
	bDelayedStart = true;

}

void AIGGameMode::PostLogin(APlayerController* NewPlayer)
{

	Super::PostLogin(NewPlayer);

	// Make sure to start the TCP server
	if (!ServerTCP)
	{
		ServerTCP = NewObject<UServerTCP>();
		ServerTCP->StartServerTCP();
	}

	// Add the new player to the player list
	AIGPlayerController* Player = Cast<AIGPlayerController>(NewPlayer);

	PlayerControllers.Add(Player);

	// Add the new player to the TCP system
	ServerTCP->AddClientForConnection(Player);
}

void AIGGameMode::StartPlay()
{

	Super::StartPlay();

}

void AIGGameMode::SendWorldToAllConnectedClients()
{

	// Loop through all players
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AIGPlayerController* Player = Cast<AIGPlayerController>(*It);

		// Make sure to not send data to self, if there is a local player on the server
		if(GetNetMode() == NM_ListenServer)
		{
			if(Player == GetWorld()->GetFirstPlayerController())
			{
				continue;
			}
		}

		// Send the world to the client
		WorldTransfer->SendWorldToClient(ServerTCP, EddGameState, Player);
	}

}

void AIGGameMode::RespawnPlayer(AIGPlayerController* Player)
{
	// Destroy the old pawn, if any
	if (Player->GetPawn())
	{
		Player->GetPawn()->Destroy();
	}

	// Spawn the new pawn
	AIGCharacter* Pawn = GetWorld()->SpawnActor<AIGCharacter>(DefaultPawnClass);

	Pawn->SetActorLocation(FVector(
		(GameConstants::WorldBlockCount / 2) * GameConstants::BlockSize, 
		(GameConstants::WorldBlockCount / 2) * GameConstants::BlockSize, 
		GameConstants::WorldHeight * GameConstants::BlockSize));

	// Finally, possess the new pawn
	Player->Possess(Pawn);

}

void AIGGameMode::UpdateBlock(const FIntVector& Position, const FBlockData& NewBlock)
{

	// Update the block
	BlockTransfer->UpdateSingleBlock(Position, NewBlock);

	// Check block physics
	FBlockPhysicsResult OutResult;
	if(BlockPhysics->CheckIfBlocksWillFall(EddGameState, Position, OutResult))
	{
		// Set all the falling blocks to air
		for(const FFallingBlocksList& FallingConstruct : OutResult.BlocksToFall)
		{
			if(FallingConstruct.Blocks.Num() > 0)
			{
				BlockTransfer->DestroyedSeveralBlocks(FallingConstruct.Blocks);
			}
		}

		// Spawn the falling blocks actor on the server, replicating it to the clients
		for(const FFallingBlocksList& FallingConstruct : OutResult.BlocksToFall)
		{
			if(FallingConstruct.Blocks.Num() > 0)
			{
				AFallingBlocks* FallingBlocksActor = GetWorld()->SpawnActor<AFallingBlocks>(FVector(0, 0, 0), FRotator(0, 0, 0));
				FallingBlocksActor->SetBlocksThatWillFall(FallingConstruct.Blocks);
			}
		}
	}
}

void AIGGameMode::OnStartButtonPressed()
{

	// Start loading the world
	WorldLoader->LoadNewWorld(EddGameState, true);

}

void AIGGameMode::OnWorldLoaded()
{

	// Send the world data to the clients
	SendWorldToAllConnectedClients();

	// Rebuild the world mesh on the server first
	BlockTransfer->GetChunkSpawner()->RebuildWorldMesh();

}

void AIGGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Prepare game object references
	EddGameState = GetGameState<AIGGameState>();

	// Craete the chunkspawner
	BlockTransfer = GetWorld()->SpawnActor<ABlockTransfer>();

	// Create the world loader
	WorldLoader = NewObject<UWorldLoader>();
	WorldLoader->OnWorldLoaded.AddDynamic(this, &AIGGameMode::OnWorldLoaded);

	// Create the world transfer
	WorldTransfer = NewObject<UWorldTransferProtocol>();

	// Create the block physics
	BlockPhysics = NewObject<UBlockPhysics>();

	// If this is a listen server, bind appropriate delegates
	if(GetNetMode() == NM_ListenServer || GetNetMode() == NM_Standalone)
	{
		AIGPlayerController* ServerPlayer = Cast<AIGPlayerController>(GetWorld()->GetFirstPlayerController());

		ServerPlayer->OnStartButtonPressed.AddDynamic(this, &AIGGameMode::OnStartButtonPressed);
	}
}

void AIGGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	FString StateString;
	MatchState.ToString(StateString);

	UE_LOG(LogTemp, Display, TEXT("Match state has been set to: %s"), *StateString);
}

