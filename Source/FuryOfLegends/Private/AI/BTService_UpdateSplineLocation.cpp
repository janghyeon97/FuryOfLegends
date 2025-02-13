// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_UpdateSplineLocation.h"
#include "Controllers/BaseAIController.h"
#include "Characters/MinionBase.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "DrawDebugHelpers.h"


UBTService_UpdateSplineLocation::UBTService_UpdateSplineLocation()
{
	NodeName = TEXT("Update Next SplineLocation");
	bNotifyTick = true;
    bTickIntervals = 0.1f;
}

void UBTService_UpdateSplineLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] AIController is null, finishing task as failed."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());
	if (!AICharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] AICharacter is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	USplineComponent* Spline = AICharacter->SplineActor->FindComponentByClass<USplineComponent>();
	if (!Spline)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Spline component is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// 현재 목표 위치와 캐릭터의 위치 확인
	FVector LocationAlongSpline = AIController->GetBlackboardComponent()->GetValueAsVector(ABaseAIController::LocationAlongSplineKey);
	FVector CurrentLocation = AICharacter->GetActorLocation();

	float DistanceToNextLocation = FVector::Dist2D(CurrentLocation, LocationAlongSpline);
	if (DistanceToNextLocation <= 100.f)
	{
		UpdateNextSplineLocation(AIController, AICharacter, Spline, DeltaSeconds);
	}
}

void UBTService_UpdateSplineLocation::UpdateNextSplineLocation(ABaseAIController* AIController, AMinionBase* AICharacter, USplineComponent* Spline, float DeltaSeconds)
{
    if (!AIController || !AICharacter || !Spline) return;

    // 이전 프레임의 DistanceAlongSplineKey 가져오기
    float PreviousDistance = AIController->GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::DistanceAlongSplineKey);

    // 현재 Spline에서 가장 가까운 거리 값 찾기
    float ClosestInputKey = Spline->FindInputKeyClosestToWorldLocation(AICharacter->GetActorLocation());
    float ClosestDistance = Spline->GetDistanceAlongSplineAtSplineInputKey(ClosestInputKey);

    // 미니언 속도 및 이동 거리 계산
    const float Speed = AIController->GetBlackboardComponent()->GetValueAsFloat(AIController->MovementSpeedKey);
    const float DeltaDistance = Speed * DeltaSeconds;
    const float SplineMaxDistance = Spline->GetSplineLength();

    // 이동 방향 설정
    float NewDistance = (AICharacter->TeamSide == ETeamSide::Blue)
        ? ClosestDistance + Speed   // 블루팀: 시작(0) → 끝(SplineMaxDistance)
        : ClosestDistance - Speed;  // 레드팀: 끝(SplineMaxDistance) → 시작(0)

    NewDistance = FMath::Clamp(NewDistance, 0.0f, SplineMaxDistance);

    // Spline 상의 새로운 위치 가져오기
    FVector NextLocation = Spline->GetLocationAtDistanceAlongSpline(NewDistance, ESplineCoordinateSpace::World);

    AIController->GetBlackboardComponent()->SetValueAsVector(ABaseAIController::LocationAlongSplineKey, NextLocation);
    AIController->GetBlackboardComponent()->SetValueAsFloat(ABaseAIController::DistanceAlongSplineKey, NewDistance);
}


