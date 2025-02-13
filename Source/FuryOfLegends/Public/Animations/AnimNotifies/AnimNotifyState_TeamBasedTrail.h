// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState_Trail.h"
#include "AnimNotifyState_TeamBasedTrail.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UAnimNotifyState_TeamBasedTrail : public UAnimNotifyState_Trail
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	UParticleSystem* TrailParticle_Ally;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	UParticleSystem* TrailParticle_Enemy;
};
