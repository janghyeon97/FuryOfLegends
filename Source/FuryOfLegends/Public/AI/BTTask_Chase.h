// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Chase.generated.h"

struct FAIRequestID;
struct FPathFollowingResult;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UBTTask_Chase : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Chase();

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	void MoveTo(class ABaseAIController* AIController, class ACharacterBase* TargetCharacter, UBehaviorTreeComponent& OwnerComp);
};
