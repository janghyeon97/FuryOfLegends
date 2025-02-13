// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_SwitchNavArea.h"
#include "AIController.h"
#include "Characters/CharacterBase.h"
#include "NavigationSystem.h"
#include "NavAreas/NavArea.h"
#include "NavAreas/NavArea_Null.h"
#include "NavAreas/NavArea_Obstacle.h"

UBTTask_SwitchNavArea::UBTTask_SwitchNavArea()
{
	NodeName = TEXT("Switch NavArea");
	NavAreaType = ENavAreaType::Default;
}

EBTNodeResult::Type UBTTask_SwitchNavArea::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (::IsValid(AIController) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UBTTask_SwitchNavArea::ExecuteTask] Failed to get AIController"));
		return EBTNodeResult::Failed;
	}

	ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
	if (!::IsValid(AICharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("[UBTTask_SwitchNavArea::ExecuteTask] Failed to get AICharacter from AIController's Pawn"));
		return EBTNodeResult::Failed;
	}

	// NavArea Ÿ���� �����ϴ� ����
	switch (NavAreaType)
	{
	case ENavAreaType::Default:
		// NavArea_Default�� ���� (�⺻��)
		AICharacter->ChangeNavModifierAreaClass(UNavArea::StaticClass());
		break;

	case ENavAreaType::Null:
		// NavArea_Null�� ����
		AICharacter->ChangeNavModifierAreaClass(UNavArea_Null::StaticClass());
		break;

	case ENavAreaType::Obstacle:
		// NavArea_Obstacle�� ���� (��ֹ��� ó��)
		AICharacter->ChangeNavModifierAreaClass(UNavArea_Obstacle::StaticClass());
		break;

	default:
		UE_LOG(LogTemp, Warning, TEXT("Invalid NavAreaType"));
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Succeeded;
}
