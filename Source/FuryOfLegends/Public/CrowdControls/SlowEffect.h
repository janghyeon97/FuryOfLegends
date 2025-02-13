// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CrowdControls/CrowdControlEffect.h"
#include "SlowEffect.generated.h"

class ACharacterBase;
class AAOSCharacterBase;
class ABaseAIController;


/**
 *
 */
UCLASS()
class FURYOFLEGENDS_API USlowEffect : public UCrowdControlEffect
{
    GENERATED_BODY()

public:
    virtual void ApplyEffect(ACharacter* InTarget, const float InDuration, const float InPercent = 0.0f) override;
    virtual void RemoveEffect() override;
    virtual void ReturnEffect() override;
    virtual void Reset() override;

    virtual float GetDuration() const override;
    virtual float GetPercent() const override;

private:
    void SetupBaseEffect(ACharacterBase* Character, ACharacter* InTarget, const float InDuration, const float InPercent);
    void ApplyEffectToPlayer(AAOSCharacterBase* Player);
    void ApplyEffectToAI(ABaseAIController* AIController, ACharacterBase* Character);

private:
    struct FActiveSlowEffect
    {
        float SlowPercent;
        float Duration;
        float StartTime;
        FTimerHandle TimerHandle;

        FActiveSlowEffect(float InPercent, float InDuration, float InStartTime)
            : SlowPercent(InPercent), Duration(InDuration), StartTime(InStartTime) {}
    };

    TArray<FActiveSlowEffect> ActiveSlowEffects;
};
