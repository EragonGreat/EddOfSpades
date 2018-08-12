// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EddOfSpades.h"
#include "UnrealNetwork.h"
#include "Online.h"
#include "EOSGameInstance.generated.h"

USTRUCT(BlueprintType)
struct EDDOFSPADES_API FOnlineSessionDescriptor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString ServerName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString ServerGameMode;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 CurrentPlayerCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 MaxPlayerCount;
	
};

/**
 * 
 */
UCLASS()
class EDDOFSPADES_API UEOSGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	TMap<FName, FOnlineSessionSearchResult> FoundSessions;
public:
	FName CurrentSessionName;

	/* Delegate called when sessions found */
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	/* Delegate called when session created */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	/* Delegate called when session started */
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	/* Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	/** Delegate for joining a session */
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

	/* Handles to registered delegates */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

public:
	UEOSGameInstance();
	~UEOSGameInstance();

	UFUNCTION(BlueprintCallable)
	void FindServers(bool bIsLAN);

	UFUNCTION(BlueprintCallable)
	bool HostServer(FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);

	UFUNCTION(BlueprintCallable)
	bool JoinServer(FName SessionName);

	UFUNCTION(BlueprintCallable)
	void DestroyServer();

	UFUNCTION(BlueprintCallable)
	void GetAllFoundServerDescriptions(TArray<FOnlineSessionDescriptor>& OutDescriptions);

	void OnFindSessionsComplete(bool bWasSuccessful);

	UFUNCTION(BlueprintImplementableEvent)
	void OnFindSessionsComplete_BP(bool bWasSuccessful);

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
