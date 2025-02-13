// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/MinionAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"
#include "Characters/MinionBase.h"
#include "Components/StatComponent.h"
#include "Components/ActionStatComponent.h"
#include "Components/SplineComponent.h"


AMinionAIController::AMinionAIController()
{
	static ConstructorHelpers::FObjectFinder<UBlackboardData> BLACKBOARD (TEXT("/Game/FuryOfLegends/AI/Minion/BB_Minion.BB_Minion"));
	if (BLACKBOARD.Succeeded()) BlackboardDataAsset = BLACKBOARD.Object;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BEHAVIORTREE(TEXT("/Game/FuryOfLegends/AI/Minion/BT_Minion.BT_Minion"));
	if (BEHAVIORTREE.Succeeded()) BehaviorTree = BEHAVIORTREE.Object;
}

void AMinionAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AMinionAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AMinionAIController::BeginAI(APawn* InPawn)
{
    if (!BlackboardDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] [Controller: %s] [Reason: BlackboardDataAsset is null]"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
        return;
    }

    UBlackboardComponent* BlackboardComponent = Cast<UBlackboardComponent>(Blackboard);
    if (!BlackboardComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] [Controller: %s] [Reason: BlackboardComponent is null]"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
        return;
    }

    if (!UseBlackboard(BlackboardDataAsset, BlackboardComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] [Controller: %s] [Reason: Failed to use Blackboard]"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
        return;
    }

    AMinionBase* Minion = Cast<AMinionBase>(InPawn);
    if (!Minion)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] [Controller: %s] [Reason: Failed to cast InPawn to AMinionBase]"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
        return;
    }

    const UStatComponent* StatComponent = Minion->GetStatComponent();
    if (!StatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] [Controller: %s] [Character: %s] [Reason: StatComponent is null]"), ANSI_TO_TCHAR(__FUNCTION__), *GetName(), *Minion->GetName());
        return;
    }

    const UActionStatComponent* ActionStatComponent = Minion->GetActionStatComponent();
    if (!ActionStatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] [Controller: %s] [Character: %s] [Reason: ActionStatComponent is null]"), ANSI_TO_TCHAR(__FUNCTION__), *GetName(), *Minion->GetName());
        return;
    }

    const float Range = ActionStatComponent->GetActionAttributes(EActionSlot::LMB).Range;
    const float Speed = StatComponent->GetMovementSpeed();
    const float ChaseThreshold = Minion->ChaseThreshold;

    BlackboardComponent->SetValueAsFloat(RangeKey, Range > 0 ? Range : 280.f);
    BlackboardComponent->SetValueAsFloat(MovementSpeedKey, Speed > 0 ? Speed : 300.f);
    BlackboardComponent->SetValueAsFloat(ChaseThresholdKey, ChaseThreshold);

    // Spline ¼³Á¤
    USplineComponent* Spline = Minion->SplineActor->FindComponentByClass<USplineComponent>();
    if (Spline)
    {
        float SplineDistanceKey = Minion->TeamSide == ETeamSide::Blue ? 0 : Spline->GetSplineLength();
        float TargetDistance = SplineDistanceKey + (Minion->TeamSide == ETeamSide::Blue ? 300 : -300);
        FVector SplineStartLocation = Spline->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
        BlackboardComponent->SetValueAsVector(LocationAlongSplineKey, SplineStartLocation);
        BlackboardComponent->SetValueAsFloat(DistanceAlongSplineKey, TargetDistance);
    }

    bool bRunSucceed = RunBehaviorTree(BehaviorTree);
}