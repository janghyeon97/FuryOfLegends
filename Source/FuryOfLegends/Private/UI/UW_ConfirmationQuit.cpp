// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_ConfirmationQuit.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/UW_Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"


UUW_ConfirmationQuit::UUW_ConfirmationQuit(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UUW_ConfirmationQuit::NativeConstruct()
{
	Super::NativeConstruct();

	QuitButton->Button->OnClicked.AddDynamic(this, &UUW_ConfirmationQuit::OnClickQuitButton);
    CancelButton->Button->OnClicked.AddDynamic(this, &UUW_ConfirmationQuit::OnExecuteCancel);

	QuitButton->SetButtonText("Quit");
    CancelButton->SetButtonText("Cancel");
}


void UUW_ConfirmationQuit::OnExecuteQuit_Client_Implementation()
{
    APlayerController* OwningPlayer = GetOwningPlayer();
    if (!OwningPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UUW_ConfirmationQuit::OnExecuteQuit_Client] Owning player is not valid"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UUW_ConfirmationQuit::OnExecuteQuit_Client] World context is not valid"));
        return;
    }

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
    if (!PlayerController || PlayerController != OwningPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UUW_ConfirmationQuit::OnExecuteQuit_Client] PlayerController is not valid or does not match the owning player"));
        return;
    }

    TEnumAsByte<EQuitPreference::Type> QuitPreference = EQuitPreference::Quit;
    UKismetSystemLibrary::QuitGame(World, PlayerController, QuitPreference, true);
}


void UUW_ConfirmationQuit::OnClickQuitButton()
{
	OnExecuteQuit_Client();
}

void UUW_ConfirmationQuit::OnExecuteCancel()
{
    RemoveFromParent();
}