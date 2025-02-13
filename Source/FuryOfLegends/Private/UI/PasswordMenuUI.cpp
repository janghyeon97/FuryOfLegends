// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PasswordMenuUI.h"
#include "Components/Button.h"
#include "UI/UW_Button.h"
#include "UI/UW_ListViewEntry.h"
#include "UI/UW_EditableText.h"
#include "Kismet/GameplayStatics.h"
#include "Controllers/UIPlayerController.h"
#include "OnlineSessionSettings.h"

void UPasswordMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	ConfirmButton->Button->OnClicked.AddDynamic(this, &UPasswordMenuUI::ConfirmPassword);
	CancelButton->Button->OnClicked.AddDynamic(this, &UPasswordMenuUI::RemovePasswordMenu);
}

void UPasswordMenuUI::ConfirmPassword()
{
	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UPasswordMenuUI::ConfirmPassword] WorldContext is not valid."));
		return;
	}

	AUIPlayerController* PlayerController = Cast<AUIPlayerController>(UGameplayStatics::GetPlayerController(WorldContext, 0));
	if (::IsValid(PlayerController))
	{
		FString InputPassword = EditablePasswordText->GetText();
		InputPassword.RemoveSpacesInline();
		PlayerController->JoinSession(InputPassword);
	}
}

void UPasswordMenuUI::RemovePasswordMenu()
{
	RemoveFromParent();
}