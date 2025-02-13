#include "AI/BTTask_MoveAlongSpline.h"
#include "Controllers/BaseAIController.h"
#include "Characters/MinionBase.h"
#include "Components/SplineComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

UBTTask_MoveAlongSpline::UBTTask_MoveAlongSpline()
{
    NodeName = TEXT("Move Along Spline");
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveAlongSpline::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: None] AIController is null, finishing task as failed."), ANSI_TO_TCHAR(__FUNCTION__));
        return EBTNodeResult::Failed;
    }

    AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());
    if (!AICharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: %s] AICharacter is null, finishing task as failed."), ANSI_TO_TCHAR(__FUNCTION__), *AIController->GetName());
        return EBTNodeResult::Failed;
    }

    // Spline을 초기화
    USplineComponent* Spline = AICharacter->SplineActor->FindComponentByClass<USplineComponent>();
    if (!Spline)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Character: %s] Spline component is null."), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName());
        return EBTNodeResult::Failed;
    }

    // 초기 목표 위치 설정 및 이동 시작
    FVector NextLocation;
    MoveToNextLocation(AIController, AICharacter, Spline, NextLocation);

    return EBTNodeResult::InProgress;
}

void UBTTask_MoveAlongSpline::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: None] AIController is null, finishing task as failed."), ANSI_TO_TCHAR(__FUNCTION__));
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());
    if (!AICharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: %s] AICharacter is null, finishing task as failed."), ANSI_TO_TCHAR(__FUNCTION__), *AIController->GetName());
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    USplineComponent* Spline = AICharacter->SplineActor->FindComponentByClass<USplineComponent>();
    if (!Spline)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Character: %s] Spline component is null."), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName());
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector NextLocation = AIController->GetBlackboardComponent()->GetValueAsVector(ABaseAIController::LocationAlongSplineKey);
    FVector CurrentLocation = AICharacter->GetActorLocation();
    float DistanceToNextLocation = FVector::Dist2D(CurrentLocation, NextLocation);

    // 목표 위치에 도달했는지 확인
    if (DistanceToNextLocation < 10.f)
    {
        MoveToNextLocation(AIController, AICharacter, Spline, NextLocation);  // 새로운 목표 위치로 업데이트
    }

    const float Speed = AICharacter->GetVelocity().Size();
    const float SpeedThreshold = 10.0f;

    // 이동하지 않았을 경우에만 경로를 재탐색
    if (Speed <= SpeedThreshold && !AIController->GetPathFollowingComponent()->HasValidPath())
    {
        MoveToNextLocation(AIController, AICharacter, Spline, NextLocation);
    }
}

void UBTTask_MoveAlongSpline::MoveToNextLocation(ABaseAIController* AIController, AMinionBase* AICharacter, USplineComponent* Spline, FVector& NextLocation)
{
    if (!AIController || !Spline)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: %s] AIController or Spline is null."), ANSI_TO_TCHAR(__FUNCTION__), AIController ? *AIController->GetName() : TEXT("None"));
        return;
    }

    UPathFollowingComponent* PathFollowingComp = AIController->GetPathFollowingComponent();
    if (PathFollowingComp && PathFollowingComp->GetStatus() != EPathFollowingStatus::Idle)
    {
        PathFollowingComp->AbortMove(*this, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest, FAIRequestID::CurrentRequest, EPathFollowingVelocityMode::Keep);
    }

    float CurrentSplineDistance = AIController->GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::DistanceAlongSplineKey);
    const float Speed = AIController->GetBlackboardComponent()->GetValueAsFloat(AIController->MovementSpeedKey);
    const float SplineLength = Spline->GetSplineLength();

    float DeltaDistance = Speed * GetWorld()->DeltaTimeSeconds * 200;

    if (EnumHasAnyFlags(AIController->GetPawn<AMinionBase>()->TeamSide, ETeamSide::Blue))
    {
        CurrentSplineDistance += DeltaDistance;  // 블루팀은 앞으로 이동
    }
    else
    {
        CurrentSplineDistance -= DeltaDistance;  // 그 외 팀은 뒤로 이동
    }

    CurrentSplineDistance = FMath::Clamp(CurrentSplineDistance, 0.0f, SplineLength);

    NextLocation = Spline->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);

    AIController->GetBlackboardComponent()->SetValueAsFloat(ABaseAIController::DistanceAlongSplineKey, CurrentSplineDistance);
    AIController->GetBlackboardComponent()->SetValueAsVector(ABaseAIController::LocationAlongSplineKey, NextLocation);
    AIController->MoveToLocation(NextLocation, 200.f, true, true, true, true, 0, true);
}
