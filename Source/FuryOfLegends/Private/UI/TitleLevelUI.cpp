// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TitleLevelUI.h"
#include "UI/UW_Button.h"
#include "UI/UW_ConfirmationQuit.h"
#include "Components/Button.h"
#include "Controllers/UIPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Plugins/MultiplaySessionSubsystem.h"

UTitleLevelUI::UTitleLevelUI(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

void UTitleLevelUI::NativeConstruct()
{
    Super::NativeConstruct();

    UWorld* World = GetWorld();
    if (::IsValid(World) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("[UTitleLevelUI::NativeConstruct] WorldContext is not valid."));
        return;
    }

    UMultiplaySessionSubsystem* SessionSystem = World->GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();
    if (!::IsValid(SessionSystem))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UTitleLevelUI::NativeConstruct] SessionSubsystem is not valid."));
        return;
    }

    // 애니메이션 실행
    HostGameButton->PlayConstructAnimation();
    JoinGameButton->PlayConstructAnimation();
    QuitButton->PlayConstructAnimation();

    // 버튼 이벤트 바인딩
    LoginButton->Button->OnClicked.AddDynamic(this, &UTitleLevelUI::ProcessLogin);
    HostGameButton->Button->OnClicked.AddDynamic(this, &UTitleLevelUI::CreateHostMenu);
    JoinGameButton->Button->OnClicked.AddDynamic(this, &UTitleLevelUI::CreateJoinMenu);
    QuitButton->Button->OnClicked.AddDynamic(this, &UTitleLevelUI::CreateQuitMenu);
    SessionSystem->OnLoginCompleteEvent.AddUObject(this, &UTitleLevelUI::OnLoginCompleted);
}

void UTitleLevelUI::CreateHostMenu()
{
    UWorld* World = GetWorld();
    if (::IsValid(World) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("[UTitleLevelUI::NativeConstruct] WorldContext is not valid."));
        return;
    }

    AUIPlayerController* PlayerController = Cast<AUIPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
    if (::IsValid(PlayerController) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UTitleLevelUI::CreateHostMenu] PlayerController is not valid."));
        return;
    }

    PlayerController->ShowHostMenu();
}

void UTitleLevelUI::CreateJoinMenu()
{
    UWorld* World = GetWorld();
    if (::IsValid(World) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("[UTitleLevelUI::CreateJoinMenu] WorldContext is not valid."));
        return;
    }

    UMultiplaySessionSubsystem* SessionSystem = World->GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();
    if (!::IsValid(SessionSystem))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UTitleLevelUI::CreateJoinMenu] SessionSubsystem is not valid."));
        return;
    }

    AUIPlayerController* PlayerController = Cast<AUIPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
    if (::IsValid(PlayerController) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UTitleLevelUI::CreateJoinMenu] PlayerController is not valid."));
        return;
    }

    PlayerController->ShowJoinMenu();
    SessionSystem->FindSessions(10, false);
}

void UTitleLevelUI::CreateQuitMenu()
{
    UWorld* World = GetWorld();
    if (::IsValid(World) == false)
    {
        UE_LOG(LogTemp, Error, TEXT("[UTitleLevelUI::CreateQuitMenu] WorldContext is not valid."));
        return;
    }

    AUIPlayerController* PlayerController = Cast<AUIPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
    if (::IsValid(PlayerController) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UTitleLevelUI::CreateQuitMenu] PlayerController is not valid."));
        return;
    }

    PlayerController->ShowConfirmationQuitMenu();
}

void UTitleLevelUI::ProcessLogin()
{
    UMultiplaySessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();
    if (::IsValid(SessionSubsystem))
    {
        SessionSubsystem->LoginWithEOS("", "", FString("AccountPortal"));
    }
}

void UTitleLevelUI::OnLoginCompleted(bool Successful)
{
    if (Successful)
    {
        HostGameButton->SetIsEnabled(true);
        JoinGameButton->SetIsEnabled(true);
    }
}