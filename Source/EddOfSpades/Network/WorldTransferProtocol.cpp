// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldTransferProtocol.h"
#include "ClientTCP.h"
#include "ServerTCP.h"
#include "EddOfSpadesGameState.h"
#include "EddOfSpadesHUD.h"
#include "EddOfSpadesPlayerController.h"


UWorldTransferProtocol::~UWorldTransferProtocol()
{
	
	if(TransferThread && !bStopping)
	{
		bStopping = true;
		TransferThread->Kill(true);
	}

}

void UWorldTransferProtocol::SendWorldToClient(UServerTCP* ServerTCP, AEddOfSpadesGameState* GameState, AEddOfSpadesPlayerController* TargetPlayer)
{
	this->ServerTCP = ServerTCP;
	this->TargetPlayer = TargetPlayer;
	this->GameState = GameState;

	bIsReceiving = false;

	StartTransfering();
}

void UWorldTransferProtocol::ReceiveWorldFromServer(UClientTCP* ClientTCP, AEddOfSpadesGameState* GameState, AEddOfSpadesPlayerController* ClientController)
{
	this->ClientTCP = ClientTCP;
	this->TargetPlayer = ClientController;
	this->GameState = GameState;
	
	bIsReceiving = true;
	
	StartTransfering();
}

uint32 UWorldTransferProtocol::Run()
{
	ChunksHandled = 0;
	int32 ProgressID = 0;

	if(bIsReceiving)
	{
		// We are receiving the world
		if(HUD)
		{
			ProgressID = HUD->InsertNewProgressItem(TEXT("Receiving World"));
		}

		while(!bStopping && ChunksHandled != GameConstants::WorldChunkCount * GameConstants::WorldChunkCount)
		{

			if(ClientTCP->HasPendingPacket())
			{
				FReceivedPacket Packet = ClientTCP->GetPendingPacket();

				// We have received a whole chunk
				int32 ChunkX = ReadInteger(Packet.DataPtr.Get(), 0);
				int32 ChunkY = ReadInteger(Packet.DataPtr.Get(), 4);

				// Get a pointer to the first block of the array
				FBlockData* FirstBlock = (FBlockData*)(Packet.DataPtr.Get() + 8);

				// Use that pointer to append into array
				FChunkData Chunk;
				Chunk.BlockData.Append(FirstBlock, GameConstants::ChunkSize * GameConstants::ChunkSize * GameConstants::WorldHeight);

				TargetPlayer->ClientReceiveNewChunk(Chunk, ChunkX, ChunkY);

				// Update progress
				ChunksHandled++;

				if(HUD)
				{
					HUD->UpdateProgressItem(ProgressID, ChunksHandled / GameConstants::WorldChunkCount * GameConstants::WorldChunkCount);
				}
			}
		}

		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
		{

			OnWorldReceived.Broadcast();

		}, TStatId(), NULL, ENamedThreads::GameThread);

	}
	else 
	{
		// We are sending the world
		if(HUD)
		{
			ProgressID = HUD->InsertNewProgressItem(TEXT("Sending World to Client"));
		}

		// This is the size of a chunk in bytes
		int32 ChunkByteSize = GameConstants::ChunkSize * GameConstants::ChunkSize * GameConstants::WorldHeight * sizeof(FBlockData) + 2 * sizeof(int32);

		// Loop through all chunks
		for(int32 x = 0; x < GameConstants::WorldChunkCount; x++)
		{
			for(int32 y = 0; y < GameConstants::WorldChunkCount; y++)
			{
				uint8* RawChunkData = new uint8[ChunkByteSize];

				// Get the chunk
				const FChunkData& Chunk = GameState->GetChunk(x, y);

				// TODO: Compress data to save bandwidth
				// Insert the chunks position
				WriteInteger(RawChunkData, x, 0);
				WriteInteger(RawChunkData, y, 4);

				// Insert the chunk into the raw byte array
				int32 i = 0;
				for(const FBlockData& Block : Chunk.BlockData)
				{
					RawChunkData[2 * sizeof(int32) + i * sizeof(FBlockData)] = static_cast<uint8>(Block.bIsAir);
					RawChunkData[2 * sizeof(int32) + i * sizeof(FBlockData) + 1] = Block.Color.r;
					RawChunkData[2 * sizeof(int32) + i * sizeof(FBlockData) + 2] = Block.Color.g;
					RawChunkData[2 * sizeof(int32) + i * sizeof(FBlockData) + 3] = Block.Color.b;

					i++;
				}

				ServerTCP->SendDataTo(TargetPlayer, ChunkByteSize, RawChunkData);

				ChunksHandled++;

				if(HUD)
				{
					HUD->UpdateProgressItem(ProgressID, ChunksHandled / GameConstants::WorldChunkCount * GameConstants::WorldChunkCount);
				}
			}
		}
	}

	if(HUD)
	{
		HUD->RemoveProgressItem(ProgressID);
	}

	bStopping = true;

	return 0;

}

void UWorldTransferProtocol::StartTransfering()
{
	
	if(TargetPlayer)
	{
		HUD = Cast<AEddOfSpadesHUD>(TargetPlayer->GetHUD());
	}

	bStopping = false;
	TransferThread = FRunnableThread::Create(this, TEXT("World Transfer Thread"));

}

#pragma optimize( "", off )
int32 UWorldTransferProtocol::ReadInteger(uint8* Data, int32 Offset)
{

	// Convert 4 bytes into an unsigned integer, unsigned because it prevents signed bit issues when shifting
	uint32 Integer = 0;
	Integer |= Data[Offset];
	Integer <<= 8;
	Integer |= Data[Offset + 1];
	Integer <<= 8;
	Integer |= Data[Offset + 2];
	Integer <<= 8;
	Integer |= Data[Offset + 3];

	return static_cast<int32>(Integer);

}

void UWorldTransferProtocol::WriteInteger(uint8* Data, int32 Integer, int32 Offset)
{

	// Write the 4 individual bytes of the integers into the array at the specified offset
	for(int i = 0; i < 4; i++)
	{
		Data[i + Offset] = (Integer << (8 * (i + 1)));
	}

}
#pragma optimize( "", on )