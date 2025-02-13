// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/AOSPlayerController.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/ActionStatComponent.h"
#include "Game/ArenaPlayerState.h"
#include "Game/ArenaGameMode.h"
#include "Game/ArenaGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MatchResultUI.h"
#include "UI/LoadingScreenUI.h"
#include "UI/TargetStatusWidget.h"
#include "UI/UW_ItemShop.h"
#include "UI/UHUD.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Camera/CameraActor.h"
#include "Props/Nexus.h"

AAOSPlayerController::AAOSPlayerController()
{

}

void AAOSPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();


}

void AAOSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		ItemShopWidget = CreateWidget<UUW_ItemShop>(this, ItemShopClass);
		if (::IsValid(ItemShopWidget))
		{
			ItemShopWidget->AddToViewport(1);
			ItemShopWidget->SetVisibility(ESlateVisibility::Hidden);
		}

		ShowLoadingScreen();
		CreateHUD();
		CreateTargetStatusWidget();
		DisplayCrosshair();
		ServerNotifyLoaded();
	}
}

void AAOSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


}



void AAOSPlayerController::ClientBindTargetStatusWidget_Implementation(UStatComponent* ObjectStatComponent)
{
	if (::IsValid(TargetStatusWidget) == false)
	{
		CreateTargetStatusWidget();
		return;
	}

	if (!ObjectStatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: ObjectStatComponent is invalid or null."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// Reset and initialize the TargetStatusWidget with the Nexus's StatComponent
	TargetStatusWidget->ResetWidget();
	TargetStatusWidget->SetVisibility(ESlateVisibility::Visible);
	TargetStatusWidget->InitializeWidget(ObjectStatComponent);
}



void AAOSPlayerController::ClientRemoveTargetStatusWidget_Implementation()
{
	if (::IsValid(TargetStatusWidget) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: TargetStatusWidget is invalid or null."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	TargetStatusWidget->ResetWidget();
	TargetStatusWidget->SetVisibility(ESlateVisibility::Hidden);
}




void AAOSPlayerController::InitializeHUD_Implementation(const FName& InChampionName)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] Initializing HUD for Champion: %s"), ANSI_TO_TCHAR(__FUNCTION__), *InChampionName.ToString());

	// 입력 모드 설정
	SetInputMode(FInputModeGameOnly());

	// HUD 위젯 확인 및 생성
	if (::IsValid(HUDWidget) == false)
	{
		CreateHUD();

		if (::IsValid(HUDWidget) == false)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create HUDWidget."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}
	}

	// 플레이어 캐릭터 확인
	AAOSCharacterBase* PlayerCharacter = GetPawn<AAOSCharacterBase>();
	if (::IsValid(PlayerCharacter) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Initialize HUD Failed: PlayerCharacter is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	HUDWidget->OnComponentsBindingCompleted.AddUObject(this, &ThisClass::OnHUDBindingComplete);
	HUDWidget->InitializeHUD(PlayerCharacter);
}


void AAOSPlayerController::ServerNotifyLoaded_Implementation()
{
	// 서버에 로딩 완료 알림
	if (AArenaGameMode* GM = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->PlayerLoaded(this);
	}
}

void AAOSPlayerController::ServerNotifyCharacterSpawned_Implementation()
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Not authorized to notify character spawn."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed: GetWorld() returned nullptr."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	AArenaGameMode* GameMode = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(World));
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed: Unable to cast GameMode to AArenaGameMode."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	AAOSCharacterBase* PlayerCharacter = GetPawn<AAOSCharacterBase>();
	if (::IsValid(PlayerCharacter) == false)
	{
		return;
	}

	GameMode->PlayerPawnReady(PlayerCharacter);
}

void AAOSPlayerController::ToggleItemShopVisibility_Implementation()
{
	if (::IsValid(ItemShopWidget) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemShopWidget is not valid"));
		return;
	}

	if (bItemShopVisibility)
	{
		// 닫기
		ItemShopWidget->SetVisibility(ESlateVisibility::Hidden);

		FInputModeGameOnly Mode;
		SetInputMode(Mode);

		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
	}
	else
	{
		// 열기
		ItemShopWidget->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI Mode;
		//Mode.SetWidgetToFocus(ItemShopWidget->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);

		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
	}

	bItemShopVisibility = !bItemShopVisibility;
}

void AAOSPlayerController::InitializeItemShop_Implementation()
{
	if (::IsValid(ItemShopWidget) == false)
	{
		return;
	}

	AAOSCharacterBase* PlayerCharacter = GetPawn<AAOSCharacterBase>();
	if (::IsValid(PlayerCharacter) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Initialize HUD Failed: PlayerCharacter is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	AArenaPlayerState* ArenaPlayerState = Cast<AArenaPlayerState>(PlayerState);
	if (::IsValid(ArenaPlayerState))
	{
		ItemShopWidget->BindPlayerState(ArenaPlayerState);
		ItemShopWidget->SetOwningActor(PlayerCharacter);
	}
}

void AAOSPlayerController::OnHUDBindingComplete_Implementation()
{
	int32 InitialCharacterLevel = 1;

	if (AArenaGameMode* GM = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		InitialCharacterLevel = GM->GetInitialCharacterLevel();
	}

	AAOSCharacterBase* PlayerCharacter = GetPawn<AAOSCharacterBase>();
	if (!::IsValid(PlayerCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] PlayerCharacter is not valid"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UStatComponent* StatComponent = PlayerCharacter->GetStatComponent();
	UActionStatComponent* ActionStatComponent = PlayerCharacter->GetActionStatComponent();

	if (!::IsValid(StatComponent) || !::IsValid(ActionStatComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] StatComponent or ActionStatComponent is not valid"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	ActionStatComponent->InitializeActionAtLevel(EActionSlot::LMB, 1);
	StatComponent->SetCurrentLevel(InitialCharacterLevel);

	ServerNotifyCharacterSpawned();

	UE_LOG(LogTemp, Log, TEXT("[%s] HUD successfully initialized."), ANSI_TO_TCHAR(__FUNCTION__));
}


void AAOSPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AArenaPlayerState* ArenaPlayerState = Cast<AArenaPlayerState>(PlayerState);
	if (!ArenaPlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] PlayerState is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	PlayerState->SetOwner(this);
}

void AAOSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

}



void AAOSPlayerController::DisplayCrosshair_Implementation()
{
	if (!CrosshairClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] CrosshairClass is not set."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (::IsValid(CrosshairWidget) == false)
	{
		CrosshairWidget = CreateWidget<UUserWidget>(this, CrosshairClass);
		if (::IsValid(CrosshairWidget) == false)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create CrosshairWidget."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}
	}

	CrosshairWidget->AddToViewport(1);
}

void AAOSPlayerController::ShowLoadingScreen_Implementation()
{
	if (!LoadingScreenClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] LoadingScreenClass is not set."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (::IsValid(LoadingScreenWidget) == false)
	{
		LoadingScreenWidget = CreateWidget<UUserWidget>(this, LoadingScreenClass);
		if (::IsValid(LoadingScreenWidget) == false)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create LoadingScreenWidget."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}
	}

	LoadingScreenWidget->AddToViewport(1);
}


void AAOSPlayerController::RemoveLoadingScreen_Implementation()
{
	if (::IsValid(LoadingScreenWidget))
	{
		LoadingScreenWidget->RemoveFromParent();
	}
}


void AAOSPlayerController::ClientTravelToTitle_Implementation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] World context is null!"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	ClientTravel(TEXT("Title"), TRAVEL_Absolute);
}

void AAOSPlayerController::ClientHandleGameEnd_Implementation(ANexus* Nexus, bool bIsVictorious)
{
	if (IsLocalController() == false)
	{
		return;
	}

	DisableInput(this);

	if (::IsValid(HUDWidget))
	{
		HUDWidget->RemoveFromParent();
	}

	if (::IsValid(CrosshairWidget))
	{
		CrosshairWidget->RemoveFromParent();
	}

	if (MatchResultUIClass)
	{
		const FString MatchResultString = bIsVictorious
			? FText::FromString(TEXT("Victory")).ToString()
			: FText::FromString(TEXT("Defeat")).ToString();

		MatchResultWidet = CreateWidget<UMatchResultUI>(this, MatchResultUIClass);
		MatchResultWidet->UpdateResultText(MatchResultString);

		MatchResultWidet->AddToViewport(0);
	}

	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(MatchResultWidet->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	if (Nexus)
	{
		Nexus->ActivateCamera();
		SetViewTargetWithBlend(Nexus, 2.0f);
	}
}


void AAOSPlayerController::DisplayMouseCursor(bool bShowCursor)
{
	if (bShowCursor)
	{
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
	}
	else
	{
		FInputModeGameOnly Mode;
		SetInputMode(Mode);

		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
	}
}


void AAOSPlayerController::CreateHUD()
{
	if (!HUDClass)
	{
		return;
	}

	HUDWidget = CreateWidget<UUHUD>(this, HUDClass);
	if (::IsValid(HUDWidget))
	{
		HUDWidget->AddToViewport(0);
	}
}

void AAOSPlayerController::CreateTargetStatusWidget()
{
	if (!TargetStatusClass)
	{
		return;
	}

	TargetStatusWidget = CreateWidget<UTargetStatusWidget>(this, TargetStatusClass);
	if (::IsValid(TargetStatusWidget))
	{
		TargetStatusWidget->AddToViewport(2);
		TargetStatusWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
