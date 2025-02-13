// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/AnimNotifies/ANS_CheckHit.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"
#include "Engine/DamageEvents.h"
#include "CollisionQueryParams.h"

void UANS_CheckHit::NotifyBegin(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComponent, Animation, TotalDuration, EventReference);

	if (::IsValid(MeshComponent))
	{
		MeshComp = MeshComponent;
		OwnerCharacter = Cast<AAOSCharacterBase>(MeshComponent->GetOwner());
		if (::IsValid(OwnerCharacter) == false)
		{
			UE_LOG(LogTemp, Log, TEXT("OwnerCharacter is not vaild"));
		}
	}
}

void UANS_CheckHit::NotifyTick(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float DeltaSeconds, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComponent, Animation, DeltaSeconds, EventReference);

	FVector SwordBase	= MeshComp->GetSocketLocation("Sword_Base");
	FVector SwordMid	= MeshComp->GetSocketLocation("Sword_Mid");
	FVector SwordTip	= MeshComp->GetSocketLocation("Sword_Tip");

	FQuat SwordQuat;

	FVector Vec = SwordBase - SwordTip;
	

	if (::IsValid(OwnerCharacter))
	{
		FVector CharacterForwadVector = OwnerCharacter->GetActorForwardVector();

		FCollisionQueryParams params(NAME_None, false, OwnerCharacter);

		bool bResult = MeshComp->GetWorld()->SweepMultiByChannel(
			OutHits,
			SwordBase,
			SwordBase * CharacterForwadVector + 200,
			FRotationMatrix::MakeFromX(CharacterForwadVector).ToQuat(),
			ECC_GameTraceChannel5,
			FCollisionShape::MakeSphere(50.f),
			params
		);
	}
}

void UANS_CheckHit::NotifyEnd(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComponent, Animation, EventReference);

}
