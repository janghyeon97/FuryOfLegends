// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UW_ListViewEntry.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnJoinButtonClickedDelegate, int Index)

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_ListViewEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UFUNCTION()
	void OnJoinButtonClicked();

	FOnJoinButtonClickedDelegate OnJoinButtonClickedEvent;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> SessionNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> MapNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> CurrentPlayers;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> MaxPlayers;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> PingText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UUW_Button> JoinButton;

	UPROPERTY()
	int32 Index;

	class USessionInfomation* Session;

	//FOnlineSessionSearchResult* SessionResult;
};
