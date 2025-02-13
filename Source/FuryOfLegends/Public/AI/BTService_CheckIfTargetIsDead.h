// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckIfTargetIsDead.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UBTService_CheckIfTargetIsDead : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_CheckIfTargetIsDead();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
