// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CrowdControls/CrowdControlEffect.h"
#include "StunEffect.generated.h"

struct FActiveActionState;
class ACharacterBase;
class AAOSCharacterBase;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UStunEffect : public UCrowdControlEffect
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
	void ApplyEffectToAI(ACharacterBase* AICharacter);

	void RemoveBaseEffect(ACharacterBase* Character);
	void RemoveEffectFromPlayer(AAOSCharacterBase* Player);
	void RemoveEffectFromAI(ACharacterBase* AICharacter);


private:
	TArray<FActiveActionState*> DisabledAbilities;
};
