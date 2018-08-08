// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldLoader.h"
#include "FastNoise/FastNoise.h"
#include "GameConstants.h"
#include "InGame/IGGameState.h"
#include "InGame/IGPlayerController.h"
#include "InGame/IGHUD.h"

UWorldLoader::~UWorldLoader()
{

	if(bRunThread)
	{
		LoadThread->Kill(true);
	}

}

void UWorldLoader::Exit()
{

	// Broadcast that the world has finished loading on the Game Thread
	FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
	{

		OnWorldLoaded.Broadcast();

	}, TStatId(), NULL, ENamedThreads::GameThread);

	bRunThread = false;
}

bool UWorldLoader::Init()
{
	bRunThread = true;

	return true;
}

void UWorldLoader::LoadNewWorld(class AIGGameState* GameState, bool bProcedural)
{

	this->bProcedural = bProcedural;
	this->GameState = GameState;

	APlayerController* LocalController = UGameplayStatics::GetPlayerController(GameState, 0);

	if(LocalController)
	{

		HUD = Cast<AIGHUD>(Cast<AIGPlayerController>(LocalController)->GetHUD());

	}

	LoadThread = FRunnableThread::Create(this, TEXT("World Load Thread"));

}

uint32 UWorldLoader::Run()
{

	int32 ProgressID = 0;
	float LoadProgress = 0.f;
	if(HUD)
	{
		ProgressID = HUD->InsertNewProgressItem(TEXT("Loading World"));
	}

	if(bProcedural)
	{
		// Makeshift world gen
		FastNoise Noise(0);

		for(int32 x = 0; x < GameConstants::WorldBlockCount; x++)
		{
			for(int32 y = 0; y < GameConstants::WorldBlockCount; y++)
			{
				int32 TerrainHeight = (GameConstants::WorldHeight / 2) * ((Noise.GetSimplexFractal(x, y) + 1) / 2);

				for(int32 z = 0; z < GameConstants::WorldHeight; z++)
				{
					GameState->SetBlockIsAirAt(x, y, z, z > TerrainHeight);

					FBlockColor color;

					color.r = (uint8)(((Noise.GetSimplexFractal(x, y, z) + 1.f) / 2.f) * 255);
					color.g = (uint8)(((Noise.GetSimplexFractal(x, y, z + 1000.f) + 1.f) / 2.f) * 255);
					color.b = (uint8)(((Noise.GetSimplexFractal(x, y, z + 2000.f) + 1.f) / 2.f) * 255);

					GameState->SetBlockColorAt(x, y, z, color);
				}

				// Update load proggress
				if(HUD)
				{
					LoadProgress += 1.f / (GameConstants::WorldBlockCount * GameConstants::WorldBlockCount);

					HUD->UpdateProgressItem(ProgressID, LoadProgress);
				}
			}
		}

	}

	if(HUD)
	{
		HUD->RemoveProgressItem(ProgressID);
	}

	return 0;

}

void UWorldLoader::Stop()
{

	bRunThread = false;

}
