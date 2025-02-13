// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_CheckHit.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UAN_CheckHit : public UAnimNotify
{
	GENERATED_BODY()

private:
	virtual void Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference);

private:
	UPROPERTY()
	TWeakObjectPtr<class AAOSCharacterBase> OwnerCharacter;
};
