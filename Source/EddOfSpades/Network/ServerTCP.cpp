// Fill out your copyright notice in the Description page of Project Settings.

#include "ServerTCP.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "EddOfSpadesPlayerController.h"

UServerTCP::UServerTCP()
	: ClientPort(GameConstants::DefaultServerPort)
	, SocketThread(NULL)
{

}

UServerTCP::~UServerTCP()
{
	for (auto& Pair : ClientSockets)
	{

		if (!Pair.Value->Close())
		{
			UE_LOG(LogTemp, Display, TEXT("Failed to close server socket: %s"), *Pair.Value->GetDescription());
		}

	}

	ClientSockets.Empty();

	ClientsToConnectTo.Empty();

	PendingPackets.Empty();

	if (SocketThread)
	{
		SocketThread->Kill(true);
	}

}

void UServerTCP::StartServerTCP()
{

	if (!SocketThread)
	{

		// Starts the socket thread, will start in run() method
		SocketThread = FRunnableThread::Create(this, TEXT("Server Socket Thread"), 0, TPri_Lowest);

	}

}

bool UServerTCP::EstablishConnectionToClient(AEddOfSpadesPlayerController* OwningController)
{

	const FString& AddressString = OwningController->GetPlayerNetworkAddress();

	/// Convert the address string to an ipv4 address
	bool bIsAddressValid = false;
	TSharedRef<FInternetAddr> ClientAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	ClientAddress->SetIp(*AddressString, bIsAddressValid);
	ClientAddress->SetPort(GameConstants::DefaultClientPort);

	// Check the validity of the address, if invalid the return with an error message
	if (!bIsAddressValid)
	{
		// The address is invalid
		UE_LOG(LogTemp, Display, TEXT("Failed to connect TCP socket to client, invalid address format: %s"), *AddressString);
		return false;

	}

	/// The address is valid, start connecting to the client
	FSocket* ClientSocket = FTcpSocketBuilder(TEXT("Client socket")).BoundToPort(ClientPort + ClientSockets.Num()).AsBlocking().Build();

	if (ClientSocket->Connect(*ClientAddress))
	{
		// Successfully connected to client
		ClientSockets.Add(OwningController, ClientSocket);

		return true;
	}
	else
	{
		ClientSocket->Close();

		// Failed to connect to client
		return false;
	}


}

void UServerTCP::AddClientForConnection(AEddOfSpadesPlayerController* OwningController)
{

	// Add the controller to the connection queue, the other thread will automatically grab it
	ClientsToConnectTo.Enqueue(OwningController);

}

bool UServerTCP::SendDataTo(AEddOfSpadesPlayerController* OwningController, int32 DataSize, uint8* DataPtr)
{

	// Check that the controller is represented by a socket
	if (!ClientSockets.Contains(OwningController))
	{
		// It does not exist yet, return with error
		UE_LOG(LogTemp, Display, TEXT("A packet send was attempted without the controller being added to TCP socket system yet!"));
		return false;
	}

	// Prepare packet struct and add to queue, socket thread will take care of it automatically
	PendingPackets.Enqueue(FPendingData(DataSize, DataPtr, OwningController));

	return true;

}

void UServerTCP::Exit()
{

}

uint32 UServerTCP::Run()
{
	bStopping = false;

	while(!bStopping)
	{
		// Check if there are any new clients that needs to be connected to
		while(!ClientsToConnectTo.IsEmpty())
		{
			// A new client needs to be connected to
			AEddOfSpadesPlayerController* OwningController;
			
			// Get the new controller from the queue
			if (ClientsToConnectTo.Dequeue(OwningController))
			{

				EstablishConnectionToClient(OwningController);

			}

		}

		// Check if any new packets needs to be sent
		while(!PendingPackets.IsEmpty())
		{

			// There is a new packet ready, send it
			FPendingData PendingData;
			PendingPackets.Dequeue(PendingData);

			// Get the correct socket
			FSocket* ClientSocket = *ClientSockets.Find(PendingData.TargetPlayer);

			int32 BytesSent = 0;
			ClientSocket->Send(PendingData.DataPtr, PendingData.PacketSize, BytesSent);

			delete PendingData.DataPtr;

			UE_LOG(LogTemp, Display, TEXT("Server TCP sent a new packet, size: %i, bytes sent: %i"), PendingData.PacketSize, BytesSent);

		}

	}

	return 0;

}

void UServerTCP::Stop()
{
	// Tell the thread to stop next time it loops
	bStopping = true;
}

