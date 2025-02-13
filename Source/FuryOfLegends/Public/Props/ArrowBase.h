// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Structs/CharacterData.h"
#include "Structs/CustomCombatData.h"
#include "ArrowBase.generated.h"

USTRUCT(BlueprintType)
struct FArrowProperties
{
	GENERATED_BODY()

public:
	FArrowProperties()
		: InitialSpeed(5000.f)
		, MaxSpeed(4000.f)
		, MaxRange(3000.f)
		, CollisionRadius(10.f)
		, MaxPierceCount(0)
		, DamageReductionPerPierce(0.0f)
		, ExplosionRadius(0.f) // �⺻�� 0: �������� ����
		, bIsHoming(false) // ���� ��� ��Ȱ��ȭ
		, TargetActor(nullptr)
		, HomingAcceleration(20000.f)
		, Detection(ECollisionChannel::ECC_Visibility)
	{
	};

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|Properties", meta = (ClampMin = "0.0", ClampMax = "20000.0", uIMin = "0.0", uIMax = "20000.0"))
	float InitialSpeed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|Properties", meta = (ClampMin = "0.0", ClampMax = "20000.0", uIMin = "0.0", uIMax = "20000.0"))
	float MaxSpeed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|Properties", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
	float MaxRange;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|Properties", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
	float CollisionRadius;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|Properties", meta = (ClampMin = "0", ClampMax = "100", uIMin = "0", uIMax = "100"))
	int MaxPierceCount;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|Properties", meta = (ClampMin = "0.0", ClampMax = "100", uIMin = "0.0", uIMax = "100"))
	float DamageReductionPerPierce;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|ExplosiveProperties", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float ExplosionRadius;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|HomingProperties")
	bool bIsHoming;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|HomingProperties")
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|HomingProperties", meta = (ClampMin = "0.0", ClampMax = "100000.0"))
	float HomingAcceleration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arrow|Properties", Meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> Detection;
};




UCLASS()
class FURYOFLEGENDS_API AArrowBase : public AActor
{
	GENERATED_BODY()

public:
	AArrowBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void InitializeArrow(const FArrowProperties& InArrowProperties, const FDamageInformation& InDamageInformation);
	virtual void AttachToClosestBone(AActor* InTargetActor);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastConfigureProjectile(const FArrowProperties& InArrowProperties);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLaunchArrow();

protected:
	virtual void StopArrow();
	
	virtual void ApplyDamage(AActor* OtherActor);
	virtual TArray<AActor*> DetectActorsInExplosionRadius();

	virtual void OnArrowHit(const FHitResult& HitResult);
	virtual void OnHitWorld(const FHitResult& HitResult);
	virtual void OnHitCharacter(const FHitResult& HitResult);

	UFUNCTION(Server, Reliable)
	void ServerNotifyParticleFinished(AController* ClientController);

	UFUNCTION()
	virtual void OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent) {};

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects|Particles", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystem> HitWorldEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects|Particles", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystem> HitPlayerEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class USceneComponent> DefaultRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components|Mesh", Meta = (AllowPrivateAccess))
	TObjectPtr<class UStaticMeshComponent> ArrowStaticMesh;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Effects|Particles", meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystemComponent> ArrowTrailParticleSystem;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Effects|Particles", meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystemComponent> ArrowImpactParticleSystem;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components|Movement", meta = (AllowPrivateAccess))
	TObjectPtr<class UProjectileMovementComponent> ArrowProjectileMovement;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow|Owner", Meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class ACharacterBase> OwnerCharacter;

	// �浹 �� ������ ���͵� ���
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow|Ignored", Meta = (AllowPrivateAccess = "true"))
	TArray<AActor*> IgnoredActors;

	// ��ƼŬ ��� �ϷḦ �˸� Ŭ���̾�Ʈ ���
	UPROPERTY(Transient)
	TArray<TObjectPtr<AController>> CompletedClients;

	// ȭ�� ���� �� ����Ǵ� ���� ĳ������ ��ġ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow|GamePlay", Meta = (AllowPrivateAccess = "true"))
	FVector OwnerLocation;

	// ��ƼŬ ����� �Ϸ��� Ŭ���̾�Ʈ ��
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|Effects", Meta = (AllowPrivateAccess = "true"))
	int32 ClientParticleFinishedCount = 0;

	// ȭ���� ������ Ƚ��
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|GamePlay", Meta = (AllowPrivateAccess = "true"))
	int32 PierceCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|GamePlay", Meta = (AllowPrivateAccess = "true"))
	ETeamSide TeamSide;

	// ��� ��ƼŬ ����� ���� �� ȭ���� �ı��ؾ� �ϴ��� ����
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|State", Meta = (AllowPrivateAccess = "true"))
	bool bShouldDestroy = false;

	// �浹 �˻縦 �ϴ� �� ����
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|State", Meta = (AllowPrivateAccess = "true"))
	bool bShouldSweep = true;

	// ȭ���� �Ӽ�(�ӵ�, ��Ÿ� ��)�� �����ϴ� ����ü
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Arrow|GamePlay", Meta = (AllowPrivateAccess = "true"))
	FArrowProperties ArrowProperties;

	// ȭ���� ������ ���� ������ �����ϴ� ����ü
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow|GamePlay", Meta = (AllowPrivateAccess = "true"))
	FDamageInformation DamageInformation;
};
