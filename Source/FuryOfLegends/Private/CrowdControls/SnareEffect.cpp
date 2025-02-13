// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdControls/SnareEffect.h"
#include "Characters/CharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/ActionStatComponent.h"
#include "CrowdControls/CrowdControlManager.h"
#include "GameFramework/CharacterMovementComponent.h"


void USnareEffect::ApplyEffect(ACharacter* InTarget, const float InDuration, const float InPercent)
{
    if (!InTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is null."));
        return;
    }

    UWorld* World = InTarget->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyEffect failed: Target's GetWorld() returned null."));
        return;
    }

    ACharacterBase* Character = Cast<ACharacterBase>(InTarget);
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is not a valid ACharacterBase."));
        return;
    }

    UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
    if (MovementComponent)
    {
        MovementComponent->StopMovementImmediately();
        Character->ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Move);
        EnumAddFlags(Character->CrowdControlState, ECrowdControl::Snare);
    }

    Target = InTarget;
    Duration = InDuration;
    Percent = InDuration;

    World->GetTimerManager().ClearTimer(TimerHandle);
    World->GetTimerManager().SetTimer(
        TimerHandle,
        [this]()
        {
            this->RemoveEffect();
        },
        InDuration,
        false
    );

    UActionStatComponent* ActionStatComponent = Character->GetActionStatComponent();
    if (!ActionStatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: ActionStatComponent is null for the target."));
        return;
    }

    // ActiveAbilityStates 는 모든 스킬 정보를 담은 배열
    TArray<FActiveActionState*> ActiveAbilityStates = ActionStatComponent->GetActiveActionStatePtrs();
    if (ActiveAbilityStates.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Abilities array is empty."));
        return;
    }

    for (FActiveActionState* ActiveAbilityState : ActiveAbilityStates)
    {
        if (ActiveAbilityState == nullptr)
        {
            continue;
        }

        if (ActiveAbilityState->Name.IsNone())
        {
            continue;
        }

        if (ActiveAbilityState->ActionType == EActionType::Cleanse)
        {
            continue;
        }

        if (ActiveAbilityState->ActionType == EActionType::Dash || ActiveAbilityState->ActionType == EActionType::Blink)
        {
            ActiveAbilityState->bCanCastAction = false; // 스킬을 사용할 수 없도록 설정
            ActiveAbilityState->ActiveCrowdControlCount++;
            ActionStatComponent->ClientNotifyActivationChanged(ActiveAbilityState->SlotID, false);
            DisabledAbilities.Add(ActiveAbilityState);
            UE_LOG(LogTemp, Log, TEXT("%s skill has been disabled due to Snare effect."), *ActiveAbilityState->Name.ToString());
        }
    }
}

void USnareEffect::RemoveEffect()
{
    if (!Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is null."));
        return;
    }

    ACharacterBase* Character = Cast<ACharacterBase>(Target);
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is not a valid ACharacterBase."));
        return;
    }

    UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: MovementComponent is null for the target."));
        return;
    }

    UActionStatComponent* ActionStatComponent = Character->GetActionStatComponent();
    if (!ActionStatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: ActionStatComponent is null for the target."));
        return;
    }

    EnumRemoveFlags(Character->CrowdControlState, ECrowdControl::Snare);
    Character->ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::Move);

    // Abilities 배열의 크기와 유효성 검사
    if (DisabledAbilities.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Abilities array is empty."));
        return;
    }

    for (FActiveActionState* ActiveAbilityState : DisabledAbilities)
    {
        if (ActiveAbilityState == nullptr)
        {
            continue;
        }

        ActiveAbilityState->ActiveCrowdControlCount--;

        // 레벨이 1 이상인 경우 활성화 알림 전송
        if (ActiveAbilityState->ActiveCrowdControlCount == 0 && ActiveAbilityState->CurrentLevel >= 1)
        {
            ActiveAbilityState->bCanCastAction = true;
            ActionStatComponent->ClientNotifyActivationChanged(ActiveAbilityState->SlotID, true);
            UE_LOG(LogTemp, Log, TEXT("%s skill has been re-enabled after Snare effect."), *ActiveAbilityState->Name.ToString());
        }
    }

    DisabledAbilities.Empty();
    ReturnEffect();
}

void USnareEffect::ReturnEffect()
{
    if (!::IsValid(Target))
    {
        UE_LOG(LogTemp, Warning, TEXT("ReturnEffect failed: Target is null."));
        return;
    }

    ACharacterBase* Character = Cast<ACharacterBase>(Target);
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is not a valid ACharacterBase."));
        return;
    }

    // ActiveEffects에서 효과 제거
    if (Character->ActiveEffects.Remove(ECrowdControl::Snare) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Removed Stun effect from ActiveEffects."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect: ActiveEffects does not contain Stun effect."));
    }

    // 효과 객체를 풀로 반환
    if (UCrowdControlManager* CrowdControlManager = UCrowdControlManager::Get())
    {
        if (::IsValid(CrowdControlManager))
        {
            CrowdControlManager->ReturnEffect(ECrowdControl::Stun, this);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Retrieved CrowdControlManager instance is invalid."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Could not retrieve UCrowdControlManager instance."));
    }
}

void USnareEffect::Reset()
{
    DisabledAbilities.Empty();
    Duration = 0.0f;
    Percent = 0.0f;
}



float USnareEffect::GetDuration() const
{
    return 0.0f;
}

float USnareEffect::GetPercent() const
{
    return 0.0f;
}
