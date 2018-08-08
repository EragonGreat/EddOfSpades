// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "UObject/NoExportTypes.h"
#include "ClientTCP.generated.h"

USTRUCT()
struct EDDOFSPADES_API FReceivedPacket
{
	GENERATED_BODY()

public:
	int32 PacketSize;
	TSharedPtr<uint8> DataPtr;

};

/**
 * 
 */
UCLASS()
class EDDOFSPADES_API UClientTCP : public UObject, public FRunnable
{
	GENERATED_BODY()
	
private:
	class AIGPlayerController* OwningController;

	FSocket* ClientSocket;
	FSocket* ConnectionSocket;

	TQueue<FReceivedPacket, EQueueMode::Spsc> PendingPackets;

	bool bStopping;
	FRunnableThread* SocketThread;

	int32 FirstServerPort;

public:
	UClientTCP();
	~UClientTCP();

	bool WaitForServerConnection(int32 GamePort);
	
	void StartTCPSocket(class AIGPlayerController* OwningController);

	bool HasPendingPacket() const;

	FReceivedPacket GetPendingPacket();

	virtual void Exit() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

};
