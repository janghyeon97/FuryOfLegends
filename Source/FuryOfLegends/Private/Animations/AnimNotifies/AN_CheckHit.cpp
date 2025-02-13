// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/AnimNotifies/AN_CheckHit.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"
#include "Engine/DamageEvents.h"
#include "CollisionQueryParams.h"

void UAN_CheckHit::Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComponent, Animation, EventReference);

	if (::IsValid(MeshComponent) == true)
	{
		OwnerCharacter = Cast<AAOSCharacterBase>(MeshComponent->GetOwner());
		if (::IsValid(OwnerCharacter.Get()))
		{
			OwnerCharacter->LMB_CheckHit();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OwnerCharacter is not vaild"))
		}
	}
}
