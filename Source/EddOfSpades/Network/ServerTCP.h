// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Networking.h"	
#include "UObject/NoExportTypes.h"
#include "ServerTCP.generated.h"

USTRUCT()
struct EDDOFSPADES_API FPendingData
{
	GENERATED_BODY()

public:
	FPendingData()
	{}

	FPendingData(int32 PacketSize, uint8* DataPtr, class AIGPlayerController* TargetPlayer)
		: PacketSize(PacketSize)
		, DataPtr(DataPtr)
		, TargetPlayer(TargetPlayer)
	{}

	int32 PacketSize;

	uint8* DataPtr;

	UPROPERTY()
	class AIGPlayerController* TargetPlayer;

};

/**
 * 
 */
UCLASS()
class EDDOFSPADES_API UServerTCP : public UObject, public FRunnable
{
	GENERATED_BODY()
	
private:
	TMap<class AIGPlayerController*, FSocket*> ClientSockets;
	TQueue<class AIGPlayerController*, EQueueMode::Spsc> ClientsToConnectTo;
	TQueue<FPendingData, EQueueMode::Spsc> PendingPackets;

	int32 ClientPort;

	bool bStopping;
	FRunnableThread* SocketThread;

public:
	UServerTCP();
	~UServerTCP();

	void StartServerTCP();
	void AddClientForConnection(class AIGPlayerController* OwningController);

	bool SendDataTo(class AIGPlayerController* OwningController, int32 DataSize, uint8* DataPtr);

	virtual void Exit() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

private:
	bool EstablishConnectionToClient(class AIGPlayerController* OwningController);

};
