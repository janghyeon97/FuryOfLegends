// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_IsInAttackRange.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"


UBTDecorator_IsInAttackRange::UBTDecorator_IsInAttackRange()
{
	NodeName = TEXT("Is In AttackRange");
}

bool UBTDecorator_IsInAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

    if (!bResult)
    {
        UE_LOG(LogTemp, Warning, TEXT("Super::CalculateRawConditionValue returned false"));
        return false;
    }

    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get AIController"));
        return false;
    }

    ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
    if (!AICharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get AICharacter from AIController's Pawn"));
        return false;
    }

    ACharacterBase* TargetCharacter = Cast<ACharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
    if (!TargetCharacter)
    {
        return false;
    }

    const float AttackRange = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::RangeKey);
    float DistanceToTarget = AICharacter->GetDistanceTo(TargetCharacter);

    bResult = (DistanceToTarget <= AttackRange);

    return bResult;
}