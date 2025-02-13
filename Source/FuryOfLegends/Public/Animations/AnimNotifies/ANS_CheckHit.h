// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_CheckHit.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UANS_CheckHit : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyTick(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float DeltaSeconds, const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	class AAOSCharacterBase* OwnerCharacter;
	class USkeletalMeshComponent* MeshComp;

	TArray<FHitResult> OutHits;
	TArray<class ACharacterBase*> FinalHitResults;
};
