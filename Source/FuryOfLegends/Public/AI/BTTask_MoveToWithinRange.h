// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_MoveToWithinRange.generated.h"

class ABaseAIController;
class ACharacterBase;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UBTTask_MoveToWithinRange : public UBTTask_MoveTo
{
	GENERATED_BODY()
	
public:
	UBTTask_MoveToWithinRange();

protected:
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
	void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds);

	void CheckAndRecalculatePath(ABaseAIController* AIController, ACharacterBase* AICharacter, ACharacterBase* TargetCharacter);
	void OnNavMeshUpdated(ANavigationData* NavData);
	void CheckTargetInRange(UBehaviorTreeComponent& OwnerComp, ACharacterBase* AICharacter, ACharacterBase* TargetCharacter);
};
