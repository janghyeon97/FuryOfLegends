// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdControls/SlowEffect.h"
#include "Characters/CharacterBase.h"
#include "Components/StatComponent.h"
#include "CrowdControls/CrowdControlManager.h"
#include "GameFramework/CharacterMovementComponent.h"

void USlowEffect::ApplyEffect(ACharacter* InTarget, const float InDuration, const float InPercent)
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

    UStatComponent* StatComponent = Character->GetStatComponent();
    if (!StatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: StatComponent is null for the target."));
        return;
    }

    // 현재 적용된 가장 강력한 둔화 효과를 계산
    float MaxSlowPercent = Percent;
    Target = InTarget;

    // 새로운 효과가 더 강력한 경우, 차이를 계산하여 이동 속도에 반영
    if (FMath::Abs(InPercent) > FMath::Abs(MaxSlowPercent))
    {
        float Difference = InPercent - MaxSlowPercent;
        Percent = InPercent;

        // 이동 속도에 차이를 반영
        StatComponent->ModifyAccumulatedPercentMovementSpeed(-Difference);
        EnumAddFlags(Character->CrowdControlState, ECrowdControl::Slow);

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
    }
    else
    {
        // 새로운 효과가 더 약한 경우, 배열에만 추가하고 나중에 처리
        FActiveSlowEffect NewEffect(InPercent, InDuration, GetWorld()->GetTimeSeconds());
        ActiveSlowEffects.Add(NewEffect);
    }
}

void USlowEffect::RemoveEffect()
{
    if (!Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Target is null."));
        return;
    }

    UWorld* World = Target->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyEffect failed: Target's GetWorld() returned null."));
        return;
    }

    ACharacterBase* Character = Cast<ACharacterBase>(Target);
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Target is not a valid ACharacterBase."));
        return;
    }

    UStatComponent* StatComponent = Character->GetStatComponent();
    if (!StatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: StatComponent is null for the target."));
        return;
    }

    // 만료된 효과를 배열에서 제거하고, 가장 강력한 효과를 찾음
    float CurrentTime = World->GetTimeSeconds();
    ActiveSlowEffects.RemoveAll([CurrentTime](const FActiveSlowEffect& Effect)
        {
            return (CurrentTime - Effect.StartTime) >= Effect.Duration;
        });

    // 남아있는 둔화 효과 중 가장 강력한 효과를 계산
    float MaxSlowPercent = 0.f;
    for (const FActiveSlowEffect& Effect : ActiveSlowEffects)
    {
        if (FMath::Abs(Effect.SlowPercent) > FMath::Abs(MaxSlowPercent))
        {
            MaxSlowPercent = Effect.SlowPercent;
        }
    }

    // 현재 효과와 비교하여 차이만큼 이동 속도 조정
    float Difference = MaxSlowPercent - Percent;
    StatComponent->ModifyAccumulatedPercentMovementSpeed(-Difference);

    // 새로운 효과로 Percent 값 갱신
    Percent = MaxSlowPercent;

    // 만료된 효과가 있으면 타이머를 새로 설정
    if (MaxSlowPercent != 0.f)
    {
        float NextExpiration = FLT_MAX;
        for (const FActiveSlowEffect& Effect : ActiveSlowEffects)
        {
            float ExpirationTime = Effect.StartTime + Effect.Duration - CurrentTime;
            if (ExpirationTime < NextExpiration)
            {
                NextExpiration = ExpirationTime;
            }
        }

        World->GetTimerManager().SetTimer(
            TimerHandle,
            [this]()
            {
                this->RemoveEffect();
            },
            NextExpiration,
            false
        );
    }
    else
    {
        // 모든 효과가 만료되면 Percent 초기화
        Percent = 0.f;
        EnumRemoveFlags(Character->CrowdControlState, ECrowdControl::Slow);
        ReturnEffect();
    }
}

void USlowEffect::ReturnEffect()
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
    if (Character->ActiveEffects.Remove(ECrowdControl::Slow) > 0)
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
            CrowdControlManager->ReturnEffect(ECrowdControl::Slow, this);
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

void USlowEffect::Reset()
{
    ActiveSlowEffects.Empty();
    Duration = 0.0f;
    Percent = 0.0f;
}

float USlowEffect::GetDuration() const
{
    return Duration;
}

float USlowEffect::GetPercent() const
{
    return Percent;
}