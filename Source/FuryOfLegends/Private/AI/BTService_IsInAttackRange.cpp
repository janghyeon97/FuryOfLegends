// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_IsInAttackRange.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTService_IsInAttackRange::UBTService_IsInAttackRange()
{
	NodeName = TEXT("Is In AttackRange");
	bNotifyTick = true;
}

void UBTService_IsInAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] AIController is null, finishing task as failed."), ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }

    ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
    if (!AICharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] AICharacter is null."), ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }

    ACharacterBase* TargetCharacter = Cast<ACharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
    if (!TargetCharacter)
    {
        return;
    }

    const float AttackRange = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::RangeKey);
    float DistanceToTarget = AICharacter->GetDistanceTo(TargetCharacter);


    if (DistanceToTarget <= AttackRange)
    {
        // AI 캐릭터의 경로를 중단하고 멈춤
        UPathFollowingComponent* PathFollowingComp = AIController->GetPathFollowingComponent();
        if (PathFollowingComp && PathFollowingComp->GetStatus() != EPathFollowingStatus::Idle)
        {
            PathFollowingComp->AbortMove(*this, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest, FAIRequestID::CurrentRequest, EPathFollowingVelocityMode::Keep);
        }
    }
}
