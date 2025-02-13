// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NPCAnimInstance.generated.h"

DECLARE_DELEGATE(FOnNPCCanNextComboDelegate);

UCLASS()
class FURYOFLEGENDS_API UNPCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

	FOnNPCCanNextComboDelegate OnNPCCanNextCombo;

private:
	/* Character Ref */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "NonPlayerCharacter", Meta = (AllowPrivateAccess))
	TObjectPtr<class ACharacterBase> OwnerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter|Anim", Meta = (AllowPrivateAccess))
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter|Anim", Meta = (AllowPrivateAccess))
	FVector Acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter|Anim", Meta = (AllowPrivateAccess))
	float CurrentSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter|Anim", Meta = (AllowPrivateAccess))
	uint8 bIsAccelerating : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter|Anim", Meta = (AllowPrivateAccess))
	uint8 bIsFalling : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter|Anim", Meta = (AllowPrivateAccess))
	uint8 bShouldMove : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter|Anim", Meta = (AllowPrivateAccess))
	uint8 bIsDead : 1;
};
