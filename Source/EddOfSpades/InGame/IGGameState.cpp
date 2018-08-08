// Fill out your copyright notice in the Description page of Project Settings.
#include "IGGameState.h"
#include "UnrealNetwork.h"

void AIGGameState::BeginPlay()
{
	// Prepare world data array
	Chunks.SetNum(GameConstants::WorldChunkCount * GameConstants::WorldChunkCount);

	for (FChunkData& Chunk : Chunks)
	{
		Chunk.BlockData.SetNum(GameConstants::ChunkSize * GameConstants::ChunkSize * GameConstants::WorldHeight);
	}
}

const FBlockColor& AIGGameState::GetBlockColorAt(int32 x, int32 y, int32 z) const
{
	const TArray<FBlockData>& Chunk = GetChunk(x / GameConstants::ChunkSize, y / GameConstants::ChunkSize).BlockData;

	int32 ChunkX = x % GameConstants::ChunkSize;
	int32 ChunkY = y % GameConstants::ChunkSize;

	return Chunk[ChunkX + GameConstants::ChunkSize * (ChunkY + GameConstants::ChunkSize * z)].Color;
}

const FBlockColor& AIGGameState::GetBlockColorAt(const FIntVector& Pos) const
{
	return GetBlockColorAt(Pos.X, Pos.Y, Pos.Z);
}

void AIGGameState::SetBlockColorAt(int32 x, int32 y, int32 z, const FBlockColor& NewColor)
{
	TArray<FBlockData>& Chunk = GetEditableChunk(x / GameConstants::ChunkSize, y / GameConstants::ChunkSize).BlockData;

	int32 ChunkX = x % GameConstants::ChunkSize;
	int32 ChunkY = y % GameConstants::ChunkSize;

	Chunk[ChunkX + GameConstants::ChunkSize * (ChunkY + GameConstants::ChunkSize * z)].Color = NewColor;
}

void AIGGameState::SetBlockColorAt(const FIntVector& Pos, const FBlockColor& NewColor)
{
	SetBlockColorAt(Pos.X, Pos.Y, Pos.Z, NewColor);
}

bool AIGGameState::IsBlockAirAt(int32 x, int32 y, int32 z) const
{
	const TArray<FBlockData>& Chunk = GetChunk(x / GameConstants::ChunkSize, y / GameConstants::ChunkSize).BlockData;

	int32 ChunkX = x % GameConstants::ChunkSize;
	int32 ChunkY = y % GameConstants::ChunkSize;

	return Chunk[ChunkX + GameConstants::ChunkSize * (ChunkY + GameConstants::ChunkSize * z)].bIsAir;
}

bool AIGGameState::IsBlockAirAt(const FIntVector& Pos) const
{
	return IsBlockAirAt(Pos.X, Pos.Y, Pos.Z);
}

void AIGGameState::SetBlockIsAirAt(int32 x, int32 y, int32 z, bool bIsAir)
{
	TArray<FBlockData>& Chunk = GetEditableChunk(x / GameConstants::ChunkSize, y / GameConstants::ChunkSize).BlockData;

	int32 ChunkX = x % GameConstants::ChunkSize;
	int32 ChunkY = y % GameConstants::ChunkSize;

	Chunk[ChunkX + GameConstants::ChunkSize * (ChunkY + GameConstants::ChunkSize * z)].bIsAir = bIsAir;
}

void AIGGameState::SetBlockIsAirAt(const FIntVector& Pos, bool bIsAir)
{
	SetBlockIsAirAt(Pos.X, Pos.Y, Pos.Z, bIsAir);
}

const FBlockData& AIGGameState::GetBlockAt(int32 x, int32 y, int32 z) const
{

	const TArray<FBlockData>& Chunk = GetChunk(x / GameConstants::ChunkSize, y / GameConstants::ChunkSize).BlockData;

	int32 ChunkX = x % GameConstants::ChunkSize;
	int32 ChunkY = y % GameConstants::ChunkSize;

	return Chunk[ChunkX + GameConstants::ChunkSize * (ChunkY + GameConstants::ChunkSize * z)];

}

const FBlockData& AIGGameState::GetBlockAt(const FIntVector& Pos) const
{
	return GetBlockAt(Pos.X, Pos.Y, Pos.Z);
}

void AIGGameState::SetBlockAt(int32 x, int32 y, int32 z, const FBlockData& NewBlock)
{
	TArray<FBlockData>& Chunk = GetEditableChunk(x / GameConstants::ChunkSize, y / GameConstants::ChunkSize).BlockData;

	int32 ChunkX = x % GameConstants::ChunkSize;
	int32 ChunkY = y % GameConstants::ChunkSize;

	Chunk[ChunkX + GameConstants::ChunkSize * (ChunkY + GameConstants::ChunkSize * z)] = NewBlock;
}

void AIGGameState::SetBlockAt(const FIntVector& Pos, const FBlockData& NewBlock)
{
	SetBlockAt(Pos.X, Pos.Y, Pos.Z, NewBlock);
}

void AIGGameState::SetChunk(const FChunkData& NewChunk, int32 ChunkX, int32 ChunkY)
{
	Chunks[ChunkY * GameConstants::WorldChunkCount + ChunkX] = NewChunk;
}

const FChunkData& AIGGameState::GetChunk(int32 ChunkX, int32 ChunkY) const
{
	return Chunks[ChunkY * GameConstants::WorldChunkCount + ChunkX];
}

FChunkData& AIGGameState::GetEditableChunk(int32 ChunkX, int32 ChunkY)
{
	return Chunks[ChunkY * GameConstants::WorldChunkCount + ChunkX];
}

const TArray<FChunkData>& AIGGameState::GetAllChunks() const
{
	return Chunks;
}

