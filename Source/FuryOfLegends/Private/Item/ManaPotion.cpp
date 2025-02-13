// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ManaPotion.h"
#include "Game/ArenaPlayerState.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"
#include "Plugins/UniqueCodeGenerator.h"

AManaPotion::AManaPotion()
{
    HealingAmount = 150.f;
    HealingDuration = 15.f;
    AccumulatedHealing = 0.f;

    ElapsedTime = 0.f;
    TotalElapsedTime = 0.f;
    RemainingTime = 0.f;
}


void AManaPotion::Initialize()
{
    HealingAmount = UniqueAttributes.Contains("HealingAmount") ? UniqueAttributes["HealingAmount"] : 0.f;
    HealingDuration = UniqueAttributes.Contains("HealingDuration") ? UniqueAttributes["HealingDuration"] : 0.f;
    RemainingTime = HealingDuration;
}


void AManaPotion::Use(AArenaPlayerState* PlayerState)
{
    if (!::IsValid(PlayerState))
    {
        return;
    }

    APlayerController* PlayerController = PlayerState->GetPlayerController();
    if (!::IsValid(PlayerController))
    {
        return;
    }

    AAOSCharacterBase* PlayerCharacter = PlayerController->GetPawn<AAOSCharacterBase>();
    if (!::IsValid(PlayerCharacter))
    {
        return;
    }

    if (EnumHasAnyFlags(PlayerCharacter->CharacterState, ECharacterState::Death))
    {
        return;
    }

    UStatComponent* PlayerStatComponent = PlayerCharacter->GetStatComponent();
    if (!::IsValid(PlayerStatComponent))
    {
        return;
    }

    EObjectType ObjectType = PlayerCharacter->ObjectType;
    int32 PlayerIndex = PlayerState->GetPlayerIndex();
    ETimerCategory TimerCategory = ETimerCategory::Item;
    uint8 TimerType = static_cast<uint8>(ETimerType::BuffList);
    uint8 ItemType = static_cast<uint8>(ItemCode);

    uint32 UniqueCode = UUniqueCodeGenerator::GenerateUniqueCode(ObjectType, PlayerIndex, TimerCategory, TimerType, ItemType);

    // 한 번에 회복할 체력 계산
    const float TickInterval = 0.5f;
    const int RepeatCount = FMath::CeilToInt(HealingDuration / TickInterval);
    const float IncrementAmount = HealingAmount / RepeatCount;

    // 회복 타이머 콜백: 체력을 일정 시간 간격으로 회복
    auto HealthRegenCallback = [this, WeakStatComponent = TWeakObjectPtr<UStatComponent>(PlayerStatComponent), WeakPlayerState = TWeakObjectPtr<AArenaPlayerState>(PlayerState), TickInterval, IncrementAmount, UniqueCode]()
        {
            if (!WeakStatComponent.IsValid() || !WeakPlayerState.IsValid())
            {
                return;
            }

            ElapsedTime += 0.1f;
            TotalElapsedTime += 0.1f;
            RemainingTime = FMath::Max(0.f, HealingDuration - TotalElapsedTime);
            WeakPlayerState->ClientNotifyRemainingTime(UniqueCode, RemainingTime, TotalElapsedTime);

            if (ElapsedTime >= TickInterval)
            {
                ElapsedTime = 0.f;
                WeakStatComponent->ModifyCurrentMP(IncrementAmount);
                AccumulatedHealing += IncrementAmount;
            }

            // 회복 완료 처리
            if (TotalElapsedTime >= HealingDuration)
            {
                --ConcurrentUses;
                WeakPlayerState->ClientNotifyTimerUpdated(UniqueCode, ConcurrentUses);
                
                if (ConcurrentUses <= 0)
                {
                    WeakPlayerState->ClearTimer(UniqueCode);
                    WeakPlayerState->ClientNotifyRemainingTime(UniqueCode, 0, 0);
                    ActivationState = EItemActivationState::Expired;
                }
                else
                {
                    ElapsedTime = 0.f;
                    TotalElapsedTime = 0.f;
                    RemainingTime = 0.f;
                    AccumulatedHealing = 0.f;
                }
            }
        };

    ConcurrentUses++;
    CurrentStackPerSlot--;

    // 기존 타이머가 없으면 새로운 타이머 설정
    if (!PlayerState->IsTimerActive(UniqueCode))
    {
        ActivationState = EItemActivationState::Active;
        PlayerState->SetTimer(UniqueCode, HealthRegenCallback, 0.1f, true, 0.1f);
    }
    else
    {
        PlayerState->ClientNotifyTimerUpdated(UniqueCode, ConcurrentUses);
    }
}