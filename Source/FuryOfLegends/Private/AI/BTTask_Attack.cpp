// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Attack.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavAreas/NavArea_Null.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
	bNotifyTick = true;
	bTickIntervals = 0.1f;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return;
	}

	ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
	if (!AICharacter)
	{
		return;
	}

	ACharacterBase* TargetCharacter = Cast<ACharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
	if (!TargetCharacter)
	{
		OwnerComp.GetBlackboardComponent()->ClearValue(ABaseAIController::TargetActorKey);
		return;
	}

	const FVector TargetLocation = TargetCharacter->GetActorLocation();
	const float Distance = FVector::Dist2D(AICharacter->GetActorLocation(), TargetLocation);
	const float Range = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::RangeKey);

	if (EnumHasAnyFlags(TargetCharacter->CharacterState, ECharacterState::Death) || EnumHasAnyFlags(TargetCharacter->CharacterState, ECharacterState::Invulnerability))
	{
		OwnerComp.GetBlackboardComponent()->ClearValue(ABaseAIController::TargetActorKey);
		AICharacter->ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::AttackEnded);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	if (EnumHasAnyFlags(AICharacter->CharacterState, ECharacterState::AttackEnded))
	{
		// Å¸°ÙÀÇ Ä¸½¶ ¹Ý°æÀ» Ãß°¡
		float TargetCapsuleRadius = 0.0f;
		if (UCapsuleComponent* TargetCapsule = TargetCharacter->GetCapsuleComponent())
		{
			TargetCapsuleRadius = TargetCapsule->GetScaledCapsuleRadius();
		}

		// AIÀÇ Ä¸½¶ ¹Ý°æµµ °í·Á
		float AICapsuleRadius = 0.0f;
		if (UCapsuleComponent* AICapsule = AICharacter->GetCapsuleComponent())
		{
			AICapsuleRadius = AICapsule->GetScaledCapsuleRadius();
		}

		if (Distance > Range)
		{
			AICharacter->ComboCount = 1;
		}

		AICharacter->ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::AttackEnded);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
	if (Result == EBTNodeResult::Failed)
	{
		return EBTNodeResult::Failed;
	}

	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (::IsValid(AIController) == false)
	{
		return EBTNodeResult::Failed;
	}

	ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
	if (::IsValid(AICharacter) == false)
	{
		return EBTNodeResult::Failed;
	}

	AICharacter->LMB_Executed();
	return EBTNodeResult::InProgress;
}