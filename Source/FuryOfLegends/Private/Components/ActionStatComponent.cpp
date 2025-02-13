#include "Components/ActionStatComponent.h"
#include "Components/StatComponent.h"
#include "Characters/AOSCharacterBase.h"
#include "Game/AOSGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

UActionStatComponent::UActionStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bTickEvenWhenPaused = false;
	bWantsInitializeComponent = false;

	StatTable = nullptr;
}

void UActionStatComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UActionStatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TSet<FActiveActionState*> SlotsToRemove;

	for (auto It = ActiveSlots.CreateIterator(); It; ++It)
	{
		FActiveActionState* ActiveSlot = *It;

		if (!ActiveSlot) continue;

		// Cooldown 업데이트
		ActiveSlot->Cooldown = FMath::Max(ActiveSlot->Cooldown - DeltaTime, 0.0f);

		// ReuseDuration 업데이트
		ActiveSlot->ReuseDuration = FMath::Max(ActiveSlot->ReuseDuration - DeltaTime, 0.0f);

		// ReuseDuration이 0이 되었을 때 InstanceIndex 초기화
		if (ActiveSlot->ReuseDuration == 0.0f && ActiveSlot->InstanceIndex != 1)
		{
			//UE_LOG(LogTemp, Log, TEXT("[%s] ReuseDuration expired, resetting InstanceIndex to 1"),
			//	*StaticEnum<EActionSlot>()->GetValueAsString(static_cast<int64>(ActiveSlot->SlotID)));

			ActiveSlot->InstanceIndex = 1;
		}

		if (ActiveSlot->Cooldown == 0.0f && ActiveSlot->ReuseDuration == 0.0f)
		{
			SlotsToRemove.Add(ActiveSlot);
		}
	}

	for (FActiveActionState* Slot : SlotsToRemove)
	{
		ActiveSlots.Remove(Slot);
	}
}

void UActionStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ActiveActionState_Q);
	DOREPLIFETIME(ThisClass, ActiveActionState_E);
	DOREPLIFETIME(ThisClass, ActiveActionState_R);
	DOREPLIFETIME(ThisClass, ActiveActionState_LMB);
	DOREPLIFETIME(ThisClass, ActiveActionState_RMB);

	DOREPLIFETIME(ThisClass, ActionAttributes_Q);
	DOREPLIFETIME(ThisClass, ActionAttributes_E);
	DOREPLIFETIME(ThisClass, ActionAttributes_R);
	DOREPLIFETIME(ThisClass, ActionAttributes_LMB);
	DOREPLIFETIME(ThisClass, ActionAttributes_RMB);
}

void UActionStatComponent::InitActionStatComponent(UDataTable* InDataTable, UStatComponent* InStatComponent)
{
	if (InDataTable == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] InDataTable is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (InStatComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] InStatComponent is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	StatTable = InDataTable;
	StatComponent = InStatComponent;

	InitializeActiveActionState(EActionSlot::Q, ActiveActionState_Q);
	InitializeActiveActionState(EActionSlot::E, ActiveActionState_E);
	InitializeActiveActionState(EActionSlot::R, ActiveActionState_R);
	InitializeActiveActionState(EActionSlot::LMB, ActiveActionState_LMB);
	InitializeActiveActionState(EActionSlot::RMB, ActiveActionState_RMB);
}

void UActionStatComponent::InitializeActiveActionState(EActionSlot SlotID, FActiveActionState& ActiveActionState)
{
	if (!StatTable) return;
	if (!StatComponent.IsValid()) return;

	FName SlotName = NAME_None;
	switch (SlotID)
	{
	case EActionSlot::Q:    SlotName = FName(TEXT("Q"));    break;
	case EActionSlot::E:    SlotName = FName(TEXT("E"));    break;
	case EActionSlot::R:    SlotName = FName(TEXT("R"));    break;
	case EActionSlot::LMB:  SlotName = FName(TEXT("LMB"));  break;
	case EActionSlot::RMB:  SlotName = FName(TEXT("RMB"));  break;
	default:                SlotName = NAME_None;    break;
	}

	const FActionTableRow* StatRow = StatTable->FindRow<FActionTableRow>(SlotName, TEXT(""));
	if (!StatRow)
	{
		return;
	}

	int32 MaxLevel = StatRow->Actions.Num();
	if (!StatRow->Actions.IsValidIndex(0))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Actions array is invalid or empty for ActionName: %s"), ANSI_TO_TCHAR(__FUNCTION__), *SlotName.ToString());
		return;
	}

	const FAction* Action = &StatRow->Actions[0];
	if (!Action)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to get Action at index 0 for ActionName: %s"), ANSI_TO_TCHAR(__FUNCTION__), *SlotName.ToString());
		return;
	}

	const FActionDefinition& ActionDefinition = Action->ActionDefinition;
	if (ActionDefinition.Name.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to get ActionDefinition at index 0 for ActionName: %s"), ANSI_TO_TCHAR(__FUNCTION__), *SlotName.ToString());
		return;
	}

	const TArray<FActionAttributes>& ActionAttributes = Action->ActionAttributes;
	if (ActionAttributes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: ActionAttributes array is empty for ActionName: %s"),
			ANSI_TO_TCHAR(__FUNCTION__), *Action->ActionDefinition.Name.ToString());
		return;
	}

	ActiveActionState.SlotID				= SlotID;
	ActiveActionState.Name					= ActionDefinition.Name;
	ActiveActionState.Description			= ActionDefinition.Description;
	ActiveActionState.MaxLevel				= MaxLevel;
	ActiveActionState.MaxInstance			= ActionAttributes.Num();

	UE_LOG(LogTemp, Log, TEXT("[%s] %s- %s Max Level [%d], Max Instance [%d]"), ANSI_TO_TCHAR(__FUNCTION__), *GetOwner()->GetName(), *ActiveActionState.Name.ToString(), ActiveActionState.MaxLevel, ActiveActionState.MaxInstance);
}

bool UActionStatComponent::InitializeActionAtLevel(EActionSlot SlotID, const int32 InLevel)
{
	switch (SlotID)
	{
	case EActionSlot::Q:
		return InitializeAction(EActionSlot::Q, ActiveActionState_Q, ActionAttributes_Q, InLevel);

	case EActionSlot::E:
		return InitializeAction(EActionSlot::E, ActiveActionState_E, ActionAttributes_E, InLevel);

	case EActionSlot::R:
		return InitializeAction(EActionSlot::R, ActiveActionState_R, ActionAttributes_R, InLevel);

	case EActionSlot::LMB:
		return InitializeAction(EActionSlot::LMB, ActiveActionState_LMB, ActionAttributes_LMB, InLevel);

	case EActionSlot::RMB:
		return InitializeAction(EActionSlot::RMB, ActiveActionState_RMB, ActionAttributes_RMB, InLevel);

	default:
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid SlotID: %d. Please check the input."), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID));
		return false;
	}
}




bool UActionStatComponent::InitializeAction(EActionSlot SlotID, FActiveActionState& ActiveActionState, TArray<FActionAttributes>& ActionAttributesSlot, const int32 InLevel)
{
	if (ActiveActionState.MaxLevel < InLevel)
	{
		return false;
	}

	if (!StatTable) return false;
	if (!StatComponent.IsValid()) return false;

	FName SlotName = *StaticEnum<EActionSlot>()->GetNameStringByValue(static_cast<int64>(SlotID));
	const FActionTableRow* StatRow = StatTable->FindRow<FActionTableRow>(SlotName, TEXT(""));
	if (!StatRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to find StatRow for ActionName: %s"), ANSI_TO_TCHAR(__FUNCTION__), *SlotName.ToString());
		return false;
	}

	if (!StatRow->Actions.IsValidIndex(InLevel - 1))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Actions array index %d is invalid for ActionName: %s"), ANSI_TO_TCHAR(__FUNCTION__), InLevel - 1, *SlotName.ToString());
		return false;
	}

	const FAction* Action = &StatRow->Actions[InLevel - 1];
	if (!Action)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to get Action at index %d for ActionName: %s"), ANSI_TO_TCHAR(__FUNCTION__), InLevel - 1, *SlotName.ToString());
		return false;
	}

	const FActionDefinition& ActionDefinition = Action->ActionDefinition;
	if (ActionDefinition.Name.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to get ActionDefinition at index 0 for ActionName: %s"), ANSI_TO_TCHAR(__FUNCTION__), *SlotName.ToString());
		return false;
	}

	const TArray<FActionAttributes>& ActionAttributes = Action->ActionAttributes;
	if (ActionAttributes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: ActionAttributes array is empty for ActionName: %s"), ANSI_TO_TCHAR(__FUNCTION__), *Action->ActionDefinition.Name.ToString());
		return false;
	}

	int32 CharacterLevel = StatComponent->GetCurrentLevel();

	ActiveActionState.Name					= ActionDefinition.Name;
	ActiveActionState.Description			= ActionDefinition.Description;
	ActiveActionState.CurrentLevel			= InLevel;
	ActiveActionState.MaxInstance			= ActionAttributes.Num();
	ActiveActionState.InstanceIndex			= 1;
	ActiveActionState.bIsUpgradable			= CharacterLevel >= ActionDefinition.RequiredLevel;
	ActiveActionState.MaxCooldown			= ActionAttributes[0].CooldownTime;
	ActiveActionState.MaxReuseDuration		= ActionAttributes[0].ReuseDuration;
	ActiveActionState.ActionType			= ActionDefinition.ActionType;
	ActiveActionState.ActivationType		= ActionDefinition.ActivationType;
	ActiveActionState.CollisionDetection	= ActionDefinition.CollisionDetection;

	ActionAttributesSlot.Empty();
	for (auto& ActionStat : ActionAttributes)
	{
		ActionAttributesSlot.Emplace(ActionStat);
	}

	if (InLevel >= 1 && ActiveActionState.bCanCastAction)
	{
		ClientNotifyActivationChanged(SlotID, true);
	}

	ClientNotifyLevelChanged(SlotID, InLevel);

	return true;
}


const FAction* UActionStatComponent::GetAction(EActionSlot SlotID, const int32 InLevel) const
{
	if (!StatTable) return nullptr;

	FName ActionName = NAME_None;
	switch (SlotID)
	{
	case EActionSlot::Q:    ActionName = TEXT("Q");    break;
	case EActionSlot::E:    ActionName = TEXT("E");    break;
	case EActionSlot::R:    ActionName = TEXT("R");    break;
	case EActionSlot::LMB:  ActionName = TEXT("LMB");  break;
	case EActionSlot::RMB:  ActionName = TEXT("RMB");  break;
	default:                 ActionName = NAME_None;    break;
	}

	// 최대 레벨을 가져올 행
	const FActionTableRow* StatRow = StatTable->FindRow<FActionTableRow>(ActionName, TEXT(""));
	if (!StatRow)
	{
		return nullptr;
	}

	uint8 MaxLevel = StatRow->Actions.Num();
	if (MaxLevel == 0)
	{
		return nullptr;
	}

	uint8 ClampedLevel = FMath::Clamp<uint8>(InLevel - 1, 0, MaxLevel - 1);
	const FAction* Action = &StatRow->Actions[ClampedLevel];
	if (!Action)
	{
		return nullptr;
	}

	return Action;
}

void UActionStatComponent::HandleActionExecution_Implementation(EActionSlot SlotID, float CurrentTime)
{
	switch (SlotID)
	{
	case EActionSlot::None:
		UE_LOG(LogTemp, Error, TEXT("[%s] SlotID cannot be None. Please provide a valid slot."), ANSI_TO_TCHAR(__FUNCTION__));
		return;

	case EActionSlot::Q:
		ExecuteAction(ActiveActionState_Q, ActionAttributes_Q, EActionSlot::Q);
		break;

	case EActionSlot::E:
		ExecuteAction(ActiveActionState_E, ActionAttributes_E, EActionSlot::E);
		break;

	case EActionSlot::R:
		ExecuteAction(ActiveActionState_R, ActionAttributes_R, EActionSlot::R);
		break;
			
	case EActionSlot::LMB:
		ExecuteAction(ActiveActionState_LMB, ActionAttributes_LMB, EActionSlot::LMB);
		break;

	case EActionSlot::RMB:
		ExecuteAction(ActiveActionState_RMB, ActionAttributes_RMB, EActionSlot::RMB);
		break;

	default:
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid SlotID: %d. Please check the input."), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID));
		return;
	}
}

void UActionStatComponent::ExecuteAction(FActiveActionState& ActiveActionState, TArray<FActionAttributes>& ActionAttributesSlot, EActionSlot SlotID)
{
	if (ActiveActionState.Name.IsNone() || ActionAttributesSlot.Num() == 0 || ActiveActionState.MaxInstance <= 0 || ActiveActionState.InstanceIndex < 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid action execution for SlotID %d"), *FString(__FUNCTION__), SlotID);
		return;
	}

	if (!ActionAttributesSlot.IsValidIndex(ActiveActionState.InstanceIndex - 1))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid ActionAttributesSlot index: %d for SlotID %d"), *FString(__FUNCTION__), ActiveActionState.InstanceIndex, SlotID);
		return;
	}

	const auto& ActionAttributes = ActionAttributesSlot[ActiveActionState.InstanceIndex - 1];
	if (ActionAttributes.Name.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ActionAttributesSlot at index %d is invalid for SlotID %d"), *FString(__FUNCTION__), ActiveActionState.InstanceIndex, SlotID);
		return;
	}

	ActiveActionState.ReuseDuration = ActionAttributes.ReuseDuration;

	if (ActiveActionState.MaxInstance == 1)
	{
		ActiveActionState.InstanceIndex = 1; 
	}
	else
	{
		ActiveActionState.InstanceIndex = (ActiveActionState.InstanceIndex % ActiveActionState.MaxInstance) + 1;
	}

	if (ActiveActionState.ReuseDuration > 0)
	{
		ActiveSlots.Add(&ActiveActionState);

		FTimerHandle& TimerHandle = ReuseTimerHandles.FindOrAdd(SlotID);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, SlotID, &ActiveActionState]()
			{
				ClientNotifyReuseTimerChanged(SlotID, ActiveActionState.ReuseDuration, ActiveActionState.MaxReuseDuration);
				if (ActiveActionState.ReuseDuration <= 0.0f)
				{
					ClientNotifyReuseTimerChanged(SlotID, 0.0f, ActiveActionState.MaxReuseDuration);
					FTimerHandle& TimerHandle = ReuseTimerHandles[SlotID];
					GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
				}
			},
			0.05f,
			true,
			0.0f
		);
	}
	else
	{
		if (ReuseTimerHandles.Contains(SlotID))
		{
			GetWorld()->GetTimerManager().ClearTimer(ReuseTimerHandles[SlotID]);
			ReuseTimerHandles.Remove(SlotID);
		}
		ClientNotifyReuseTimerChanged(SlotID, 0.0f, ActiveActionState.MaxReuseDuration);
	}
}




bool UActionStatComponent::IsActionReady(EActionSlot SlotID) const
{
	switch (SlotID)
	{
	case EActionSlot::None:
		UE_LOG(LogTemp, Error, TEXT("[%s] SlotID cannot be None. Please provide a valid slot."), ANSI_TO_TCHAR(__FUNCTION__));
		return false;

	case EActionSlot::Q:
		return (ActiveActionState_Q.CurrentLevel >= 1) && (ActiveActionState_Q.Cooldown <= 0 || ActiveActionState_Q.ReuseDuration > 0) && ActiveActionState_Q.bCanCastAction;

	case EActionSlot::E:
		return (ActiveActionState_E.CurrentLevel >= 1) && (ActiveActionState_E.Cooldown <= 0 || ActiveActionState_E.ReuseDuration > 0) && ActiveActionState_E.bCanCastAction;

	case EActionSlot::R:
		return (ActiveActionState_R.CurrentLevel >= 1) && (ActiveActionState_R.Cooldown <= 0 || ActiveActionState_R.ReuseDuration > 0) && ActiveActionState_R.bCanCastAction;

	case EActionSlot::LMB:
		return (ActiveActionState_LMB.CurrentLevel >= 1) && (ActiveActionState_LMB.Cooldown <= 0 || ActiveActionState_LMB.ReuseDuration > 0) && ActiveActionState_LMB.bCanCastAction;

	case EActionSlot::RMB:
		return (ActiveActionState_RMB.CurrentLevel >= 1) && (ActiveActionState_RMB.Cooldown <= 0 || ActiveActionState_RMB.ReuseDuration > 0) && ActiveActionState_RMB.bCanCastAction;
	default:
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid SlotID: %d. Please check the input."), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID));
		return false;
	}
}


void UActionStatComponent::ActivateActionCooldown_Implementation(EActionSlot SlotID)
{
	if (StatComponent.IsValid() == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] StatComponent is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	FActiveActionState* ActiveActionState = nullptr;
	TArray<FActionAttributes>* StatTables = nullptr;

	const float AttackSpeed = StatComponent->GetAttackSpeed();
	const int ActionHaste = StatComponent->GetAbilityHaste();
	
	switch (SlotID)
	{
	case EActionSlot::None:
		UE_LOG(LogTemp, Error, TEXT("[%s] SlotID cannot be None. Please provide a valid slot."), ANSI_TO_TCHAR(__FUNCTION__));
		return;

	case EActionSlot::Q:
		ActiveActionState = &ActiveActionState_Q;
		StatTables = &ActionAttributes_Q;
		break;
	case EActionSlot::E:
		ActiveActionState = &ActiveActionState_E;
		StatTables = &ActionAttributes_E;
		break;
	case EActionSlot::R:
		ActiveActionState = &ActiveActionState_R;
		StatTables = &ActionAttributes_R;
		break;
	case EActionSlot::LMB:
		ActiveActionState = &ActiveActionState_LMB;
		StatTables = &ActionAttributes_LMB;
		break;
	case EActionSlot::RMB:
		ActiveActionState = &ActiveActionState_RMB;
		StatTables = &ActionAttributes_RMB;
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid SlotID: %d. Please check the input."), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID));
		return;
	}

	if (!StatTables)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] StatTables is null for SlotID %d"), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID));
		return;
	}

	if (!ActiveActionState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ActiveActionState is null for SlotID %d"), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID));
		return;
	}

	if (ActiveActionState->InstanceIndex <= 0)
	{
		FString SlotName = StaticEnum<EActionSlot>()->GetNameStringByValue(static_cast<int64>(ActiveActionState->SlotID));
		UE_LOG(LogTemp, Warning, TEXT("[%s] Action slot [%s] is not initialized yet. InstanceIndex: %d"),
			ANSI_TO_TCHAR(__FUNCTION__), *SlotName, ActiveActionState->InstanceIndex);
		return;
	}

	if (!StatTables->IsValidIndex(ActiveActionState->InstanceIndex -1))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid InstanceIndex %d for SlotID %d"), ANSI_TO_TCHAR(__FUNCTION__), ActiveActionState->InstanceIndex, static_cast<int32>(SlotID));
		return;
	}

	if (EnumHasAnyFlags(SlotID, EActionSlot::LMB))
	{
		ActiveActionState->MaxCooldown = 1.0f / AttackSpeed;
		ActiveActionState->Cooldown = ActiveActionState->MaxCooldown;
	}
	else
	{
		ActiveActionState->MaxCooldown = (*StatTables)[ActiveActionState->InstanceIndex -1].CooldownTime * (100.f / (100.f + ActionHaste));
		ActiveActionState->Cooldown = ActiveActionState->MaxCooldown;
	}

	ActiveSlots.Add(ActiveActionState);

	FTimerHandle& TimerHandle = TimerHandles.FindOrAdd(SlotID);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, SlotID, ActiveActionState]()
		{
			ClientNotifyCooldownChanged(SlotID, ActiveActionState->Cooldown, ActiveActionState->MaxCooldown);
			if (ActiveActionState->Cooldown <= 0.0f)
			{
				FTimerHandle& TimerHandle = TimerHandles[SlotID];
				GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
			}
		}, 
		0.05f, 
		true, 
		0.0f
	);
}


FActiveActionState& UActionStatComponent::GetActiveActionState(EActionSlot SlotID)
{
	FActiveActionState EmptyStruct = FActiveActionState();

	if (SlotID == EActionSlot::None)
	{
		return EmptyStruct;
	}
	else if (SlotID == EActionSlot::Q)
	{
		return ActiveActionState_Q;
	}
	else if (SlotID == EActionSlot::E)
	{
		return ActiveActionState_E;
	}
	else if (SlotID == EActionSlot::R)
	{
		return ActiveActionState_R;
	}
	else if (SlotID == EActionSlot::LMB)
	{
		return ActiveActionState_LMB;
	}
	else if (SlotID == EActionSlot::RMB)
	{
		return ActiveActionState_RMB;
	}

	return EmptyStruct;
}

const FActionAttributes& UActionStatComponent::GetActionAttributes(EActionSlot SlotID) const
{
	// 기본값 반환을 위한 빈 테이블 생성
	static FActionAttributes EmptyTable;
	const FActiveActionState* ActiveActionState = nullptr;
	const TArray<FActionAttributes>* StatTables = nullptr;

	// SlotID에 따라 포인터 설정
	switch (SlotID)
	{
	case EActionSlot::Q:
		ActiveActionState = &ActiveActionState_Q;
		StatTables = &ActionAttributes_Q;
		break;
	case EActionSlot::E:
		ActiveActionState = &ActiveActionState_E;
		StatTables = &ActionAttributes_E;
		break;
	case EActionSlot::R:
		ActiveActionState = &ActiveActionState_R;
		StatTables = &ActionAttributes_R;
		break;
	case EActionSlot::LMB:
		ActiveActionState = &ActiveActionState_LMB;
		StatTables = &ActionAttributes_LMB;
		break;
	case EActionSlot::RMB:
		ActiveActionState = &ActiveActionState_RMB;
		StatTables = &ActionAttributes_RMB;
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid SlotID: %d. Please check the input."), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID));
		return EmptyTable;
	}

	if (ActiveActionState->InstanceIndex <= 0)
	{
		FString SlotName = StaticEnum<EActionSlot>()->GetNameStringByValue(static_cast<int64>(ActiveActionState->SlotID));
		UE_LOG(LogTemp, Warning, TEXT("[%s] Action slot [%s] is not initialized yet. InstanceIndex: %d"),
			ANSI_TO_TCHAR(__FUNCTION__), *SlotName, ActiveActionState->InstanceIndex);
		return EmptyTable;
	}

	// StatTables 및 ArrayIndex 유효성 검사
	if (ActiveActionState && !StatTables->IsValidIndex(ActiveActionState->InstanceIndex - 1))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Valid ActionStatTable not found. Returning default value. SlotID: %d, ArrayIndex: %d"), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID), ActiveActionState->InstanceIndex);
		return EmptyTable;
	}

	return (*StatTables)[ActiveActionState->InstanceIndex - 1];
}



float UActionStatComponent::GetUniqueValue(EActionSlot SlotID, const FName& InKey)
{
	// 능력 정보와 능력 속성 배열에 대한 포인터 정의
	const FActiveActionState* ActiveActionState = nullptr;
	const TArray<FActionAttributes>* StatTables = nullptr;

	// SlotID에 따라 포인터 설정
	switch (SlotID)
	{
	case EActionSlot::Q:
		ActiveActionState = &ActiveActionState_Q;
		StatTables = &ActionAttributes_Q;
		break;
	case EActionSlot::E:
		ActiveActionState = &ActiveActionState_E;
		StatTables = &ActionAttributes_E;
		break;
	case EActionSlot::R:
		ActiveActionState = &ActiveActionState_R;
		StatTables = &ActionAttributes_R;
		break;
	case EActionSlot::LMB:
		ActiveActionState = &ActiveActionState_LMB;
		StatTables = &ActionAttributes_LMB;
		break;
	case EActionSlot::RMB:
		ActiveActionState = &ActiveActionState_RMB;
		StatTables = &ActionAttributes_RMB;
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid SlotID: %d. Please check the input."), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID));
		return 0.0f;
	}

	// ActiveActionState와 ActionStatTables가 유효한지 확인
	if (!ActiveActionState || !StatTables)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid Action info or attributes"), ANSI_TO_TCHAR(__FUNCTION__));
		return 0.f;
	}

	if (ActiveActionState->InstanceIndex <= 0)
	{
		FString SlotName = StaticEnum<EActionSlot>()->GetNameStringByValue(static_cast<int64>(ActiveActionState->SlotID));
		UE_LOG(LogTemp, Warning, TEXT("[%s] Action slot [%s] is not initialized yet. InstanceIndex: %d"),
			ANSI_TO_TCHAR(__FUNCTION__), *SlotName, ActiveActionState->InstanceIndex);
		return 0.f;
	}

	// 인덱스를 제한하고 속성 배열 가져오기
	if ((*StatTables).IsValidIndex(ActiveActionState->InstanceIndex -1) == false)
	{
		return 0.f;
	}

	const TArray<FUniqueAttribute>& Attributes = (*StatTables)[ActiveActionState->InstanceIndex -1].UniqueAttributes;
	for (const auto& ActionAttribute : Attributes)
	{
		if (ActionAttribute.Key.IsEqual(InKey))
		{
			return ActionAttribute.Value;
		}
	}

	return 0.f;
}

float UActionStatComponent::GetUniqueValue(EActionSlot SlotID, const FName& InKey, float DefaultValue)
{
	float Value = GetUniqueValue(SlotID, InKey);
	if (FMath::IsNearlyZero(Value))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Key '%s' not found for SlotID '%d', using default value %f."), ANSI_TO_TCHAR(__FUNCTION__), *InKey.ToString(), static_cast<int32>(SlotID), DefaultValue);
		return DefaultValue;
	}
	return Value;
}


TArray<FActiveActionState*> UActionStatComponent::GetActiveActionStatePtrs()
{
	TArray<FActiveActionState*> OutDetails;

	if (&ActiveActionState_Q != nullptr)
	{
		OutDetails.Add(&ActiveActionState_Q);
	}

	if (&ActiveActionState_E != nullptr)
	{
		OutDetails.Add(&ActiveActionState_E);
	}

	if (&ActiveActionState_R != nullptr)
	{
		OutDetails.Add(&ActiveActionState_R);
	}

	if (&ActiveActionState_LMB != nullptr)
	{
		OutDetails.Add(&ActiveActionState_LMB);
	}

	if (&ActiveActionState_RMB != nullptr)
	{
		OutDetails.Add(&ActiveActionState_RMB);
	}

	return OutDetails;
}

void UActionStatComponent::UpdateUpgradableStatus(EActionSlot SlotID, FActiveActionState& ActiveActionState, int32 InNewCurrentLevel)
{
	if (ActiveActionState.CurrentLevel >= ActiveActionState.MaxLevel)
	{
		ActiveActionState.bIsUpgradable = false;
		ClientNotifyUpgradeStateChanged(SlotID, false);
		return;
	}

	const FAction* Action = GetAction(SlotID, ActiveActionState.CurrentLevel + 1);
	if (!Action)
	{
		return;
	}

	if (Action->ActionDefinition.Name.IsNone())
	{
		return;
	}

	int32 RequiredLevel = Action->ActionDefinition.RequiredLevel;
	if (InNewCurrentLevel >= RequiredLevel)
	{
		ActiveActionState.bIsUpgradable = true;
		ClientNotifyUpgradeStateChanged(SlotID, true);
	}
	else
	{
		ActiveActionState.bIsUpgradable = false;
		ClientNotifyUpgradeStateChanged(SlotID, false);
	}
}

void UActionStatComponent::ServerUpdateUpgradableStatus_Implementation(int32 InOldCurrentLevel, int32 InNewCurrentLevel)
{
	UpdateUpgradableStatus(EActionSlot::Q, ActiveActionState_Q, InNewCurrentLevel);
	UpdateUpgradableStatus(EActionSlot::E, ActiveActionState_E, InNewCurrentLevel);
	UpdateUpgradableStatus(EActionSlot::R, ActiveActionState_R, InNewCurrentLevel);
	UpdateUpgradableStatus(EActionSlot::LMB, ActiveActionState_LMB, InNewCurrentLevel);
	UpdateUpgradableStatus(EActionSlot::RMB, ActiveActionState_RMB, InNewCurrentLevel);
}

void UActionStatComponent::ServerToggleUpgradeStat_Implementation(bool Visibility)
{
	ClientNotifyUpgradeStateChanged(EActionSlot::Q, Visibility);
	ClientNotifyUpgradeStateChanged(EActionSlot::E, Visibility);
	ClientNotifyUpgradeStateChanged(EActionSlot::R, Visibility);
	ClientNotifyUpgradeStateChanged(EActionSlot::LMB, Visibility);
	ClientNotifyUpgradeStateChanged(EActionSlot::RMB, Visibility);
}

void UActionStatComponent::ClientNotifyUpgradeStateChanged_Implementation(EActionSlot SlotID, bool bCanUpgrade)
{
	OnUpgradeStateChanged.Broadcast(SlotID, bCanUpgrade);
}

void UActionStatComponent::ClientNotifyActivationChanged_Implementation(EActionSlot SlotID, bool bIsActivated)
{
	OnActivationChanged.Broadcast(SlotID, bIsActivated);
}

void UActionStatComponent::ClientNotifyLevelChanged_Implementation(EActionSlot SlotID, int InLevel)
{
	OnActionLevelChanged.Broadcast(SlotID, InLevel);
}

void UActionStatComponent::ClientNotifyCooldownChanged_Implementation(EActionSlot SlotID, const float CurrentCooldownTime, const float MaxCooldownTime)
{
	if (OnCooldownTimeChanged.IsBound())
	{
		OnCooldownTimeChanged.Broadcast(SlotID, CurrentCooldownTime, MaxCooldownTime);
	}
}

void UActionStatComponent::ClientNotifyReuseTimerChanged_Implementation(EActionSlot SlotID, const float CurrentReuseTime, const float MaxReuseTime)
{
	if (OnReuseTimeChanged.IsBound())
	{
		OnReuseTimeChanged.Broadcast(SlotID, CurrentReuseTime, MaxReuseTime);
	}
}

void UActionStatComponent::ClientNotifyAlertTextChanged_Implementation(const FString& InString)
{
	if (OnAlertTextChanged.IsBound())
	{
		OnAlertTextChanged.Broadcast(InString);
	}
}
