// Fill out your copyright notice in the Description page of Project Settings.


#include "Plugins/MultiplaySessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Kismet/GameplayStatics.h"
#include "Structs/SessionInfomation.h"
#include "Interfaces/OnlineIdentityInterface.h"


UMultiplaySessionSubsystem::UMultiplaySessionSubsystem()
	:CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionCompleted))
	, UpdateSessionCompleteDelegate(FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateSessionCompleted))
	, StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionCompleted))
	, EndSessionCompleteDelegate(FOnEndSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnEndSessionCompleted))
	, DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionCompleted))
	, FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsCompleted))
	, JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionCompleted))
{
}

void UMultiplaySessionSubsystem::CreateSession(bool IsDedicated, bool IsLANMatch, FString SessionName, int32 NumPublicConnections, FString MatchMap, FString Password)
{
	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UMultiplaySessionSubsystem::CreateSession] WorldContext is not valid."));
		return;
	}

	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContext);
	if (!OnlineSubsystem)
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (SessionInterface)
	{
		SessionSettings = MakeShareable(new FOnlineSessionSettings());
		if (IsDedicated)
		{
			SessionSettings->bIsDedicated = true;
			SessionSettings->bAllowInvites = false;
			SessionSettings->bIsLANMatch = false;
			SessionSettings->NumPublicConnections = NumPublicConnections;
			SessionSettings->bUseLobbiesIfAvailable = false;
			SessionSettings->bUsesStats = true;
			SessionSettings->bUsesPresence = false;
			SessionSettings->bUseLobbiesVoiceChatIfAvailable = false;
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->bAllowJoinInProgress = true;
			SessionSettings->bAllowJoinViaPresence = false;
			SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
		}
		else
		{
			SessionSettings->bIsDedicated = false;
			SessionSettings->bAllowInvites = true;
			SessionSettings->bIsLANMatch = true;
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->NumPublicConnections = NumPublicConnections;
			SessionSettings->bUseLobbiesIfAvailable = false;
			SessionSettings->bUsesPresence = false;
		}

		Password.RemoveSpacesInline();

		UE_LOG(LogTemp, Warning, TEXT("UMultiplaySessionSubsystem - Password :: %s"), *Password);
		bool IsExistPassword = Password.Len() > 0 ? true : false;

		SessionSettings->Set(SEARCH_KEYWORDS, FString("FuryOfLegends"), EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(FName("SessionName"), SessionName, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(FName("MapName"), MatchMap, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(FName("IsExistPassword"), IsExistPassword, EOnlineDataAdvertisementType::ViaOnlineService);
		if (IsExistPassword) SessionSettings->Set(FName("Password"), Password, EOnlineDataAdvertisementType::ViaOnlineService);

		CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

		if (!SessionInterface->CreateSession(0, FName("MainSession"), *SessionSettings))
		{
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
			OnCreateSessionCompleteEvent.Broadcast(false);
		}
	}
}

void UMultiplaySessionSubsystem::OnCreateSessionCompleted(FName SessionName, bool Successful)
{
	// WorldContext 유효성 검사
	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] WorldContext is not valid. Unable to proceed with session completion."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// Session Interface 가져오기
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(WorldContext);
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] SessionInterface is null. Delegate handle not cleared."), ANSI_TO_TCHAR(__FUNCTION__));
	}

	// 성공 여부에 따라 처리
	if (Successful)
	{
		if (OnCreateSessionCompleteEvent.IsBound())
		{
			OnCreateSessionCompleteEvent.Broadcast(Successful);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create session '%s'."), ANSI_TO_TCHAR(__FUNCTION__), *SessionName.ToString());
	}
}


void UMultiplaySessionSubsystem::FindSessions(int32 MaxSearchResults, bool IsLANQuery)
{
	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UMultiplaySessionSubsystem::FindSessions] WorldContext is not valid."));
		return;
	}

	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContext);
	if (!OnlineSubsystem)
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<USessionInfomation*>(), false);
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<USessionInfomation*>(), false);
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.SearchParams.Empty();
	//SessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, true, EOnlineComparisonOp::Equals);
	//SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	SessionSearch->MaxSearchResults = 20;

	if (!SessionInterface->FindSessions(0, SessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		OnFindSessionsCompleteEvent.Broadcast(TArray<USessionInfomation*>(), false);
	}
}

void UMultiplaySessionSubsystem::OnFindSessionsCompleted(bool Successful)
{
	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UMultiplaySessionSubsystem::OnFindSessionsCompleted] WorldContext is not valid."));
		return;
	}

	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContext);
	if (!OnlineSubsystem)
	{
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		return;
	}

	if (SessionSearch->SearchResults.Num() <= 0)
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<USessionInfomation*>(), false);
		return;
	}

	int index = 0;
	bool IsExistPassword;
	FString SessionName;
	FString Password;
	FString MapName;

	TArray<USessionInfomation*> SessionSearchResults;
	for (auto& Result : SessionSearch->SearchResults)
	{
		USessionInfomation* SessionInfo = NewObject<USessionInfomation>(this, USessionInfomation::StaticClass());
		Result.Session.SessionSettings.Get(FName("IsExistPassword"), IsExistPassword);
		Result.Session.SessionSettings.Get(FName("SessionName"), SessionName);
		Result.Session.SessionSettings.Get(FName("Password"), Password);
		Result.Session.SessionSettings.Get(FName("MapName"), MapName);


		SessionInfo->SetIndex(index);
		SessionInfo->SetSessionName(SessionName);
		SessionInfo->SetMapName(MapName);
		SessionInfo->SetMaxPlayers(Result.Session.SessionSettings.NumPublicConnections);
		SessionInfo->SetCurrentPlayers(Result.Session.SessionSettings.NumPublicConnections - Result.Session.NumOpenPublicConnections);
		SessionInfo->SetPing(Result.PingInMs);
		SessionInfo->SetIsExistPassword(IsExistPassword);
		//SessionInfo->SetSessionResult(Result);

		SessionSearchResults.Add(SessionInfo);
		index++;
	}

	OnFindSessionsCompleteEvent.Broadcast(SessionSearchResults, true);
}

void UMultiplaySessionSubsystem::JoinSession(int Index, FString InputPassword)
{
	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UMultiplaySessionSubsystem::JoinSession] WorldContext is not valid."));
		return;
	}

	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContext);
	if (!OnlineSubsystem)
	{
		OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	bool IsExistPassword;
	if (SessionSearch->SearchResults.IsValidIndex(Index))
	{
		SessionSearch->SearchResults[Index].Session.SessionSettings.Get(FName("IsExistPassword"), IsExistPassword);
		if (IsExistPassword)
		{
			FString Password;
			InputPassword.RemoveSpacesInline();
			SessionSearch->SearchResults[Index].Session.SessionSettings.Get(FName("Password"), Password);
			if (InputPassword.Equals(Password))
			{
				JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

				if (!SessionInterface->JoinSession(0, FName("MainSession"), SessionSearch->SearchResults[Index]))
				{
					SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
					OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("JoinSession - Password is not matched!!!"));
				SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
				OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
			}
		}
		else
		{
			JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

			if (!SessionInterface->JoinSession(0, FName("MainSession"), SessionSearch->SearchResults[Index]))
			{
				SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
				OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
			}
		}
	}
}

void UMultiplaySessionSubsystem::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!GEngine)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GEngine is null. Cannot proceed with joining the session."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UWorld* WorldContext = GetWorld();
	if (!::IsValid(WorldContext))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] WorldContext is invalid. Unable to join session."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContext);
	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] OnlineSubsystem is null. Failed to retrieve subsystem for joining the session."), ANSI_TO_TCHAR(__FUNCTION__));
		OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] SessionInterface is invalid. Unable to proceed with session join."), ANSI_TO_TCHAR(__FUNCTION__));
		OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to join session. Result: %d"), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(Result));
		OnJoinSessionCompleteEvent.Broadcast(Result);
		return;
	}

	FString ConnectString = "127.0.0.1:17777";
	/*FString DedicatedServerJoinError;

	FURL DedicatedServerURL(nullptr, *ConnectString, TRAVEL_Absolute);
	auto DedicatedServerJoinStatus = GEngine->Browse(GEngine->GetWorldContextFromWorldChecked(WorldContext), DedicatedServerURL, DedicatedServerJoinError);
	if (DedicatedServerJoinStatus == EBrowseReturnVal::Failure)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to browse for the dedicated server. Error: %s"), ANSI_TO_TCHAR(__FUNCTION__), *DedicatedServerJoinError);
		OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}*/

	/*APlayerController* playerController = WorldContext->GetFirstPlayerController();
	if (playerController)
	{
		playerController->ClientTravel(ConnectString, TRAVEL_Absolute);
	}*/

	
	//UGameplayStatics::OpenLevel(GetWorld(), TEXT("Loading"), true, FString::Printf(TEXT("NextLevel=%s"), *ConnectString));

	TryClientTravelToCurrentSession();

	UE_LOG(LogTemp, Log, TEXT("[%s] Successfully connected to the session at %s"), ANSI_TO_TCHAR(__FUNCTION__), *ConnectString);

	// Clear the session delegate after successful handling
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
}


void UMultiplaySessionSubsystem::UpdateSession(FString MapName)
{

}

void UMultiplaySessionSubsystem::OnUpdateSessionCompleted(FName SessionName, bool Successful)
{

}

void UMultiplaySessionSubsystem::StartSession()
{

}

void UMultiplaySessionSubsystem::OnStartSessionCompleted(FName SessionName, bool Successful)
{

}

void UMultiplaySessionSubsystem::EndSession()
{

}

void UMultiplaySessionSubsystem::OnEndSessionCompleted(FName SessionName, bool Successful)
{

}

void UMultiplaySessionSubsystem::DestroySession()
{
	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UMultiplaySessionSubsystem::DestroySession] WorldContext is not valid."));
		return;
	}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContext);
	if (!OnlineSubsystem)
	{
		OnDestroySessionCompleteEvent.Broadcast(false);
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		OnDestroySessionCompleteEvent.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(FName("MainSession")))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		OnDestroySessionCompleteEvent.Broadcast(false);
	}
}

void UMultiplaySessionSubsystem::RegisterPlayer(APlayerController* NewPlayer, bool bWasFromInvite)
{
	if (::IsValid(NewPlayer) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] NewPlayer is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	FUniqueNetIdRepl UniqueNetIDRepl;
	if (NewPlayer->IsLocalPlayerController())
	{
		// 로컬 플레이어 컨트롤러 처리
		ULocalPlayer* LocalPlayer = NewPlayer->GetLocalPlayer();
		if (::IsValid(LocalPlayer))
		{
			UniqueNetIDRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] LocalPlayer is not valid. Using fallback ID."), ANSI_TO_TCHAR(__FUNCTION__));
		}
	}
	else
	{
		// 원격 네트워크 연결 처리
		UNetConnection* RemoteNetConnectionRef = Cast<UNetConnection>(NewPlayer->Player);
		if (::IsValid(RemoteNetConnectionRef) && RemoteNetConnectionRef->PlayerId.IsValid())
		{
			UniqueNetIDRepl = RemoteNetConnectionRef->PlayerId;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] RemoteNetConnectionRef is not valid or PlayerId is null."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}
	}

	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIDRepl.GetUniqueNetId();
	if (UniqueNetId.IsValid() == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] UniqueNetId is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	const IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(NewPlayer->GetWorld());
	if (!SubsystemRef)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] OnlineSubsystem is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	IOnlineSessionPtr SessionRef = SubsystemRef->GetSessionInterface();
	if (SessionRef.IsValid() == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] SessionInterface is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// 플레이어 등록
	bool bRegistrationSuccess = SessionRef->RegisterPlayer(FName("MainSession"), *UniqueNetId, bWasFromInvite);
	if (bRegistrationSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Player registration successful!"), ANSI_TO_TCHAR(__FUNCTION__));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Player registration failed!"), ANSI_TO_TCHAR(__FUNCTION__));
	}
}


void UMultiplaySessionSubsystem::OnDestroySessionCompleted(FName SessionName, bool Successful)
{

}

bool UMultiplaySessionSubsystem::TryServerTravel(FString OpenLevelName)
{
	UWorld* WorldContext = GetWorld();
	if (!::IsValid(WorldContext))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] WorldContext is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return false;
	}

	if (OpenLevelName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] OpenLevelName is empty. Cannot travel."), ANSI_TO_TCHAR(__FUNCTION__));
		return false;
	}

	WorldContext->ServerTravel(*OpenLevelName, false, false);
	return true;
}


bool UMultiplaySessionSubsystem::TryClientTravelToCurrentSession()
{
	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UMultiplaySessionSubsystem::TryClientTravelToCurrentSession] WorldContext is not valid."));
		return false;
	}

	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(WorldContext);
	if (!sessionInterface.IsValid())
	{
		return false;
	}

	FString ConnectString;
	if (!sessionInterface->GetResolvedConnectString(FName("MainSession"), ConnectString))
	{
		return false;
	}

	APlayerController* playerController = WorldContext->GetFirstPlayerController();
	if (!playerController)
	{
		return false;
	}

	//playerController->ClientTravel(connectString, TRAVEL_Absolute);
	
	UE_LOG(LogTemp, Log, TEXT("[%s] %s Client attempting to travel to map: %s"), ANSI_TO_TCHAR(__FUNCTION__), *playerController->GetName(), *ConnectString);
	UGameplayStatics::OpenLevel(WorldContext, TEXT("Loading"), true, FString::Printf(TEXT("NextLevel=%s"), *ConnectString));

	return true;
}

void UMultiplaySessionSubsystem::LoginWithEOS(const FString& ID, const FString& Token, const FString& LoginType)
{
	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UMultiplaySessionSubsystem::JoinSession] WorldContext is not valid."));
		return;
	}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContext);
	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr IdentityPointerRef = OnlineSubsystem->GetIdentityInterface();
		if (IdentityPointerRef)
		{
			FOnlineAccountCredentials AccountDetails;
			AccountDetails.Id = ID;
			AccountDetails.Token = Token;
			AccountDetails.Type = LoginType;

			IdentityPointerRef->OnLoginCompleteDelegates->AddUObject(this, &UMultiplaySessionSubsystem::OnLoginCompleted);
			IdentityPointerRef->Login(0, AccountDetails);
		}
	}
}

FString UMultiplaySessionSubsystem::GetPlayerName()
{
	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UMultiplaySessionSubsystem::GetPlayerName] WorldContext is not valid."));
		return FString();
	}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContext);
	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr IdentityPointerRef = OnlineSubsystem->GetIdentityInterface();
		if (IdentityPointerRef)
		{
			if (IdentityPointerRef->GetLoginStatus(0) == ELoginStatus::LoggedIn)
			{
				return IdentityPointerRef->GetPlayerNickname(0);
			}
		}
	}
	return FString();
}

bool UMultiplaySessionSubsystem::IsPlayerLoggedIn() const
{
	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UMultiplaySessionSubsystem::IsPlayerLoggedIn] WorldContext is not valid."));
		return false;
	}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContext);
	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr IdentityPointerRef = OnlineSubsystem->GetIdentityInterface();
		if (IdentityPointerRef)
		{
			if (IdentityPointerRef->GetLoginStatus(0) == ELoginStatus::LoggedIn)
			{
				return true;
			}
		}
	}
	return false;
}

void UMultiplaySessionSubsystem::OnLoginCompleted(int32 LocalUserNum, bool bSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Login Success"));
		OnLoginCompleteEvent.Broadcast(true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Login Fail :: %s"), *Error);
		OnLoginCompleteEvent.Broadcast(false);
	}
}

void UMultiplaySessionSubsystem::CreateEOSSession(bool bIsDedicatedServer, bool bIsLanServer, int32 NumberOfPublicConnections)
{

}
