// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Structs/SessionInfomation.h"
#include "MultiplaySessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnCreateSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnUpdateSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnStartSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnEndSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnDestroySessionComplete, bool, Successful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCSOnFindSessionsComplete, const TArray<USessionInfomation*>& SessionResults, bool Successful);
DECLARE_MULTICAST_DELEGATE_OneParam(FCSOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_MULTICAST_DELEGATE_OneParam(FCOnLoginCompleteDelegate, bool Successful);

UCLASS()
class FURYOFLEGENDS_API UMultiplaySessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplaySessionSubsystem();

	void CreateSession(bool IsDedicated = false, bool IsLANMatch = false, FString SessionName = "", int32 NumPublicConnections = 10, FString MatchMap = "Rift", FString Password = "");

	void FindSessions(int32 MaxSearchResults, bool IsLANQuery);

	void JoinSession(int Index, FString Password);

	void UpdateSession(FString MatchType);

	void StartSession();

	void EndSession();

	void DestroySession();

	void RegisterPlayer(APlayerController* NewPlayer, bool bWasFromInvite);

	bool TryServerTravel(FString OpenLevelName);

public:
	UFUNCTION(BlueprintCallable, Category = "EOS_Functions")
	void LoginWithEOS(const FString& ID, const FString& Token, const FString& LoginType);

	UFUNCTION(BlueprintCallable, Category = "EOS_Functions")
	FString GetPlayerName();

	UFUNCTION(BlueprintCallable, Category = "EOS_Functions")
	bool IsPlayerLoggedIn() const;

	UFUNCTION(BlueprintCallable, Category = "EOS_Functions")
	void CreateEOSSession(bool bIsDedicatedServer, bool bIsLanServer, int32 NumberOfPublicConnections);

	void OnLoginCompleted(int32 LocalUserNum, bool bSuccessful, const FUniqueNetId& UserId, const FString& Error);

public:
	FString LevelPath = L"/Game/FuryOfLegends/Level/Arena?listen";

public:
	FCSOnCreateSessionComplete OnCreateSessionCompleteEvent;
	FCSOnFindSessionsComplete OnFindSessionsCompleteEvent;
	FCSOnJoinSessionComplete OnJoinSessionCompleteEvent;
	FCSOnUpdateSessionComplete OnUpdateSessionCompleteEvent;
	FCSOnStartSessionComplete OnStartSessionCompleteEvent;
	FCSOnEndSessionComplete OnEndSessionCompleteEvent;
	FCSOnDestroySessionComplete OnDestroySessionCompleteEvent;
	FCOnLoginCompleteDelegate OnLoginCompleteEvent;

protected:
	void OnCreateSessionCompleted(FName SessionName, bool Successful);
	void OnUpdateSessionCompleted(FName SessionName, bool Successful);
	void OnStartSessionCompleted(FName SessionName, bool Successful);
	void OnEndSessionCompleted(FName SessionName, bool Successful);
	void OnDestroySessionCompleted(FName SessionName, bool Successful);
	void OnFindSessionsCompleted(bool Successful);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	bool TryClientTravelToCurrentSession();

private:
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;

	FOnUpdateSessionCompleteDelegate UpdateSessionCompleteDelegate;
	FDelegateHandle UpdateSessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	FOnEndSessionCompleteDelegate EndSessionCompleteDelegate;
	FDelegateHandle EndSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
};
