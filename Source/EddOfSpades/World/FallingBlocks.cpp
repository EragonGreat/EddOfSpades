// Fill out your copyright notice in the Description page of Project Settings.

#include "FallingBlocks.h"
#include "ProceduralMeshComponent.h"
#include "GameConstants.h"
#include "InGame/IGGameState.h"

// Sets default values
AFallingBlocks::AFallingBlocks()
{
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel3);
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->bUseComplexAsSimpleCollision = false;

	// Get the block material for the procedural mesh, to color the blocks
	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Game/Materials/BlockMaterial.BlockMaterial'"));
	if(Material.Succeeded())
	{
		UMaterial* BlockMaterial = Material.Object;
		Mesh->SetMaterial(0, BlockMaterial);
	}

	// Make sure the spawning of the falling blocks is replicated, as well as it's movement
	SetReplicates(true);
	NetPriority = 1;
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.02f;

	RootComponent = Mesh;
}

void AFallingBlocks::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFallingBlocks, BlocksThatFell);
}

void AFallingBlocks::OnRep_BlocksThatFell()
{

	// Data about which blocks to fall has arrived, build the mesh
	BuildMesh();

}

void AFallingBlocks::MulticastUpdateTransform_Implementation(const FVector& Location, const FRotator Rotation)
{

	SetActorLocation(Location);
	SetActorRotation(Rotation);

}

void AFallingBlocks::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(10.f);

}

void AFallingBlocks::BuildMesh()
{

	// Data for the mesh
	TArray<FVector> Positions;
	TArray<int32> Indices;
	TArray<FVector2D> UVs;
	TArray<FVector> Normals;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// Get the game state for block color extraction
	AIGGameState* GameState = GetWorld()->GetGameState<AIGGameState>();

	// Populate data
	for(const FIntVector& Block : BlocksThatFell)
	{

		AddBlockMesh(GameState, Block, Positions, Indices, VertexColors);

	}

	Mesh->CreateMeshSection_LinearColor(0, Positions, Indices, Normals, UVs, VertexColors, Tangents, true);
	Mesh->SetCollisionConvexMeshes({Positions});

}

void AFallingBlocks::Tick(float DeltaSeconds)
{

	MulticastUpdateTransform(GetActorLocation(), GetActorRotation());

}

void AFallingBlocks::SetBlocksThatWillFall(const TArray<FIntVector>& Blocks)
{

	UE_LOG(LogTemp, Display, TEXT("Falling blocks spawned! Block count: %i"), Blocks.Num());

	// Save the data, it will then be replicated to clients
	BlocksThatFell = Blocks;

	// Create the mesh for the server
	BuildMesh();

	// If this is a server, then simulate physics. Do not do this on client, as it makes it look glitchy
	Mesh->SetSimulatePhysics(GetNetMode() < NM_Client);

}

void AFallingBlocks::AddBlockMesh(AIGGameState* GameState, const FIntVector& Block, TArray<FVector>& Positions, TArray<int32>& Indices, TArray<FLinearColor>& VertexColors)
{
	// Get the start index before positions are added, in order to prevent offset 
	int32 StartIndex = Positions.Num();

	// Add block vertex positions
	Positions.Add(FVector(Block.X, Block.Y, Block.Z) * GameConstants::BlockSize);
	Positions.Add(FVector(Block.X, (Block.Y + 1), Block.Z) * GameConstants::BlockSize);
	Positions.Add(FVector(Block.X, Block.Y, (Block.Z + 1))  * GameConstants::BlockSize);
	Positions.Add(FVector(Block.X, (Block.Y + 1), (Block.Z + 1)) * GameConstants::BlockSize);
	Positions.Add(FVector((Block.X + 1), Block.Y, Block.Z) * GameConstants::BlockSize);
	Positions.Add(FVector((Block.X + 1), (Block.Y + 1), Block.Z) * GameConstants::BlockSize);
	Positions.Add(FVector((Block.X + 1), Block.Y, (Block.Z + 1)) * GameConstants::BlockSize);
	Positions.Add(FVector((Block.X + 1), (Block.Y + 1), (Block.Z + 1)) * GameConstants::BlockSize);

	// Index the vertex positions
	Indices.Append({StartIndex + 7, StartIndex + 4, StartIndex + 6, StartIndex + 7, StartIndex + 5, StartIndex + 4,
					StartIndex + 2, StartIndex + 1, StartIndex + 3, StartIndex + 2, StartIndex + 0, StartIndex + 1,
					StartIndex + 3, StartIndex + 5, StartIndex + 7, StartIndex + 3, StartIndex + 1, StartIndex + 5,
					StartIndex + 6, StartIndex + 0, StartIndex + 2, StartIndex + 6, StartIndex + 4, StartIndex + 0,
					StartIndex + 6, StartIndex + 3, StartIndex + 7, StartIndex + 6, StartIndex + 2, StartIndex + 3,
					StartIndex + 1, StartIndex + 0, StartIndex + 4, StartIndex + 1, StartIndex + 4, StartIndex + 5,
					StartIndex + 7, StartIndex + 4, StartIndex + 6, StartIndex + 7, StartIndex + 5, StartIndex + 4,
					StartIndex + 2, StartIndex + 1, StartIndex + 3, StartIndex + 2, StartIndex + 0, StartIndex + 1,
					StartIndex + 3, StartIndex + 5, StartIndex + 7, StartIndex + 3, StartIndex + 1, StartIndex + 5,
					StartIndex + 6, StartIndex + 0, StartIndex + 2, StartIndex + 6, StartIndex + 4, StartIndex + 0,
					StartIndex + 6, StartIndex + 3, StartIndex + 7, StartIndex + 6, StartIndex + 2, StartIndex + 3,
					StartIndex + 1, StartIndex + 0, StartIndex + 4, StartIndex + 1, StartIndex + 4, StartIndex + 5});

	// Add the vertex color of the falling block
	const FBlockColor& BlockColor = GameState->GetBlockColorAt(Block);
	FLinearColor LinearColor(BlockColor.r / 255.f, BlockColor.g / 255.f, BlockColor.b / 255.f);
	for(int32 i = 0; i < 8; i++)
	{
		VertexColors.Add(LinearColor);
	}

}

