// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Structs/CustomCombatData.h"
#include "Structs/EnumProjectileInteractionType.h"
#include "Projectile.generated.h"

class UParticleSystemComponent;
class UParticleSystem;
class UProjectileMovementComponent;
class UBoxComponent;

UCLASS()
class FURYOFLEGENDS_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastConfigureProjectile(UParticleSystem* TrailEffect, AActor* InTarget, bool bIsHomingProjectile, float HomingAcceleration, float InitialSpeed, float MaxSpeed);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Projectile|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class USceneComponent> DefaultRootComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Projectile|Components", meta = (AllowPrivateAccess))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Projectile|Components", meta = (AllowPrivateAccess))
	TObjectPtr<UBoxComponent> BoxCollision;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Projectile|Particles", meta = (AllowPrivateAccess))
	TObjectPtr<UParticleSystemComponent> TrailParticleSystem;

public:
	UPROPERTY(Transient, VisibleDefaultsOnly, BlueprintReadWrite, Category = "Projectile|Damage", meta = (AllowPrivateAccess))
	AActor* TargetActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Damage", Meta = (AllowPrivateAccess = "true"))
	FDamageInformation DamageInformation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Damage", Meta = (AllowPrivateAccess = "true"))
	EProjectileInteractionType ProjectileInteractionType;
};
