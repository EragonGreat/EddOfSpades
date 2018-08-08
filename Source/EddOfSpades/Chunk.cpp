// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"
#include "GameConstants.h"
#include "InGame/IGGameState.h"
#include "ConstructorHelpers.h"
#include "KismetProceduralMeshLibrary.h"

// Sets default values
AChunk::AChunk()
	: bMeshIsGenerated(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RuntimeMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("Runtime Mesh"));
	RuntimeMesh->SetCollisionObjectType(ECC_GameTraceChannel2);
	

	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Game/Materials/BlockMaterial.BlockMaterial'"));
	if (Material.Succeeded())
	{
		UMaterial* BlockMaterial = Material.Object;
		RuntimeMesh->SetMaterial(0, BlockMaterial);
	}

	RootComponent = RuntimeMesh;

}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{

	Super::BeginPlay();
	
	GameState = GetWorld()->GetGameState<AIGGameState>();

}

void AChunk::AddBlockMeshVertices(int32 BlockX, int32 BlockY, int32 BlockZ, 
	TArray<FVector>& Positions,
	TArray<int32>& Indices,
	TArray<FVector2D>& UVs,
	TArray<FVector>& Normals,
	TArray<FColor>& VertexColors,
	TArray<FRuntimeMeshTangent>& Tangents)
{
	if (GameState->IsBlockAirAt(BlockX, BlockY, BlockZ))
	{
		return;
	}

	int32 CBlockX = BlockX % GameConstants::ChunkSize;
	int32 CBlockY = BlockY % GameConstants::ChunkSize;

	// Predefine block vertex positions
	FVector v0(CBlockX * 100.f,			CBlockY * 100.f,		BlockZ * 100.f);
	FVector v1(CBlockX * 100.f,			(CBlockY + 1) * 100.f,	BlockZ * 100.f);
	FVector v2(CBlockX * 100.f,			CBlockY * 100.f,		(BlockZ + 1) * 100.f);
	FVector v3(CBlockX * 100.f,			(CBlockY + 1) * 100.f,	(BlockZ + 1) * 100.f);
	FVector v4((CBlockX + 1) * 100.f,	CBlockY * 100.f,		BlockZ * 100.f);
	FVector v5((CBlockX + 1) * 100.f,	(CBlockY + 1) * 100.f,	BlockZ * 100.f);
	FVector v6((CBlockX + 1) * 100.f,	CBlockY * 100.f,		(BlockZ + 1) * 100.f);
	FVector v7((CBlockX + 1) * 100.f,	(CBlockY + 1) * 100.f,	(BlockZ + 1) * 100.f);

	int32 FaceCount = 0;
	int32 LastIndex = Positions.Num();

	// +X
	if (BlockX < GameConstants::WorldBlockCount - 1 && GameState->IsBlockAirAt(BlockX + 1, BlockY, BlockZ))
	{
		// Positions
		Positions.Append({ v5, v4, v7, v6 });//v7, v4, v6, v7, v5, v4 });

		// Normals
		for(int i = 0; i < 4; i++) Normals.Add(FVector(1.f, 0.f, 0.f));
		
		FaceCount++;
	}

	// -X
	if (BlockX > 0 && GameState->IsBlockAirAt(BlockX - 1, BlockY, BlockZ))
	{
		// Positions
		Positions.Append({ v0, v1, v2, v3 });//v2, v1, v3, v2, v0, v1 });

		// Normals
		for (int i = 0; i < 4; i++) Normals.Add(FVector(-1.f, 0.f, 0.f));

		FaceCount++;
	}

	// +Y
	if (BlockY < GameConstants::WorldBlockCount - 1 && GameState->IsBlockAirAt(BlockX, BlockY + 1, BlockZ))
	{
		// Positions
		Positions.Append({ v1, v5, v3, v7 });// v3, v5, v7, v3, v1, v5 });

		// Normals
		for (int i = 0; i < 4; i++) Normals.Add(FVector(0.f, 1.f, 0.f));

		FaceCount++;
	}

	// -Y
	if (BlockY > 0 && GameState->IsBlockAirAt(BlockX, BlockY - 1, BlockZ))
	{
		// Positions
		Positions.Append({ v4, v0, v6, v2 });

		// Normals
		for (int i = 0; i < 4; i++) Normals.Add(FVector(0.f, -1.f, 0.f));

		FaceCount++;
	}

	// +Z
	if (BlockZ < GameConstants::WorldHeight - 1 && GameState->IsBlockAirAt(BlockX, BlockY, BlockZ + 1))
	{
		// Positions
		Positions.Append({ v2, v3, v6, v7 });

		// Normals
		for (int i = 0; i < 4; i++) Normals.Add(FVector(0.f, 0.f, 1.f));

		FaceCount++;
	}

	// -Z
	if (BlockZ > 0 && GameState->IsBlockAirAt(BlockX, BlockY, BlockZ - 1))
	{
		// Positions
		Positions.Append({ v4, v5, v0, v1 });

		// Normals
		for (int i = 0; i < 4; i++) Normals.Add(FVector(0.f, 0.f, -1.f));

		FaceCount++;
	}

	// Get the block color
	FBlockColor Color = GameState->GetBlockColorAt(BlockX, BlockY, BlockZ);
	FColor VertexColor(Color.r, Color.g, Color.b);

	for(int i = 0; i < FaceCount; i++)
	{
		//UVs.Append({{0.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, {1.f, 1.f}});

		int32 StartIndex = LastIndex + i * 4;
		Indices.Append({StartIndex + 2, StartIndex + 0, StartIndex + 3, StartIndex + 0, StartIndex + 1, StartIndex + 3});

		for(int j = 0; j < 4; j++)
		{

			VertexColors.Add(VertexColor);

		}
	}
}

void AChunk::GenerateMeshFromBlockData()
{
	MeshMutex.Lock();

	Positions.Empty();
	Indices.Empty();
	UVs.Empty();
	Normals.Empty();
	VertexColors.Empty();
	Tangents.Empty();

	for(int32 X = ChunkX * GameConstants::ChunkSize; X < GameConstants::ChunkSize + ChunkX * GameConstants::ChunkSize; X++)
	{
		for(int32 Y = ChunkY * GameConstants::ChunkSize; Y < GameConstants::ChunkSize + ChunkY * GameConstants::ChunkSize; Y++)
		{
			for(int32 Z = 0; Z < GameConstants::WorldHeight; Z++)
			{
				AddBlockMeshVertices(X, Y, Z, Positions, Indices, UVs, Normals, VertexColors, Tangents);
			}
		}
	}

	MeshMutex.Unlock();

	// Dispatch changes
	FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
	{

		MeshMutex.Lock();

		if(!bMeshIsGenerated) 
		{
			RuntimeMesh->CreateMeshSection(0, Positions, Indices, Normals, UVs, VertexColors, Tangents, true, EUpdateFrequency::Infrequent, ESectionUpdateFlags::None, false, false);
			bMeshIsGenerated = true;
		}
		else
		{
			RuntimeMesh->UpdateMeshSection(0, Positions, Indices, Normals, UVs, VertexColors, Tangents);
		}

		Positions.Empty();
		Indices.Empty();
		UVs.Empty();
		Normals.Empty();
		VertexColors.Empty();
		Tangents.Empty();

		MeshMutex.Unlock();

	}, TStatId(), NULL, ENamedThreads::GameThread);



}

void AChunk::SetChunkPosition(int32 NewChunkX, int32 NewChunkY)
{
	ChunkX = NewChunkX;
	ChunkY = NewChunkY;
}

void AChunk::GetChunkPosition(int32& OutChunkX, int32& OutChunkY)
{
	OutChunkX = ChunkX;
	OutChunkY = ChunkY;
}
