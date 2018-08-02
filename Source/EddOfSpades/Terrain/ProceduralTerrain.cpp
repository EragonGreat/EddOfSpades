// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralTerrain.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

#include "ConstructorHelpers.h"

#include "FastNoise/FastNoise.h"


// Sets default values
AProceduralTerrain::AProceduralTerrain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialObject(TEXT("Material'/Game/Materials/BlockMaterial.BlockMaterial'"));

	if (MaterialObject.Succeeded())
	{
		UMaterial* TerrainMaterial = MaterialObject.Object;
		TerrainMesh->SetMaterial(0, TerrainMaterial);
	}

	RootComponent = TerrainMesh;

	GenerateTerrainMesh(100, 100.f);
}

// Called when the game starts or when spawned
void AProceduralTerrain::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProceduralTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralTerrain::GenerateTerrainMesh(int32 Quads, float QuadSize)
{
	this->Quads = Quads;
	this->QuadSize = QuadSize;
	this->VertexCount = Quads + 1;

	TArray<FVector> Vertices;
	TArray<int32> Indices;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	FastNoise Noise(0);

	Noise.SetFractalOctaves(6);

	// Generate vertices
	for (int32 i = 0; i < VertexCount; i++)
	{
		for (int32 j = 0; j < VertexCount; j++)
		{

			float Height = Noise.GetSimplexFractal(i, j);

			FVector Vertex(i * QuadSize, j * QuadSize, Height * 1000.f);
			Vertices.Push(Vertex);

		}
	}

	// Connect vertices with indices
	for (int32 i = 0; i < Quads; i++)
	{
		for (int32 j = 0; j < Quads; j++)
		{

			Indices.Push(j * VertexCount + i);
			Indices.Push(j * VertexCount + (i + 1));
			Indices.Push((j + 1) * VertexCount + i);

			Indices.Push(j * VertexCount + (i + 1));
			Indices.Push((j + 1) * VertexCount + (i + 1));
			Indices.Push((j + 1) * VertexCount + i);

		}
	}

	// Calculate UVs
	for (int32 i = 0; i < VertexCount; i++)
	{
		for (int32 j = 0; j < VertexCount; j++)
		{

			FVector2D UV((float)i, (float)j);
			UVs.Push(UV);

		}
	}

	// Calculate Normals
	for (int32 i = 0; i < VertexCount; i++)
	{
		for (int32 j = 0; j < VertexCount; j++)
		{

			if (i > 0 && j > 0 && i < VertexCount - 1 && j < VertexCount - 1)
			{
				float Current = Vertices[ToIndex(i, j)].Z;

				FVector Left = Vertices[ToIndex(i - 1, j)];
				FVector Right = Vertices[ToIndex(i + 1, j)];
				FVector Up = Vertices[ToIndex(i, j - 1)];
				FVector Down = Vertices[ToIndex(i, j + 1)];

				FVector CL = Left - Current;
				FVector CR = Right - Current;
				FVector CU = Up - Current;
				FVector CD = Down - Current;

				FVector CUxCL = FVector::CrossProduct(CU, CL);
				FVector CUxCR = FVector::CrossProduct(CR, CU);
				FVector CDxCL = FVector::CrossProduct(CL, CD);
				FVector CDxCR = FVector::CrossProduct(CD, CR);

				/*float Left = Vertices[ToIndex(i - 1, j)].Z;
				float Right = Vertices[ToIndex(i + 1, j)].Z;
				float Up = Vertices[ToIndex(i, j - 1)].Z;
				float Down = Vertices[ToIndex(i, j + 1)].Z;

				FVector UPxLP(Current - Up, Current - Up, -1);
				FVector UPxPR(Right - Current, Current - Up, -1);
				FVector PDxLP(Current - Left, Right - Current, -1);
				FVector PDxPR(Right - Current, Down - Current, -1);*/

				FVector Normal = CUxCL + CUxCR + CDxCL + CDxCR;
				Normal.Normalize();

				Normals.Push(Normal);
			}
			else
			{

				Normals.Push(FVector(0.f, 0.f, 1.f));

			}

		}
	}

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Indices, UVs, Normals, Tangents);
	TerrainMesh->CreateMeshSection_LinearColor(0, Vertices, Indices, Normals, UVs, VertexColors, Tangents, true);
	
}

int32 AProceduralTerrain::ToIndex(int32 x, int32 y) const
{
	return y * VertexCount + x;
}

