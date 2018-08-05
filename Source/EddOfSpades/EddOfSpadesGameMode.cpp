// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "EddOfSpadesGameMode.h"
#include "EddOfSpadesHUD.h"
#include "Network/ServerTCP.h"
#include "EddOfSpadesCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "World/WorldLoader.h"
#include "World/BlockPhysics.h"
#include "World/FallingBlocks.h"
#include "GameConstants.h"
#include "Network/WorldTransferProtocol.h"

AEddOfSpadesGameMode::AEddOfSpadesGameMode()
	: Super()
	, ServerTCP(NULL)
{

	// Setup defaults
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_EddOfSpadesCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	static ConstructorHelpers::FClassFinder<ASpectatorPawn> SpectatorPawnClassFinder(TEXT("/Game/Blueprints/BP_EddOfSpadesSpectatorPawn"));
	SpectatorClass = SpectatorPawnClassFinder.Class;

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassFinder(TEXT("/Game/Blueprints/BP_EddOfSpadesPlayerController"));
	PlayerControllerClass = PlayerControllerClassFinder.Class;

	static ConstructorHelpers::FClassFinder<AEddOfSpadesGameState> GameStateClassFinder(TEXT("/Game/Blueprints/BP_EddOfSpadesGameState"));
	GameStateClass = GameStateClassFinder.Class;

	static ConstructorHelpers::FClassFinder<AEddOfSpadesHUD> HUDClassFinder(TEXT("/Game/Blueprints/BP_EddOfSpadesHUD"));
	HUDClass = HUDClassFinder.Class;

	bStartPlayersAsSpectators = true;
	bDelayedStart = true;

}

void AEddOfSpadesGameMode::PostLogin(APlayerController* NewPlayer)
{

	Super::PostLogin(NewPlayer);

	// Make sure to start the TCP server
	if (!ServerTCP)
	{
		ServerTCP = NewObject<UServerTCP>();
		ServerTCP->StartServerTCP();
	}

	// Add the new player to the player list
	AEddOfSpadesPlayerController* Player = Cast<AEddOfSpadesPlayerController>(NewPlayer);

	PlayerControllers.Add(Player);

	// Add the new player to the TCP system
	ServerTCP->AddClientForConnection(Player);
}

void AEddOfSpadesGameMode::StartPlay()
{

	Super::StartPlay();

}

void AEddOfSpadesGameMode::SendWorldToAllConnectedClients()
{

	// Loop through all players
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AEddOfSpadesPlayerController* Player = Cast<AEddOfSpadesPlayerController>(*It);

		// Make sure to not send data to self, if there is a local player on the server
		if(GetNetMode() == NM_ListenServer)
		{
			if(Player == GetWorld()->GetFirstPlayerController())
			{
				continue;
			}
		}

		// Send the world to the client
		WorldTransfer->SendWorldToClient(ServerTCP, GameState, Player);
	}

}

void AEddOfSpadesGameMode::RespawnPlayer(AEddOfSpadesPlayerController* Player)
{
	// Destroy the old pawn, if any
	if (Player->GetPawn())
	{
		Player->GetPawn()->Destroy();
	}

	// Spawn the new pawn
	AEddOfSpadesCharacter* Pawn = GetWorld()->SpawnActor<AEddOfSpadesCharacter>(DefaultPawnClass);

	Pawn->SetActorLocation(FVector(
		(GameConstants::WorldBlockCount / 2) * GameConstants::BlockSize, 
		(GameConstants::WorldBlockCount / 2) * GameConstants::BlockSize, 
		GameConstants::WorldHeight * GameConstants::BlockSize));

	// Finally, possess the new pawn
	Player->Possess(Pawn);

}

void AEddOfSpadesGameMode::OnBlockChanged(const FIntVector& Block)
{

	// Check block physics
	FBlockPhysicsResult OutResult;
	if(BlockPhysics->CheckIfBlocksWillFall(GameState, Block, OutResult))
	{
		// Set all the falling blocks to air
		for(const FFallingBlocksList& FallingConstruct : OutResult.BlocksToFall)
		{
			for(const FIntVector& FallingBlock : FallingConstruct.Blocks)
			{
				GameState->SetBlockIsAirAt(FallingBlock, true);

				// Update all clients
				for(auto It = GetWorld()->GetPlayerControllerIterator(); It; It++)
				{
					AEddOfSpadesPlayerController* Client = Cast<AEddOfSpadesPlayerController>(*It);

					Client->ClientBlockChanged(FallingBlock, GameState->GetBlockAt(FallingBlock), false);
				}
			}
		}

		// Tell clients to refresh chunk
		int32 ChunkXMin = OutResult.MinBlock.X / GameConstants::ChunkSize - 1;
		int32 ChunkYMin = OutResult.MinBlock.Y / GameConstants::ChunkSize - 1;
		int32 ChunkXMax = OutResult.MaxBlock.X / GameConstants::ChunkSize + 1;
		int32 ChunkYMax = OutResult.MaxBlock.Y / GameConstants::ChunkSize + 1;
		for(auto It = GetWorld()->GetPlayerControllerIterator(); It; It++)
		{
			AEddOfSpadesPlayerController* Client = Cast<AEddOfSpadesPlayerController>(*It);

			Client->ClientForceChunkSectionRefresh(ChunkXMin, ChunkYMin, ChunkXMax, ChunkYMax);

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

void AEddOfSpadesGameMode::OnStartButtonPressed()
{

	// Start loading the world
	WorldLoader->LoadNewWorld(GameState, true);

}

void AEddOfSpadesGameMode::OnWorldLoaded()
{

	// The world has finished loading
	SendWorldToAllConnectedClients();

	// If a local player, tell it to build the world mesh
	if(GetNetMode() == NM_ListenServer || GetNetMode() == NM_Standalone)
	{

		Cast<AEddOfSpadesPlayerController>(GetWorld()->GetFirstPlayerController())->RebuildWorldMesh();

	}

}

void AEddOfSpadesGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Prepare game object references
	GameState = GetGameState<AEddOfSpadesGameState>();

	// Create the world loader
	WorldLoader = NewObject<UWorldLoader>();
	WorldLoader->OnWorldLoaded.AddDynamic(this, &AEddOfSpadesGameMode::OnWorldLoaded);

	// Create the world transfer
	WorldTransfer = NewObject<UWorldTransferProtocol>();

	// Create the block physics
	BlockPhysics = NewObject<UBlockPhysics>();

	// If this is a listen server, bind appropriate delegates
	if(GetNetMode() == NM_ListenServer || GetNetMode() == NM_Standalone)
	{
		AEddOfSpadesPlayerController* ServerPlayer = Cast<AEddOfSpadesPlayerController>(GetWorld()->GetFirstPlayerController());

		ServerPlayer->OnStartButtonPressed.AddDynamic(this, &AEddOfSpadesGameMode::OnStartButtonPressed);
	}
}

void AEddOfSpadesGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	FString StateString;
	MatchState.ToString(StateString);

	UE_LOG(LogTemp, Display, TEXT("Match state has been set to: %s"), *StateString);
}

