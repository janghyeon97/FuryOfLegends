// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UHUD.h"

// Core 관련 헤더
#include "Game/AOSGameInstance.h"
#include "Game/ArenaPlayerState.h"
#include "Game/ArenaGameState.h"

// 캐릭터 및 컴포넌트 관련 헤더
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/ActionStatComponent.h"

// UMG 및 UI 컴포넌트 관련 헤더
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/ComboBoxString.h"
#include "Components/CanvasPanel.h"
#include "Components/SizeBox.h"

// 사용자 정의 UI 클래스 헤더
#include "UI/UW_AbilityLevel.h"
#include "UI/UW_Inventory.h"
#include "UI/UW_BuffListEntry.h"
#include "UI/UW_ItemEntry.h"	
#include "UI/UW_ItemShop.h"
#include "UI/UW_StatPanel.h"
#include "UI/UW_ResourceBars.h"
#include "UI/UW_ChampionOverview.h"
#include "UI/UW_ActionPanel.h"

// 기타 필요한 헤더
#include "Item/ItemData.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Structs/ActionData.h"

// 플러그인 헤더
#include "Plugins/UniqueCodeGenerator.h"


void UUHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();

}

void UUHUD::InitializeHUD(AAOSCharacterBase* OwningPlayer)
{
	UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (::IsValid(GameInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid GameInstance."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (::IsValid(OwningPlayer) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid OwningPlayer."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	OwningCharacter = OwningPlayer;
	PlayerCharacterName = OwningPlayer->GetCharacterName();
	
	UStatComponent* NewStatComponenet = OwningPlayer->GetStatComponent();
	if (::IsValid(NewStatComponenet))
	{
		StatPanel->InitializeWidget(NewStatComponenet);
		ResourceBars->InitializeWidget(NewStatComponenet);

		BindStatComponent(NewStatComponenet);
	}

	UActionStatComponent* NewActionStatComponent = OwningPlayer->GetActionStatComponent();
	if (::IsValid(NewActionStatComponent))
	{
		ActionStatComponent = NewActionStatComponent;
		
		InitializeActions();

		ActionStatComponent->OnCooldownTimeChanged.AddDynamic(this, &ThisClass::OnActionCooldownTimeChanged);
		ActionStatComponent->OnAlertTextChanged.AddDynamic(this, &ThisClass::OnAlertTextChanged);
		ActionStatComponent->OnActionLevelChanged.AddDynamic(this, &ThisClass::OnActionLevelChanged);
		ActionStatComponent->OnActivationChanged.AddDynamic(this, &ThisClass::OnActionActivationChanged);
		ActionStatComponent->OnUpgradeStateChanged.AddDynamic(this, &ThisClass::OnActionUpgradeStateChanged);
	}

	AArenaPlayerState* ArenaPlayerState = Cast<AArenaPlayerState>(OwningPlayer->GetPlayerState());
	if (::IsValid(ArenaPlayerState))
	{
		PlayerState = ArenaPlayerState;
		PlayerIndex = PlayerState->GetPlayerIndex();

		PlayerState->OnInventoryUpdated.AddDynamic(this, &ThisClass::OnInventoryChanged);
		PlayerState->OnCurrencyUpdated.AddDynamic(this, &ThisClass::OnCurrencyChanged);
		PlayerState->OnTimerUpdated.AddDynamic(this, &ThisClass::OnTimerUpdated);
		PlayerState->OnRemainingTimeChanged.AddDynamic(this, &ThisClass::HandleTimerByCategory);
	}

	const FCharacterAttributesRow* CharacterAttributesRow = GameInstance->GetChampionListTableRow(PlayerCharacterName);
	if (!CharacterAttributesRow)
	{
		return;
	}

	UTexture* Texture = CharacterAttributesRow->ChampionImage;
	AArenaGameState* ArenaGameState = Cast<AArenaGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (::IsValid(ArenaGameState))
	{
		GameState = ArenaGameState;
		ChampionOverview->InitializeWidget(ArenaGameState, NewStatComponenet, PlayerIndex, PlayerCharacterName, Texture);
	}

	if (OnComponentsBindingCompleted.IsBound())
	{
		OnComponentsBindingCompleted.Broadcast();
	}
}


void UUHUD::BindStatComponent(UStatComponent* InStatComponent)
{
	if (::IsValid(InStatComponent))
	{
		StatComponent = InStatComponent;

		DecreaseHPButton->OnClicked.AddDynamic(this, &UUHUD::DecreaseHP);
		DecreaseMPButton->OnClicked.AddDynamic(this, &UUHUD::DecreaseMP);
		IncreaseLevelButton->OnClicked.AddDynamic(this, &UUHUD::IncreaseLevel);
		IncreaseEXPButton->OnClicked.AddDynamic(this, &UUHUD::IncreaseEXP);
		IncreaseCriticalButton->OnClicked.AddDynamic(this, &UUHUD::IncreaseCriticalChance);
		IncreaseAttackSpeedButton->OnClicked.AddDynamic(this, &UUHUD::IncreaseAttackSpeed);
		TeamSelectionComboBox->OnSelectionChanged.AddDynamic(this, &UUHUD::ChangeTeamSide);

		HUDToggleButton->OnClicked.AddDynamic(this, &UUHUD::ToggleHUD);
	}
}


void UUHUD::BindItemShop(UUW_ItemShop* InItemShop)
{
	if (::IsValid(InItemShop) == false)
	{
		return;
	}

	ItemShop = InItemShop;
}


void UUHUD::InitializeActions()
{
	if (::IsValid(OwningCharacter) == false)
	{
		return;
	}

	UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeActions: Failed to retrieve the World context. Unable to proceed with initialization."));
		return;
	}

	if (!ActionStatComponent.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeActions: ActionStatComponent is not valid."));
		return;
	}

	TArray<FActiveActionState*> ActionPtrs = ActionStatComponent->GetActiveActionStatePtrs();
	if (ActionPtrs.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeActions: ActionPtrs is empty."));
		return;
	}

	for (FActiveActionState* ActionPtr : ActionPtrs)
	{
		if (ActionPtr == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("InitializeActions: ActionPtr is null."));
			continue;
		}

		if (ActionPtr->Name.IsNone())
		{
			UE_LOG(LogTemp, Error, TEXT("InitializeActions: ActionPtr->Name is None."));
			continue;
		}

		const UEnum* EnumPtr = StaticEnum<EActionSlot>();
		if (!EnumPtr || !EnumPtr->IsValidEnumValue(static_cast<int64>(ActionPtr->SlotID))) {
			UE_LOG(LogTemp, Error, TEXT("InitializeActions: EnumPtr is None."));
			return; // Enum 유효성 검사
		}

		FString ActionBaseName = EnumPtr->GetNameStringByValue(static_cast<int64>(ActionPtr->SlotID));
		FName ActionVariantName = FName(FString::Printf(TEXT("%s%d"), *ActionBaseName, 1));

		UTexture* ActionTexture = OwningCharacter->GetOrLoadTexture(ActionVariantName,
			*FString::Printf(TEXT("/Game/FuryOfLegends/Characters/%s/Images/T_Action_%s.T_Action_%s"), *PlayerCharacterName.ToString(), *ActionVariantName.ToString(), *ActionVariantName.ToString()));
		if (ActionTexture == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("InitializeActions: ActionTexture is null."));
			continue;
		}

		UUW_ActionPanel* ActionPanel = CreateWidget<UUW_ActionPanel>(CurrentWorld, ActionPanelClass);
		if (ActionPanel)
		{
			ActionPanel->SetActionSlotText(ActionBaseName);
			ActionPanel->SetActionImage(ActionTexture);
			ActionPanel->SetActionMaxLevel(ActionPtr->MaxLevel);
		}
		else
		{
			continue;
		}

		UHorizontalBoxSlot* HorizontalBoxSlot = ActionPanels->AddChildToHorizontalBox(ActionPanel);
		if (HorizontalBoxSlot)
		{

		}

		Actions.Add(ActionPtr->SlotID, ActionPanel);
	}
}

void UUHUD::DecreaseHP()
{
	AAOSCharacterBase* OwningPlayer = Cast<AAOSCharacterBase>(OwningCharacter);
	if (true == ::IsValid(OwningPlayer))
	{
		OwningPlayer->DecreaseHP_Server();
	}
}

void UUHUD::DecreaseMP()
{
	AAOSCharacterBase* OwningPlayer = Cast<AAOSCharacterBase>(OwningCharacter);
	if (true == ::IsValid(OwningPlayer))
	{
		OwningPlayer->DecreaseMP_Server();
	}
}

void UUHUD::IncreaseEXP()
{
	AAOSCharacterBase* OwningPlayer = Cast<AAOSCharacterBase>(OwningCharacter);
	if (true == ::IsValid(OwningPlayer))
	{
		OwningPlayer->IncreaseEXP_Server();
	}
}

void UUHUD::IncreaseLevel()
{
	AAOSCharacterBase* OwningPlayer = Cast<AAOSCharacterBase>(OwningCharacter);
	if (true == ::IsValid(OwningPlayer))
	{
		OwningPlayer->IncreaseLevel_Server();
	}
}

void UUHUD::IncreaseCriticalChance()
{
	AAOSCharacterBase* OwningPlayer = Cast<AAOSCharacterBase>(OwningCharacter);
	if (true == ::IsValid(OwningPlayer))
	{
		OwningPlayer->IncreaseCriticalChance_Server();
	}
}

void UUHUD::IncreaseAttackSpeed()
{
	AAOSCharacterBase* OwningPlayer = Cast<AAOSCharacterBase>(OwningCharacter);
	if (true == ::IsValid(OwningPlayer))
	{
		OwningPlayer->IncreaseAttackSpeed_Server();
	}
}

void UUHUD::ChangeTeamSide(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	AAOSCharacterBase* OwningPlayer = Cast<AAOSCharacterBase>(OwningCharacter);
	if (true == ::IsValid(OwningPlayer))
	{
		if (SelectedItem.Equals("Blue"))
		{
			OwningPlayer->ChangeTeamSide_Server(ETeamSide::Blue);
		}
		else if (SelectedItem.Equals("Red"))
		{
			OwningPlayer->ChangeTeamSide_Server(ETeamSide::Red);
		}
		else
		{
			UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Team change failed. Please choose between blue and red."), true, true, FLinearColor::Red, 2.0f, FName("Name_None"));
		}
	}
}

void UUHUD::ToggleHUD()
{
	if (::IsValid(HUD) == false)
	{
		return;
	}

	ESlateVisibility CurrentVisibility = HUD->GetVisibility();
	if (CurrentVisibility == ESlateVisibility::Visible)
	{
		HUD->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		HUD->SetVisibility(ESlateVisibility::Visible);
	}
}

void UUHUD::OnActionActivationChanged(EActionSlot SlotID, bool InVisibility)
{
	if (!Actions.Contains(SlotID))
	{
		UE_LOG(LogTemp, Error, TEXT("OnActionActivationChanged: Actions does not contain SlotID: %d"), static_cast<int32>(SlotID));
		return;
	}

	UUW_ActionPanel* ActionPanel = Actions[SlotID];
	if (ActionPanel)
	{
		ActionPanel->SetActionActivation(InVisibility);
	}
}


void UUHUD::OnActionUpgradeStateChanged(EActionSlot SlotID, bool InVisibility)
{
	if (!Actions.Contains(SlotID))
	{
		UE_LOG(LogTemp, Error, TEXT("OnActionUpgradeStateChanged: Actions does not contain SlotID: %d"), static_cast<int32>(SlotID));
		return;
	}
	UUW_ActionPanel* ActionPanel = Actions[SlotID];
	if (ActionPanel)
	{
		ActionPanel->SetLevelUpArrowVisibility(InVisibility);
	}
}


void UUHUD::OnActionLevelChanged(EActionSlot SlotID, int InLevel)
{
	if (!Actions.Contains(SlotID))
	{
		UE_LOG(LogTemp, Error, TEXT("OnActionLevelChanged: Actions does not contain SlotID: %d"), static_cast<int32>(SlotID));
		return;
	}

	UUW_ActionPanel* ActionPanel = Actions[SlotID];	
	if (::IsValid(ActionPanel))
	{
		ActionPanel->SetActionCurrentLevel(InLevel);
	}
}


void UUHUD::OnActionCooldownTimeChanged(EActionSlot SlotID, float CurrentCooldownTime, float MaxCooldownTime)
{
	if (!Actions.Contains(SlotID))
	{
		UE_LOG(LogTemp, Error, TEXT("OnActionCooldownTimeChanged: Actions does not contain SlotID: %d"), static_cast<int32>(SlotID));
		return;
	}	

	UUW_ActionPanel* ActionPanel = Actions[SlotID];
	if (ActionPanel)
	{
		ActionPanel->SetCooldownTime(CurrentCooldownTime, MaxCooldownTime);
	}
}



void UUHUD::OnAlertTextChanged(const FString& InString)
{
	FTimerHandle NewTimerHandle;

	DescriptionText->SetVisibility(ESlateVisibility::Visible);
	DescriptionText->SetText(FText::FromString(InString));

	GetWorld()->GetTimerManager().SetTimer(
		NewTimerHandle, [&]()
		{
			DescriptionText->SetText(FText());
			DescriptionText->SetVisibility(ESlateVisibility::Hidden);
		},
		0.1f,
		false,
		2.0f
	);
}


void UUHUD::OnInventoryChanged(const int32 InventoryIndex, const int32 ItemCode, const int32 CurrentStack)
{
	if (!GameState.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: GameState is invalid (nullptr)."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (::IsValid(Inventory) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Inventory is invalid (nullptr)."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (ItemCode <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Clearing inventory slot %d (ItemCode: %d)."), ANSI_TO_TCHAR(__FUNCTION__), InventoryIndex, ItemCode);
		Inventory->UpdateItem(InventoryIndex, 0);
		Inventory->UpdateItemImage(nullptr, InventoryIndex);
		Inventory->UpdateItemCountText(0, InventoryIndex);
		return;
	}

	FItemTableRow* ItemInformation = GameState->GetItemInfoByID(ItemCode);
	if (!ItemInformation)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: No item information found for ItemCode: %d. TimerID:"), ANSI_TO_TCHAR(__FUNCTION__), ItemCode);
		return;
	}

	Inventory->UpdateItem(InventoryIndex, ItemCode);
	Inventory->UpdateItemImage(ItemInformation->Icon, InventoryIndex);
	Inventory->UpdateItemCountText(CurrentStack, InventoryIndex);
}

void UUHUD::OnCurrencyChanged(const int32 NewCurrency)
{
	FString CurrencyString = FString::Printf(TEXT("%d"), NewCurrency);
	Inventory->UpdateCurrencyText(FText::FromString(CurrencyString));
}


void UUHUD::OnTimerUpdated(const uint32 UniqueCode, const int32 ConcurrentUses)
{
	if (!TimerWidgets.Contains(UniqueCode))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: No TimerWidget found for UniqueCode %u."), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
		return;
	}

	UUW_ItemEntry* Widget = TimerWidgets[UniqueCode];
	if (!::IsValid(Widget))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: TimerWidget for UniqueCode %u is invalid (nullptr)."), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
		return;
	}

	Widget->UpdateItemCount(ConcurrentUses);
}




void UUHUD::HandleTimerByCategory(const uint32 UniqueCode, float RemainingTime, float ElapsedTime)
{
	ETimerCategory TimerCategory = UUniqueCodeGenerator::DecodeTimerCategory(UniqueCode);

	switch (TimerCategory)
	{
	case ETimerCategory::Action:
		UpdateActionTimer(UniqueCode, RemainingTime, ElapsedTime);
		break;
	case ETimerCategory::Item:
		UpdateItemTimer(UniqueCode, RemainingTime, ElapsedTime);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("[%s] Unknown DataType for UniqueCode: %u. RemainingTime: %.2f, ElapsedTime: %.2f"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode, RemainingTime, ElapsedTime);
		break;
	}
}


void UUHUD::UpdateActionTimer(const uint32 UniqueCode, float RemainingTime, float ElapsedTime)
{
	if (RemainingTime <= 0.15f && TimerWidgets.Contains(UniqueCode))
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Removing expired TimerWidget for UniqueCode: %d"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode);
		TimerWidgets[UniqueCode]->RemoveFromParent();
		TimerWidgets.Remove(UniqueCode);
		return;
	}

	EActionSlot SlotID = static_cast<EActionSlot>(UUniqueCodeGenerator::DecodeSubField1(UniqueCode));
	uint8 AttackPhase = UUniqueCodeGenerator::DecodeSubField2(UniqueCode);

	// SlotName 생성
	FName SlotName = (SlotID == EActionSlot::None) ? NAME_None : FName(*FString::Printf(TEXT("%s%u"), *StaticEnum<EActionSlot>()->GetNameStringByValue(static_cast<int64>(SlotID)), AttackPhase));

	float CooldownRatio = ElapsedTime / (RemainingTime + ElapsedTime);
	UUW_ItemEntry* Widget = TimerWidgets.Contains(UniqueCode) ? TimerWidgets[UniqueCode] : nullptr;

	// 기존 위젯이 존재할 경우 처리 후 종료
	if (::IsValid(Widget))
	{
		Widget->UpdateCooldownPercent(CooldownRatio);
		return;
	}

	if (::IsValid(OwningCharacter) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Skipping because OwningCharacter is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UTexture* Texture = OwningCharacter->GetOrLoadTexture(SlotName, *FString::Printf(TEXT("/Game/FuryOfLegends/Characters/%s/Images/T_Ability_%s.T_Ability_%s"), *PlayerCharacterName.ToString(), *SlotName.ToString(), *SlotName.ToString()));
	if (!Texture)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid ability texture for UniqueCode: %d"), UniqueCode);
		return;
	}

	// 새 위젯 생성
	Widget = CreateWidget<UUW_ItemEntry>(this, ItemEntryClass);
	if (::IsValid(Widget) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create BuffWidget for UniqueCode: %d"), UniqueCode);
		return;
	}

	Widget->UpdateItemImage(Texture);
	Widget->UpdateDisplaySubItems(false);
	Widget->UpdateCanPurchaseItem(false);
	Widget->UpdatePriceVisibility(ESlateVisibility::Collapsed);
	Widget->UpdateCountPadding(FMargin(0, 0, 2, 0));
	Widget->UpdateCountTextSize(0.7f);
	Widget->SetSize(40.f);

	Widget->UpdateCooldownPercent(CooldownRatio);
	Widget->UpdateCooldownMaskOpacity(0.01f);

	TimerWidgets.Add(UniqueCode, Widget);
	BuffList->AddChildToHorizontalBox(Widget);
}


void UUHUD::UpdateItemTimer(const uint32 UniqueCode, float RemainingTime, float ElapsedTime)
{
	if (RemainingTime <= 0.15f && TimerWidgets.Contains(UniqueCode))
	{
		TimerWidgets[UniqueCode]->RemoveFromParent();
		TimerWidgets.Remove(UniqueCode);
		return;
	}

	ETimerType TimerType = static_cast<ETimerType>(UUniqueCodeGenerator::DecodeSubField1(UniqueCode));

	float CooldownRatio = ElapsedTime / (RemainingTime + ElapsedTime);
	UUW_ItemEntry* Widget = TimerWidgets.Contains(UniqueCode) ? TimerWidgets[UniqueCode] : nullptr;

	switch (TimerType)
	{
	case ETimerType::Inventory:
		UpdateInventoryTimer(UniqueCode, RemainingTime, ElapsedTime);
		break;
	case ETimerType::BuffList:
		UpdateItemBuffTimer(UniqueCode, RemainingTime, ElapsedTime);
		break;
	case ETimerType::Both:
		UpdateInventoryTimer(UniqueCode, RemainingTime, ElapsedTime);
		UpdateItemBuffTimer(UniqueCode, RemainingTime, ElapsedTime);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("[%s] Unknown DataType for UniqueCode: %u. RemainingTime: %.2f, ElapsedTime: %.2f"), ANSI_TO_TCHAR(__FUNCTION__), UniqueCode, RemainingTime, ElapsedTime);
		break;
	}
}

void UUHUD::UpdateInventoryTimer(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime)
{
	const int32 ItemCode = UUniqueCodeGenerator::DecodeSubField2(UniqueCode);

	// UniqueCode가 Inventory->UniqueCodeToIndx에 존재하지 않는 경우
	if (!Inventory->ItemCodeToIndexMap.Contains(ItemCode))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Unknown UniqueCode: %u. RemainingTime: %.2f, ElapsedTime: %.2f"),
			ANSI_TO_TCHAR(__FUNCTION__), UniqueCode, RemainingTime, ElapsedTime);
		return;
	}

	// ItemCode 에 해당하는 인덱스 가져오기
	int32 Index = Inventory->ItemCodeToIndexMap[ItemCode];

	// 남은 시간이 0.15초 이하인 경우
	if (RemainingTime <= 0.15f)
	{
		Inventory->UpdateCooldownRatio(0, Index);
		return;
	}
	else
	{
		// 쿨다운 비율 계산

		float CooldownRatio = ElapsedTime / (RemainingTime + ElapsedTime);
		Inventory->UpdateCooldownRatio(CooldownRatio, Index);
	}
}

void UUHUD::UpdateItemBuffTimer(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime)
{
	if (RemainingTime <= 0.15f && TimerWidgets.Contains(UniqueCode))
	{
		TimerWidgets[UniqueCode]->RemoveFromParent();
		TimerWidgets.Remove(UniqueCode);
		return;
	}

	ETimerType TimerType = static_cast<ETimerType>(UUniqueCodeGenerator::DecodeSubField1(UniqueCode));
	const int32 ItemCode = UUniqueCodeGenerator::DecodeSubField2(UniqueCode);

	float CooldownRatio = ElapsedTime / (RemainingTime + ElapsedTime);
	UUW_ItemEntry* Widget = TimerWidgets.Contains(UniqueCode) ? TimerWidgets[UniqueCode] : nullptr;

	// 기존 위젯이 존재할 경우 처리 후 종료
	if (::IsValid(Widget))
	{
		Widget->UpdateCooldownPercent(CooldownRatio);
		return;
	}

	if (!GameState.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed: GameState is invalid. Cannot retrieve item information."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	FItemTableRow* ItemInformation = GameState->GetItemInfoByID(ItemCode);
	if (!ItemInformation)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: No item information found for ItemCode: %d. UniqueCode: %d"), ANSI_TO_TCHAR(__FUNCTION__), ItemCode, UniqueCode);
		return;
	}

	// 새 위젯 생성
	Widget = CreateWidget<UUW_ItemEntry>(this, ItemEntryClass);
	if (::IsValid(Widget) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create BuffWidget for UniqueCode: %d"), UniqueCode);
		return;
	}

	Widget->UpdateItemImage(ItemInformation->Icon);
	Widget->UpdateDisplaySubItems(false);
	Widget->UpdateCanPurchaseItem(false);
	Widget->UpdatePriceVisibility(ESlateVisibility::Collapsed);
	Widget->UpdateCountPadding(FMargin(0, 0, 2, 0));
	Widget->UpdateCountTextSize(0.7f);
	Widget->SetSize(40.f);

	Widget->UpdateCooldownPercent(CooldownRatio);
	Widget->UpdateCooldownMaskOpacity(0.1f);

	TimerWidgets.Add(UniqueCode, Widget);
	BuffList->AddChildToHorizontalBox(Widget);
}


