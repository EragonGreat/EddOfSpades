// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkSpawner.h"
#include "EngineUtils.h"
#include "EddOfSpadesPlayerController.h"
#include "EddOfSpadesHUD.h"
#include "GameConstants.h"

// Sets default values
AChunkSpawner::AChunkSpawner()
	: bRunThread(false)
	, bStartWorldReload(false)
	, ChunkThread(nullptr)
	, HUD(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

AChunkSpawner::~AChunkSpawner()
{
	if(bRunThread)
	{
		ChunkThread->Kill(true);
	}
}

void AChunkSpawner::RebuildWorldMesh()
{
	// Get a reference to the HUD, this is to update progress bars and such
	AEddOfSpadesPlayerController* LocalController = Cast<AEddOfSpadesPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if(LocalController)
	{
		HUD = Cast<AEddOfSpadesHUD>(LocalController->GetHUD());
	}

	// Delete all the old chunks, if any
	for (AChunk* Chunk : SpawnedChunks) 
	{
		Chunk->Destroy();
	}
	SpawnedChunks.Empty();

	// Spawn all chunks
	for(int32 Y = 0; Y < GameConstants::WorldChunkCount; Y++)
	{
		for(int32 X = 0; X < GameConstants::WorldChunkCount; X++)
		{
			FVector Location = FVector(X * GameConstants::ChunkSize * GameConstants::BlockSize, Y * GameConstants::ChunkSize * GameConstants::BlockSize, 0.f);

			AChunk* Chunk = GetWorld()->SpawnActor<AChunk>(Location, FRotator(0.f, 0.f, 0.f));

			Chunk->SetChunkPosition(X, Y);

			// Save the chunk in an array
			SpawnedChunks.Push(Chunk);

		}
	}

	// Make a separate thread actually generate the chunk meshes
	bStartWorldReload = true;
	if(!bRunThread)
	{
		bRunThread = true;
		ChunkThread = FRunnableThread::Create(this, TEXT("Chunk Mesh Generation Thread"), 0, TPri_BelowNormal);
	}

}

void AChunkSpawner::RebuildSingleChunk(int32 ChunkX, int32 ChunkY)
{
	if(ChunkX < 0 || ChunkY < 0 || ChunkX >= GameConstants::WorldChunkCount || ChunkY >= GameConstants::WorldChunkCount)
	{
		return;
	}

	ChunksToBuild.Enqueue(SpawnedChunks[ChunkX + ChunkY * GameConstants::WorldChunkCount]);
}

void AChunkSpawner::BlockUpdatedInChunk(int32 ChunkX, int32 ChunkY)
{
	RebuildSingleChunk(ChunkX + 1, ChunkY);
	RebuildSingleChunk(ChunkX, ChunkY + 1);
	RebuildSingleChunk(ChunkX - 1, ChunkY);
	RebuildSingleChunk(ChunkX, ChunkY - 1);
	RebuildSingleChunk(ChunkX, ChunkY);
}

uint32 AChunkSpawner::Run()
{

	while (bRunThread)
	{

		if(bStartWorldReload)
		{
			bStartWorldReload = false;

			// Craete a load progress bar for generating world mesh
			int32 ProgressID = 0;
			if(HUD)
			{
				ProgressID = HUD->InsertNewProgressItem(TEXT("Building World Mesh"));
			}
			int32 ChunksBuilt = 0;
			for(int32 Y = 0; Y < GameConstants::WorldChunkCount; Y++)
			{
				for(int32 X = 0; X < GameConstants::WorldChunkCount; X++)
				{

					SpawnedChunks[X + Y * GameConstants::WorldChunkCount]->GenerateMeshFromBlockData();

					// Update progress
					if(HUD)
					{
						ChunksBuilt++;
						HUD->UpdateProgressItem(ProgressID, (float)ChunksBuilt / GameConstants::TotalChunks);
					}

				}
			}

			// Cleanup progress bar
			if(HUD)
			{
				HUD->RemoveProgressItem(ProgressID);
			}

			// Dispatch changes
			FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
			{

				OnWorldMeshBuilt.Broadcast();

			}, TStatId(), NULL, ENamedThreads::GameThread);

		}

		if(!ChunksToBuild.IsEmpty())
		{
	
			AChunk* ChunkToBuild;
			ChunksToBuild.Dequeue(ChunkToBuild);
			ChunkToBuild->GenerateMeshFromBlockData();
			
		}
	}


	return 0;
}

void AChunkSpawner::Stop()
{
	bRunThread = false;
}
