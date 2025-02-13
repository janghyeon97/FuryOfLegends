// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HostMenuUI.h"
#include "Components/EditableText.h"
#include "Components/Button.h"
#include "UI/UW_EditableText.h"
#include "UI/UW_Button.h"
#include "Plugins/MultiplaySessionSubsystem.h"
#include "Controllers/UIPlayerController.h"
#include "Kismet/GameplayStatics.h"

UHostMenuUI::UHostMenuUI(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

void UHostMenuUI::NativeConstruct()
{
	ConfirmButton->Button->OnClicked.AddDynamic(this, &UHostMenuUI::OnConfirmButtonClicked);
	CancelButton->Button->OnClicked.AddDynamic(this, &UHostMenuUI::RemoveHostMenu);
}

void UHostMenuUI::OnConfirmButtonClicked()
{
    AUIPlayerController* PlayerController = Cast<AUIPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
    if (::IsValid(PlayerController))
    {
        PlayerController->ShowLoadingScreen();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UHostMenuUI::OnConfirmButtonClicked - Invalid PlayerController."));
        return;
    }

    UWorld* World = GetWorld();
    if (!::IsValid(World))
    {
        UE_LOG(LogTemp, Error, TEXT("UHostMenuUI::OnConfirmButtonClicked - Invalid World."));
        return;
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (!::IsValid(GameInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("UHostMenuUI::OnConfirmButtonClicked - Invalid GameInstance."));
        return;
    }

    // Timer 汲沥
    FTimerHandle NewTimerHandle;
    World->GetTimerManager().SetTimer(
        NewTimerHandle,
        FTimerDelegate::CreateLambda([this, GameInstance]()
            {
                UMultiplaySessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplaySessionSubsystem>();
                if (!::IsValid(SessionSubsystem))
                {
                    UE_LOG(LogTemp, Error, TEXT("UHostMenuUI::OnConfirmButtonClicked - Invalid SessionSubsystem."));
                    return;
                }

                if (!::IsValid(EditableNameText) || !::IsValid(EditablePasswordText))
                {
                    UE_LOG(LogTemp, Error, TEXT("UHostMenuUI::OnConfirmButtonClicked - Invalid EditableNameText or EditablePasswordText."));
                    return;
                }

                // 技记 积己
                SessionSubsystem->CreateSession(false, true, EditableNameText->GetText(), 10, "Rift", EditablePasswordText->GetText());
            }),
        1.0f,
        false,
        2.0f
    );
}

void UHostMenuUI::RemoveHostMenu()
{
	RemoveFromParent();
}