// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/JoinMenuUI.h"
#include "Components/ListView.h"
#include "Components/Button.h"
#include "Structs/SessionInfomation.h"
#include "Plugins/MultiplaySessionSubsystem.h"


void UJoinMenuUI::NativeConstruct()
{
    Super::NativeConstruct();

    UGameInstance* GameInstance = GetGameInstance();
    if (!::IsValid(GameInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("UJoinMenuUI::NativeConstruct - Invalid GameInstance."));
        return;
    }

    UMultiplaySessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplaySessionSubsystem>();
    if (!::IsValid(SessionSubsystem))
    {
        UE_LOG(LogTemp, Error, TEXT("UJoinMenuUI::NativeConstruct - Invalid SessionSubsystem."));
        return;
    }

    if (::IsValid(CancelButton))
    {
        CancelButton->OnClicked.AddDynamic(this, &UJoinMenuUI::RemoveJoinMenu);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UJoinMenuUI::NativeConstruct - Invalid CancelButton."));
    }

    if (::IsValid(RefreshButton))
    {
        RefreshButton->OnClicked.AddDynamic(this, &UJoinMenuUI::FindSession);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UJoinMenuUI::NativeConstruct - Invalid RefreshButton."));
    }

    SessionSubsystem->OnFindSessionsCompleteEvent.AddUObject(this, &UJoinMenuUI::AddEntryServerList);
}


void UJoinMenuUI::FindSession()
{
    // 서버 목록 초기화
    if (::IsValid(ServerList)) ServerList->ClearListItems();

    UGameInstance* GameInstance = GetGameInstance();
    if (!::IsValid(GameInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("UJoinMenuUI::FindSession - Invalid GameInstance."));
        return;
    }

    UMultiplaySessionSubsystem* SessionSystem = GameInstance->GetSubsystem<UMultiplaySessionSubsystem>();
    if (!::IsValid(SessionSystem))
    {
        UE_LOG(LogTemp, Error, TEXT("UJoinMenuUI::FindSession - Invalid SessionSubsystem."));
        return;
    }

    SessionSystem->FindSessions(10, false);
}


void UJoinMenuUI::AddEntryServerList(const TArray<USessionInfomation*>& SessionResults, bool Successful)
{
	for (auto& Session : SessionResults)
	{
		ServerList->AddItem(Session);
	}
}

void UJoinMenuUI::RemoveJoinMenu()
{
	RemoveFromParent();
}