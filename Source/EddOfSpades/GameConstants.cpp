// Fill out your copyright notice in the Description page of Project Settings.

#include "GameConstants.h"

const int64 GameConstants::ChunkSize = 8;

const int64 GameConstants::WorldChunkCount = 32;

const int64 GameConstants::WorldHeight = 128;

const float GameConstants::BlockSize = 100.f;

const int64 GameConstants::WorldBlockCount = GameConstants::WorldChunkCount * GameConstants::ChunkSize;

const int64 GameConstants::TotalChunks = GameConstants::WorldChunkCount * GameConstants::WorldChunkCount;

const int64 GameConstants::TotalBlocks = GameConstants::WorldBlockCount * GameConstants::WorldBlockCount * GameConstants::WorldHeight;

const int32 GameConstants::DefaultServerPort = 5555;

const int32 GameConstants::DefaultClientPort = 5554;