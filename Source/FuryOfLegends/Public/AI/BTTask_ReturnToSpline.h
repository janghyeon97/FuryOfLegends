// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ReturnToSpline.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UBTTask_ReturnToSpline : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_ReturnToSpline();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
    void MoveToClosestSplinePoint(UBehaviorTreeComponent& OwnerComp, class ABaseAIController* AIController, class AMinionBase* AICharacter, class USplineComponent* Spline);
};
