// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Props/ArrowBase.h"
#include "PiercingArrow.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API APiercingArrow : public AArrowBase
{
	GENERATED_BODY()

public:
	APiercingArrow();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnHitWorld(const FHitResult& HitResult) override;
	virtual void OnHitCharacter(const FHitResult& HitResult) override;
	virtual void OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayImpactEffects(ECollisionChannel CollisionChannel, const FVector HitLocation, const int32 CurrentPierceCount, const int32 MaxPierceCount);
};
