// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsActionReady.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UBTDecorator_IsActionReady : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_IsActionReady();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
