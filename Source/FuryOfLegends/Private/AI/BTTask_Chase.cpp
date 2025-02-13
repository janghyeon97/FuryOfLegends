#include "AI/BTTask_Chase.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Actor.h"
#include "Plugins/GameLogger.h"

UBTTask_Chase::UBTTask_Chase()
{
    NodeName = TEXT("Chase");
    bNotifyTick = true;
    bTickIntervals = 0.2f;
}

EBTNodeResult::Type UBTTask_Chase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
    if (Result == EBTNodeResult::Failed)
    {
        return EBTNodeResult::Failed;
    }

    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: None] Failed to get AIController."), ANSI_TO_TCHAR(__FUNCTION__));
        return EBTNodeResult::Failed;
    }

    ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
    if (!AICharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: %s] Failed to get AICharacter from AIController's Pawn."), ANSI_TO_TCHAR(__FUNCTION__), *AIController->GetName());
        return EBTNodeResult::Failed;
    }

    ACharacterBase* TargetCharacter = Cast<ACharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
    if (!TargetCharacter || TargetCharacter == AICharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Character: %s] TargetCharacter is NULL. Clearing TargetActorKey."), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName());
        OwnerComp.GetBlackboardComponent()->ClearValue(ABaseAIController::TargetActorKey);
        return EBTNodeResult::Failed;
    }

    // 통합된 Move 함수로 이동 시작
    MoveTo(AIController, TargetCharacter, OwnerComp);

    return EBTNodeResult::InProgress;
}

void UBTTask_Chase::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
    if (!AICharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: %s] Failed to get AICharacter from AIController's Pawn."), ANSI_TO_TCHAR(__FUNCTION__), *AIController->GetName());
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    ACharacterBase* TargetCharacter = Cast<ACharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
    if (!TargetCharacter || TargetCharacter == AICharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Character: %s] Invalid TargetCharacter. Clearing TargetActorKey and finishing Task with Failed."), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName());

        OwnerComp.GetBlackboardComponent()->ClearValue(ABaseAIController::TargetActorKey);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 목표 위치와 현재 위치 간 거리 계산
    const FVector TargetLocation = TargetCharacter->GetActorLocation();
    const float Distance = FVector::Dist2D(AICharacter->GetActorLocation(), TargetLocation);
    const float Range = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::RangeKey);

    // 목표가 사거리 내에 있는 경우
    if (Distance <= Range)
    {
        UPathFollowingComponent* PathFollowingComponent = AIController->GetPathFollowingComponent();
        if (PathFollowingComponent)
        {
            PathFollowingComponent->PauseMove();
        }

        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // 캐릭터의 속도 검사 (거의 멈춘 상태인지 확인)
    const float Speed = AICharacter->GetVelocity().Size();
    const float SpeedThreshold = 10.0f;

    if (Speed <= SpeedThreshold)
    {
        MoveTo(AIController, TargetCharacter, OwnerComp);
    }

    // 경로가 유효하지 않은지 확인
    UPathFollowingComponent* PathFollowingComponent = AIController->GetPathFollowingComponent();
    if (!PathFollowingComponent->HasValidPath())
    {
        MoveTo(AIController, TargetCharacter, OwnerComp);
        return;
    }

    // 목표가 이동 중인 경우 경로 재 설정.
    float PositionChangeThreshold = 50.0f;
    if (TargetCharacter->GetVelocity().Size() > PositionChangeThreshold)
    {
        MoveTo(AIController, TargetCharacter, OwnerComp);
    }
}

void UBTTask_Chase::MoveTo(ABaseAIController* AIController, ACharacterBase* TargetCharacter, UBehaviorTreeComponent& OwnerComp)
{
    const FVector TargetLocation = TargetCharacter->GetActorLocation();
    const float Range = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::RangeKey);

    // AI가 목표 액터로 이동
    EPathFollowingRequestResult::Type MoveResult = AIController->MoveToActor(TargetCharacter, 60.f, true, true, true, 0, true);

    if (MoveResult == EPathFollowingRequestResult::Failed)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[%s][Controller: %s] Moving to Target."), ANSI_TO_TCHAR(__FUNCTION__), *AIController->GetName());
    }
}
