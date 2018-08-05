// Fill out your copyright notice in the Description page of Project Settings.

#include "ClientTCP.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "GameConstants.h"
#include "EddOfSpadesPlayerController.h"
#include "EddOfSpadesGameState.h"

UClientTCP::UClientTCP()
	: ClientSocket(NULL)
	, ConnectionSocket(NULL)
{

}

UClientTCP::~UClientTCP()
{

	if (ClientSocket)
	{
		if (!ClientSocket->Close())
		{
			UE_LOG(LogTemp, Display, TEXT("Failed to close the client socket"));
		}
	}

	if (ConnectionSocket)
	{
		if (!ConnectionSocket->Close())
		{
			UE_LOG(LogTemp, Display, TEXT("Failed to close the client socket"));
		}
	}

	if (SocketThread)
	{
		SocketThread->Kill(true);
	}
}

bool UClientTCP::WaitForServerConnection(int32 GamePort)
{
	this->FirstServerPort = GamePort;

	// Create client socket
	ConnectionSocket = FTcpSocketBuilder(TEXT("Client Connection Socket")).BoundToPort(GamePort).AsBlocking().Listening(1).Build();

	ClientSocket = ConnectionSocket->Accept(TEXT("Client Socket"));

	ConnectionSocket->Close();
	ConnectionSocket = NULL;

	if (ClientSocket)
	{
		// Connection to server was established

		return true;
	}
	else
	{
		// Connection failed to establish with server

		return false;
	}

}

void UClientTCP::StartTCPSocket(class AEddOfSpadesPlayerController* OwningController)
{
	// Set the owner
	this->OwningController = OwningController;

	// Create the thread for which to listen on
	if (!SocketThread)
	{
		SocketThread = FRunnableThread::Create(this, TEXT("Client Socket Thread"), 0, TPri_Lowest);
	}

	// Log an error if failed
	if (!SocketThread)
	{
		UE_LOG(LogTemp, Display, TEXT("Failed to create client socket thread"));
	}
}

bool UClientTCP::HasPendingPacket() const
{
	return !PendingPackets.IsEmpty();
}

FReceivedPacket UClientTCP::GetPendingPacket()
{
	FReceivedPacket Packet;
	PendingPackets.Dequeue(Packet);
	return Packet;
}

void UClientTCP::Exit()
{

}

uint32 UClientTCP::Run()
{
	bStopping = false;

	// First establish connection with server
	WaitForServerConnection(GameConstants::DefaultClientPort);

	while (!bStopping)
	{
		// Listen for incoming packets
		FReceivedPacket Packet;
		Packet.PacketSize = GameConstants::ChunkSize * GameConstants::ChunkSize * GameConstants::WorldHeight * sizeof(FBlockData) + 2 * sizeof(int32);
		Packet.DataPtr = TSharedPtr<uint8>(new uint8[Packet.PacketSize]);
		
		int32 BytesRead = 0;
		if (ClientSocket->Recv(Packet.DataPtr.Get(), Packet.PacketSize, BytesRead, ESocketReceiveFlags::WaitAll))
		{
			// In my case, the only use for this is world transfering which is why i don't need any identifier packets
			// The data received needs to be the same size as the buffer size
			if (BytesRead == Packet.PacketSize)
			{
				// We have received a whole packet, add it to the pending packets queue
				UE_LOG(LogTemp, Display, TEXT("A packet was received on the client, size: %i"), BytesRead);

				PendingPackets.Enqueue(Packet);
			}
			else
			{
				// We have received an incomplete chunk
				UE_LOG(LogTemp, Display, TEXT("Client received an incomplete packet! Desired size: %i, Actual size: %i"), Packet.PacketSize, BytesRead);
			}


		}

	}

	return 0;

}

void UClientTCP::Stop()
{
	// Tell the thread to stop next time it loops
	bStopping = true;
}

