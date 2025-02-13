// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_MoveToWithinRange.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h" 

UBTTask_MoveToWithinRange::UBTTask_MoveToWithinRange()
{
    NodeName = TEXT("Move To Within Range");
    bNotifyTick = true;
    bTickIntervals = 0.2f;
}

EBTNodeResult::Type UBTTask_MoveToWithinRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
    if (Result == EBTNodeResult::Failed)
    {
        return EBTNodeResult::Failed;
    }

    return EBTNodeResult::InProgress;
}

void UBTTask_MoveToWithinRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Character: None] AIController is NULL. Finishing Task with Failed."), ANSI_TO_TCHAR(__FUNCTION__));
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
    if (::IsValid(AICharacter) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: %s] Failed to get AICharacter from AIController's Pawn."), ANSI_TO_TCHAR(__FUNCTION__), *AIController->GetName());
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    UBlackboardComponent* BlackboardComponenet = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComponenet)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] %s BlackboardComponenet is null!"), ANSI_TO_TCHAR(__FUNCTION__), *AIController->GetName());
        return;
    }

    ACharacterBase* TargetCharacter = Cast<ACharacterBase>(BlackboardComponenet->GetValueAsObject(ABaseAIController::TargetActorKey));
    if (::IsValid(TargetCharacter) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Character: %s] Invalid TargetCharacter. Clearing TargetActorKey and finishing Task with Failed."), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName());

        BlackboardComponenet->ClearValue(ABaseAIController::TargetActorKey);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // 경로 재계산 및 거리 확인
    //CheckAndRecalculatePath(AIController, AICharacter, TargetCharacter);
    CheckTargetInRange(OwnerComp, AICharacter, TargetCharacter);
}

void UBTTask_MoveToWithinRange::CheckTargetInRange(UBehaviorTreeComponent& OwnerComp, ACharacterBase* AICharacter, ACharacterBase* TargetCharacter)
{
    if (!AICharacter || !TargetCharacter) return;

    UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComponent) return;

    // 타겟과 AI의 위치 및 방향 계산
    const FVector CharacterLocation = AICharacter->GetActorLocation();
    const FVector TargetLocation = TargetCharacter->GetActorLocation();
    const FVector Direction = (TargetLocation - CharacterLocation).GetSafeNormal();

    // 캡슐 반경 고려
    float TargetCapsuleRadius = TargetCharacter->GetCapsuleComponent() ? TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius() : 0.0f;
    float AICapsuleRadius = AICharacter->GetCapsuleComponent() ? AICharacter->GetCapsuleComponent()->GetScaledCapsuleRadius() : 0.0f;

    // 조정된 목표 위치 계산 (타겟 반경만큼 거리 유지)
    const FVector AdjustedTargetLocation = TargetLocation - Direction * TargetCapsuleRadius;
    const float Range = BlackboardComponent->GetValueAsFloat(ABaseAIController::RangeKey);
    const float Distance = FVector::Dist2D(CharacterLocation, AdjustedTargetLocation);

    // 블랙보드 값 업데이트
    BlackboardComponent->SetValueAsFloat(ABaseAIController::DistanceToTargetKey, Distance);
    BlackboardComponent->SetValueAsVector(ABaseAIController::TargetLocationKey, AdjustedTargetLocation);

    // 디버깅 시각화
    //DrawDebugSphere(GetWorld(), AdjustedTargetLocation, 20.0f, 64, FColor::Red, false, 0.1f, 0, 2.0f);
    //DrawDebugCircle(GetWorld(), CharacterLocation, Range, 64, FColor::Green, false, 0.1f, 0, 2.0f, FVector(1, 0, 0), FVector(0, 1, 0), false);

    // 사거리 내 도달 여부 확인
    if (Distance <= Range)
    {
        if (UPathFollowingComponent* PathFollowingComponent = Cast<AAIController>(AICharacter->GetController())->GetPathFollowingComponent())
        {
            PathFollowingComponent->PauseMove();
        }

        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

void UBTTask_MoveToWithinRange::CheckAndRecalculatePath(ABaseAIController* AIController, ACharacterBase* AICharacter, ACharacterBase* TargetCharacter)
{
    const FVector Velocity = AICharacter->GetVelocity();
    const float Speed = Velocity.Size();
    const float MinSpeedThreshold = 5.0f;  // 거의 멈춘 상태로 간주할 최소 속도

    if (Speed < MinSpeedThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] AI speed too low. Recalculating path."), ANSI_TO_TCHAR(__FUNCTION__));
        AIController->MoveToActor(TargetCharacter);  // 속도가 너무 느리면 재계산
    }
}