// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_IsActionReady.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "Components/ActionStatComponent.h"

UBTDecorator_IsActionReady::UBTDecorator_IsActionReady()
{
	NodeName = TEXT("Is Ability Ready");
}

bool UBTDecorator_IsActionReady::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

    if (!bResult)
    {
        UE_LOG(LogTemp, Warning, TEXT("Super::CalculateRawConditionValue returned false"));
        return false;
    }

    AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
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

    UActionStatComponent* ActionStatComponent = AICharacter->GetActionStatComponent();
    if (!ActionStatComponent)
    {
        return false;
    }

    //UE_LOG(LogTemp, Error, TEXT("[UBTDecorator_IsActionReady] %s Ability cooldown remaing %f."), *AICharacter->GetName(), ActionStatComponent->Ability_LMB_Info.Cooldown);
    return ActionStatComponent->IsActionReady(EActionSlot::LMB);
}
