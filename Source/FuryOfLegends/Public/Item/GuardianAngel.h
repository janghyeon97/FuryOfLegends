// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "GuardianAngel.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API AGuardianAngel : public AItem
{
	GENERATED_BODY()
	
public:
	AGuardianAngel();

	virtual void Initialize() override;
	virtual void Use(AArenaPlayerState* PlayerState) override;
	virtual void BindToPlayer(AAOSCharacterBase* Character) override;

protected:
	void SpawnReviveEffect();

	UFUNCTION()
	void OnParticleSystemFinished(UParticleSystemComponent* PSystem);

	UFUNCTION()
	void ReviveCharacter(bool& bDeath);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components", Meta = (AllowPrivateAccess))
	UParticleSystemComponent* ParticleSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", Meta = (AllowPrivateAccess))
	UParticleSystem* ReviveParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components", Meta = (AllowPrivateAccess))
	UParticleSystem* ReviveEndedParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GuardianAngel", meta = (AllowPrivateAccess))
	float CooldownTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GuardianAngel", meta = (AllowPrivateAccess))
	FTransform Transform;
};
