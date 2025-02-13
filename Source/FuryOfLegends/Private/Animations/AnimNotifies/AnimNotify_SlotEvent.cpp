// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/AnimNotifies/AnimNotify_SlotEvent.h"
#include "Animations/PlayerAnimInstance.h"

void UAnimNotify_SlotEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::Notify(MeshComp, Animation);

    UPlayerAnimInstance* AnimInstance = Cast<UPlayerAnimInstance>(MeshComp->GetAnimInstance());
    if (AnimInstance)
    {
        AnimInstance->AnimNotify_SlotEvent(SlotID, EventID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to trigger SlotID event: Invalid AnimInstance"), *GetNameSafe(this));
    }
}