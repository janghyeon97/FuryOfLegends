// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_CheckDistance.h"
#include "Controllers/BaseAIController.h"
#include "Characters/MinionBase.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Plugins/GameLogger.h"
#include "NavigationSystem.h"


UBTService_CheckDistance::UBTService_CheckDistance()
{
    NodeName = TEXT("Check Distance");
    Interval = 0.1f;
}

void UBTService_CheckDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    AMinionBase* AICharacter = AIController ? AIController->GetPawn<AMinionBase>() : nullptr;

    if (!AIController || !AICharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] [Character: None] AIController or AIPawn is null."), ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }

    // AICharacter�� ���� ��ġ�� Spline �󿡼� ���� ����� ���� ã��, �ش� �Ÿ��� ������� TargetActorKey�� ������Ʈ
    USplineComponent* Spline = AICharacter->SplineActor->FindComponentByClass<USplineComponent>();
    AActor* TargetActor = Cast<AActor>(AIController->GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));

    if (Spline && TargetActor)
    {
        // AICharacter�� ���� ��ġ�� Spline �󿡼� ���� ����� ��ġ�� ã��
        FVector AICharacterLocation = AICharacter->GetActorLocation();
        FVector ClosestPointOnSpline = Spline->FindLocationClosestToWorldLocation(AICharacterLocation, ESplineCoordinateSpace::World);

        // AICharacter�� Spline ���� ���� ����� �� ������ 2D �Ÿ��� ���
        float DistanceToClosestPoint = FVector::Dist2D(AICharacterLocation, ClosestPointOnSpline);
        float MaxAllowedChaseDistance = AIController->GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::ChaseThresholdKey);

        // ���� �Ÿ��� �ִ� ��� �Ÿ��� �Ѵ� ���, TargetActorKey�� Ŭ����
        if (DistanceToClosestPoint > MaxAllowedChaseDistance)
        {
            AIController->GetBlackboardComponent()->ClearValue(ABaseAIController::TargetActorKey);
            UE_LOG(LogTemp, Warning, TEXT("[%s] [Character: %s] Cleared TargetActorKey due to exceeding MaxChaseDistance. Current Distance: %f."), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName(), DistanceToClosestPoint);
        }
    }
}



