// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UW_ListViewEntry.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/UW_Button.h"
#include "Structs/SessionInfomation.h"
#include "Plugins/MultiplaySessionSubsystem.h"
#include "Controllers/UIPlayerController.h"
#include "Kismet/GameplayStatics.h"

void UUW_ListViewEntry::NativeConstruct()
{
	Super::NativeConstruct();

}

void UUW_ListViewEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Session = Cast<USessionInfomation>(ListItemObject);

	Index = Session->GetIndex();

	SessionNameText->SetText(FText::FromString(Session->GetSessionName()));
	MapNameText->SetText(FText::FromString(Session->GetMapName()));

	FString CurrentPlayersString = FString::Printf(TEXT("%d"), Session->GetCurrentPlayers());
	CurrentPlayers->SetText(FText::FromString(CurrentPlayersString));

	FString MaxPlayersString = FString::Printf(TEXT("%d"), Session->GetMaxPlayers());
	MaxPlayers->SetText(FText::FromString(MaxPlayersString));

	FString PingString = FString::Printf(TEXT("%d"), Session->GetPing());
	PingText->SetText(FText::FromString(PingString));

	JoinButton->Button->OnClicked.AddDynamic(this, &UUW_ListViewEntry::OnJoinButtonClicked);
}

void UUW_ListViewEntry::OnJoinButtonClicked()
{
	AUIPlayerController* PlayerController = Cast<AUIPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (::IsValid(PlayerController))
	{
		if (Session->GetIsExistPassword())
		{
			PlayerController->SelectedSession = Session;
			PlayerController->ShowPasswordMenu();
		}
		else
		{
			PlayerController->SelectedSession = Session;
			PlayerController->JoinSession(FString());
		}
	}


}
