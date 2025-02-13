// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveAlongSpline.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UBTTask_MoveAlongSpline : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_MoveAlongSpline();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
    // 목표 위치를 업데이트하고 이동하는 함수
    void MoveToNextLocation(class ABaseAIController* AIController, class AMinionBase* AICharacter, class USplineComponent* Spline, FVector& NextLocation);
};
