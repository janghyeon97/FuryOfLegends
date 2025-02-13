// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_CheckIfTargetIsDead.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_CheckIfTargetIsDead::UBTService_CheckIfTargetIsDead()
{
	NodeName = TEXT("Check If Target is Dead");
	Interval = 0.1f;
}

void UBTService_CheckIfTargetIsDead::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

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

    if (OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey))
    {
        return;
    }

    ACharacterBase* TargetCharacter = Cast<ACharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
    if (!TargetCharacter)
    {
        return;
    }

    if (EnumHasAnyFlags(TargetCharacter->CharacterState, ECharacterState::Death))
    {
        OwnerComp.GetBlackboardComponent()->ClearValue(ABaseAIController::TargetActorKey);
    }
}
