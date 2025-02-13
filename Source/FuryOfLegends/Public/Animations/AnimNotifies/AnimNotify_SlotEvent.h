// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Structs/ActionData.h"
#include "AnimNotify_SlotEvent.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UAnimNotify_SlotEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlotID")
	EActionSlot SlotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlotID")
	int32 EventID;
};
