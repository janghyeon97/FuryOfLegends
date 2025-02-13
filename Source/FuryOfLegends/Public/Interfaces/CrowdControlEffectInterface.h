// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CrowdControlEffectInterface.generated.h"

UINTERFACE(MinimalAPI)
class UCrowdControlEffectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FURYOFLEGENDS_API ICrowdControlEffectInterface
{
	GENERATED_BODY()

public:
    virtual void ApplyEffect(ACharacter* Target, const float InDuration, const float InPercent = 0.0f) = 0;
    virtual void RemoveEffect(ACharacter* Target) = 0;
    virtual void Reset() = 0;

    virtual float GetDuration() const = 0;
    virtual float GetPercent() const = 0;
};
