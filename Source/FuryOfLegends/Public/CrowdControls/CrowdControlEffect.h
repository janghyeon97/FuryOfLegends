// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CrowdControlEffect.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UCrowdControlEffect : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void ApplyEffect(ACharacter* InTarget, const float InDuration, const float InPercent = 0.0f);
	virtual void RemoveEffect();
	virtual void ReturnEffect();
	virtual void Reset();

	virtual float GetDuration() const;
	virtual float GetPercent() const;

protected:
	UPROPERTY(EditAnywhere, Category = "Attributes")
	ACharacter* Target;

	UPROPERTY(EditAnywhere, Category = "Attributes")
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, Category = "Attributes")
	float Percent = 0.f;

	FTimerHandle TimerHandle;
};
