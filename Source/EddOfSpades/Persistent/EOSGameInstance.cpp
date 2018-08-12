// Fill out your copyright notice in the Description page of Project Settings.

#include "EOSGameInstance.h"

UEOSGameInstance::UEOSGameInstance()
	: CurrentSessionName(TEXT("NULL"))
{
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UEOSGameInstance::OnFindSessionsComplete);
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UEOSGameInstance::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UEOSGameInstance::OnStartSessionComplete);
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UEOSGameInstance::OnDestroySessionComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEOSGameInstance::OnJoinSessionComplete);
}

UEOSGameInstance::~UEOSGameInstance()
{
	DestroyServer();
}

void UEOSGameInstance::FindServers(bool bIsLAN)
{

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	TSharedPtr<const FUniqueNetId> UserID = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(0);

	// Clear old found sessions
	FoundSessions.Empty();

	if(OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if(Sessions.IsValid() && UserID.IsValid())
		{
			SessionSearch = MakeShareable(new FOnlineSessionSearch());

			SessionSearch->bIsLanQuery = bIsLAN;
			SessionSearch->MaxSearchResults = 100;
			SessionSearch->PingBucketSize = 50;

			TSharedRef<FOnlineSessionSearch> SessionSearchRef = SessionSearch.ToSharedRef();

			OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

			Sessions->FindSessions(*UserID, SessionSearchRef);

		}
	}
	else
	{
		OnFindSessionsComplete(false);
	}

}

bool UEOSGameInstance::HostServer(FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers)
{
	// Get the Online Subsystem to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if(OnlineSub)
	{
		TSharedPtr<const FUniqueNetId> UserID = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(0);

		// Get the Session Interface, so we can call the "CreateSession" function on it
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if(Sessions.IsValid() && UserID.IsValid())
		{
			// Fill in all the Session Settings
			SessionSettings = MakeShareable(new FOnlineSessionSettings());

			SessionSettings->bIsLANMatch = bIsLAN;
			SessionSettings->bUsesPresence = bIsPresence;
			SessionSettings->NumPublicConnections = MaxNumPlayers;
			SessionSettings->NumPrivateConnections = 0;
			SessionSettings->bAllowInvites = true;
			SessionSettings->bAllowJoinInProgress = true;
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->bAllowJoinViaPresence = true;
			SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

			// Add the server name to the settings
			SessionSettings->Set(FName(TEXT("SESSION_NAME")), SessionName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

			// Add the desired map to the settings
			SessionSettings->Set(SETTING_MAPNAME, FString("MainMap"), EOnlineDataAdvertisementType::ViaOnlineService);

			// Set the delegate to the Handle of the SessionInterface
			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

			// Our delegate should get called when this is complete (doesn't need to be successful!)
			return Sessions->CreateSession(*UserID, SessionName, *SessionSettings);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
	}

	return false;
}

bool UEOSGameInstance::JoinServer(FName SessionName)
{
	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if(OnlineSub)
	{

		TSharedPtr<const FUniqueNetId> UserID = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(0);

		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if(Sessions.IsValid() && UserID.IsValid())
		{
			// Set the Handle again
			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

			// Call the "JoinSession" Function with the passed "SearchResult". The "SessionSearch->SearchResults" can be used to get such a
			// "FOnlineSessionSearchResult" and pass it. Pretty straight forward!
			return Sessions->JoinSession(*UserID, SessionName, FoundSessions.FindRef(SessionName));
		}
	}

	return false;
}

void UEOSGameInstance::DestroyServer()
{
	if(!CurrentSessionName.IsEqual(FName(TEXT("NULL"))))
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if(OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

			if(Sessions.IsValid())
			{
				Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

				Sessions->DestroySession(CurrentSessionName);
			}
		}
	}
}

void UEOSGameInstance::GetAllFoundServerDescriptions(TArray<FOnlineSessionDescriptor>& OutDescriptions)
{

	for(const auto& FoundSession : FoundSessions)
	{

		FOnlineSessionSearchResult SearchResult = FoundSession.Value;

		// Extract information from the session result into a human readable struct
		FOnlineSessionDescriptor Description;
		Description.MaxPlayerCount = SearchResult.Session.SessionSettings.NumPrivateConnections + SearchResult.Session.SessionSettings.NumPublicConnections;
		Description.CurrentPlayerCount = Description.MaxPlayerCount - (SearchResult.Session.NumOpenPrivateConnections + SearchResult.Session.NumOpenPublicConnections);
		Description.ServerName = FoundSession.Key.ToString();
		Description.ServerGameMode = TEXT("Deathmatch");

		OutDescriptions.Add(Description);
	}

}

void UEOSGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnFindSessionsComplete bSuccess: %d"), bWasSuccessful));

	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		// Get SessionInterface of the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if(Sessions.IsValid())
		{
			// Clear the Delegate handle, since we finished this call
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
			
			// Just debugging the Number of Search results. Can be displayed in UMG or something later on
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num()));

			// If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG Widgets, like it is done in the BP version!
			if(SessionSearch->SearchResults.Num() > 0)
			{
				for(const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
				{
					FName SessionName(*Result.Session.SessionSettings.Settings.FindRef(TEXT("SESSION_NAME")).Data.ToString());

					FoundSessions.Add(SessionName, Result);

					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Session Name: %s "), *SessionName.ToString()));
				}

			}
		}
	}

	// Notify the bluperint child
	OnFindSessionsComplete_BP(bWasSuccessful);
}

void UEOSGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// Get the OnlineSubsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		// Get the Session Interface to call the StartSession function
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if(Sessions.IsValid())
		{
			// Clear the SessionComplete delegate handle, since we finished this call
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);

			if(bWasSuccessful)
			{
				CurrentSessionName = SessionName;

				// Set the StartSession delegate handle
				OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

				// Our StartSessionComplete delegate should get called after this
				Sessions->StartSession(SessionName);
			}
		}
	}

}

void UEOSGameInstance::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnStartSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// Get the Online Subsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		// Get the Session Interface to clear the Delegate
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if(Sessions.IsValid())
		{
			// Clear the delegate, since we are done with this call
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}

	// If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
	if(bWasSuccessful)
	{
		UGameplayStatics::OpenLevel(GetWorld(), "MainMap", true, "listen");
	}
}

void UEOSGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		// Get the SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if(Sessions.IsValid())
		{
			// Clear the Delegate
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

			// If it was successful, we just load another level (could be a MainMenu!)
			if(bWasSuccessful)
			{
				UGameplayStatics::OpenLevel(GetWorld(), "MainMap", true);
			}
		}
	}
}

void UEOSGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnJoinSessionComplete %s, %d"), *SessionName.ToString(), static_cast<int32>(Result)));

	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if(Sessions.IsValid())
		{
			// Clear the Delegate again
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

			// Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
			// This is something the Blueprint Node "Join Session" does automatically!
			APlayerController* const PlayerController = GetFirstLocalPlayerController();

			// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
			// String for us by giving him the SessionName and an empty String. We want to do this, because
			// Every OnlineSubsystem uses different TravelURLs
			FString TravelURL;

			if(PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL))
			{
				// Finally call the ClienTravel. If you want, you could print the TravelURL to see
				// how it really looks like
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}
