// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SessionInfomation.generated.h"


/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class FURYOFLEGENDS_API USessionInfomation : public UObject
{
	GENERATED_BODY()

public:
	USessionInfomation();

#pragma region GetterAndSetter
	int32 GetIndex() const { return Index; };

	void SetIndex(const int InIndex) { Index = InIndex; };

	FString GetSessionName() const { return SessionName; };

	void SetSessionName(const FString InSessionName) { SessionName = InSessionName; };

	FString GetMapName() const { return MapName; };

	void SetMapName(const FString InMapName) { MapName = InMapName; };

	int32 GetMaxPlayers() const { return MaxPlayers; };

	void SetMaxPlayers(const int InMaxPlayers) { MaxPlayers = InMaxPlayers; };

	int32 GetCurrentPlayers() const { return CurrentPlayers; };

	void SetCurrentPlayers(const int InCurrentPlayers) { CurrentPlayers = InCurrentPlayers; };

	int32 GetPing() const { return Ping; };

	void SetPing(const int InPing) { Ping = InPing; };

	bool GetIsExistPassword() { return IsExistPassword; };

	void SetIsExistPassword(bool IsExist) { IsExistPassword = IsExist; };

	//FOnlineSessionSearchResult* GetSessionResult() const { return SessionResult; };

	//void SetSessionResult(FOnlineSessionSearchResult& InSessionResult) { SessionResult = &InSessionResult; };

#pragma endregion


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess))
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess))
	FString SessionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess))
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess))
	int32 CurrentPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess))
	int32 MaxPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess))
	int32 Ping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess))
	bool IsExistPassword;

	//FOnlineSessionSearchResult* SessionResult;
};
