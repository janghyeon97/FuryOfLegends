// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_TurnToTarget.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_TurnToTarget::UBTTask_TurnToTarget()
{
	NodeName = TEXT("TurnToTarget");
}

EBTNodeResult::Type UBTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

    if (Result == EBTNodeResult::Failed)
    {
        return Result;
    }

    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (::IsValid(AIController) == false)
    {
        return Result = EBTNodeResult::Failed;
    }

    ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
    if (::IsValid(AICharacter) == false)
    {
        return Result = EBTNodeResult::Failed;
    }

    ACharacterBase* TargetCharacter = Cast<ACharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
    if (::IsValid(TargetCharacter) == false)
    {
        return Result = EBTNodeResult::Failed;
    }


    FVector LookVector = TargetCharacter->GetActorLocation() - AICharacter->GetActorLocation();
    LookVector.Z = 0.f;

    FRotator ToTargetRotator = FRotationMatrix::MakeFromX(LookVector).Rotator();
    AICharacter->SetActorRelativeRotation(FMath::RInterpTo(AICharacter->GetActorRotation(), ToTargetRotator, GetWorld()->GetDeltaSeconds(), 2.f));

    return Result = EBTNodeResult::Succeeded;
}
