// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"
#include "GameConstants.h"
#include "EddOfSpadesGameState.h"
#include "ConstructorHelpers.h"
#include "KismetProceduralMeshLibrary.h"

// Sets default values
AChunk::AChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	ProceduralMesh->SetCollisionObjectType(ECC_GameTraceChannel2);

	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Game/Materials/BlockMaterial.BlockMaterial'"));
	if (Material.Succeeded())
	{
		UMaterial* BlockMaterial = Material.Object;
		ProceduralMesh->SetMaterial(0, BlockMaterial);
	}

	RootComponent = ProceduralMesh;

}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();
	
	GameState = GetWorld()->GetGameState<AEddOfSpadesGameState>();

}

void AChunk::AddBlockMeshVertices(int32 BlockX, int32 BlockY, int32 BlockZ, 
	TArray<FVector>& Positions,
	TArray<int32>& Indices,
	TArray<FVector2D>& UVs,
	TArray<FVector>& Normals,
	TArray<FLinearColor>& VertexColors,
	TArray<FProcMeshTangent>& Tangents)
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

	// +X
	if (BlockX < GameConstants::WorldBlockCount - 1 && GameState->IsBlockAirAt(BlockX + 1, BlockY, BlockZ))
	{
		// Positions
		Positions.Append({ v7, v4, v6, v7, v5, v4 });

		// Normals
		for(int i = 0; i < 6; i++) Normals.Add(FVector(1.f, 0.f, 0.f));
		
		FaceCount++;
	}

	// -X
	if (BlockX > 0 && GameState->IsBlockAirAt(BlockX - 1, BlockY, BlockZ))
	{
		// Positions
		Positions.Append({ v2, v1, v3, v2, v0, v1 });

		// Normals
		for (int i = 0; i < 6; i++) Normals.Add(FVector(-1.f, 0.f, 0.f));

		FaceCount++;
	}

	// +Y
	if (BlockY < GameConstants::WorldBlockCount - 1 && GameState->IsBlockAirAt(BlockX, BlockY + 1, BlockZ))
	{
		// Positions
		Positions.Append({ v3, v5, v7, v3, v1, v5 });

		// Normals
		for (int i = 0; i < 6; i++) Normals.Add(FVector(0.f, 1.f, 0.f));

		FaceCount++;
	}

	// -Y
	if (BlockY > 0 && GameState->IsBlockAirAt(BlockX, BlockY - 1, BlockZ))
	{
		// Positions
		Positions.Append({ v6, v0, v2, v6, v4, v0 });

		// Normals
		for (int i = 0; i < 6; i++) Normals.Add(FVector(0.f, -1.f, 0.f));

		FaceCount++;
	}

	// +Z
	if (BlockZ < GameConstants::WorldHeight - 1 && GameState->IsBlockAirAt(BlockX, BlockY, BlockZ + 1))
	{
		// Positions
		Positions.Append({ v6, v3, v7, v6, v2, v3 });

		// Normals
		for (int i = 0; i < 6; i++) Normals.Add(FVector(0.f, 0.f, 1.f));

		FaceCount++;
	}

	// -Z
	if (BlockZ > 0 && GameState->IsBlockAirAt(BlockX, BlockY, BlockZ - 1))
	{
		// Positions
		Positions.Append({ v1, v0, v4, v1, v4, v5 });

		// Normals
		for (int i = 0; i < 6; i++) Normals.Add(FVector(0.f, 0.f, -1.f));

		FaceCount++;
	}

	// +X
	if (BlockX < GameConstants::WorldBlockCount - 1 && GameState->IsBlockAirAt(BlockX + 1, BlockY, BlockZ))
	{
		// Positions
		Positions.Append({ v7, v4, v6, v7, v5, v4 });

		// Normals
		for(int i = 0; i < 6; i++) Normals.Add(FVector(1.f, 0.f, 0.f));
		
		FaceCount++;
	}

	// -X
	if (BlockX > 0 && GameState->IsBlockAirAt(BlockX - 1, BlockY, BlockZ))
	{
		// Positions
		Positions.Append({ v2, v1, v3, v2, v0, v1 });

		// Normals
		for (int i = 0; i < 6; i++) Normals.Add(FVector(-1.f, 0.f, 0.f));

		FaceCount++;
	}

	// +Y
	if (BlockY < GameConstants::WorldBlockCount - 1 && GameState->IsBlockAirAt(BlockX, BlockY + 1, BlockZ))
	{
		// Positions
		Positions.Append({ v3, v5, v7, v3, v1, v5 });

		// Normals
		for (int i = 0; i < 6; i++) Normals.Add(FVector(0.f, 1.f, 0.f));

		FaceCount++;
	}

	// -Y
	if (BlockY > 0 && GameState->IsBlockAirAt(BlockX, BlockY - 1, BlockZ))
	{
		// Positions
		Positions.Append({ v6, v0, v2, v6, v4, v0 });

		// Normals
		for (int i = 0; i < 6; i++) Normals.Add(FVector(0.f, -1.f, 0.f));

		FaceCount++;
	}

	// +Z
	if (BlockZ < GameConstants::WorldHeight - 1 && GameState->IsBlockAirAt(BlockX, BlockY, BlockZ + 1))
	{
		// Positions
		Positions.Append({ v6, v3, v7, v6, v2, v3 });

		// Normals
		for (int i = 0; i < 6; i++) Normals.Add(FVector(0.f, 0.f, 1.f));

		FaceCount++;
	}

	// -Z
	if (BlockZ > 0 && GameState->IsBlockAirAt(BlockX, BlockY, BlockZ - 1))
	{
		// Positions
		Positions.Append({ v1, v4, v5, v1, v0, v4 });

		// Normals
		for (int i = 0; i < 6; i++) Normals.Add(FVector(0.f, 0.f, -1.f));

		FaceCount++;
	}
	
	// Get the block color
	FBlockColor Color = GameState->GetBlockColorAt(BlockX, BlockY, BlockZ);
	FLinearColor VertexColor = FLinearColor(Color.r / 255.f, Color.g / 255.f, Color.b / 255.f);


	int32 LastIndex = Indices.Num();
	for (int i = 0; i < FaceCount; i++)
	{
		//UVs.Append({{0.f, 0.f}, {1.f, 1.f}, {1.f, 0.f}, {0.f, 0.f}, {0.f, 1.f}, {1.f, 1.f}});

		for (int j = 0; j < 6; j++)
		{
			Indices.Add(i * 6 + j + LastIndex);
			VertexColors.Add(VertexColor);
			
		}
	}

}

void AChunk::GenerateMeshFromBlockData()
{

	MeshMutex.Lock();

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

	// Dispatch changes
	FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
	{
		ProceduralMesh->ClearMeshSection(0);
		ProceduralMesh->CreateMeshSection_LinearColor(0, Positions, Indices, Normals, UVs, VertexColors, Tangents, true);
		ProceduralMesh->SetCollisionConvexMeshes({Positions});

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
