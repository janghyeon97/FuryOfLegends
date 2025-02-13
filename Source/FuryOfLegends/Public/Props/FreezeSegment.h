// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Structs/CustomCombatData.h"
#include "FreezeSegment.generated.h"

UCLASS()
class FURYOFLEGENDS_API AFreezeSegment : public AActor
{
	GENERATED_BODY()
	
public:	
	AFreezeSegment();

	void OnParticleEnded();

protected:
	virtual void BeginPlay() override;

	virtual void HandleServer();
	virtual void HandleClient();

	void CalculateParticleLocations(TArray<FVector>& OutLocations, TArray<FRotator>& OutRotations);
	virtual void ApplyDamage(AActor* OtherActor);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayParticles(const TArray<FVector>& SpawnLocations, const TArray<FRotator>& SpawnRotations, const int32 InNumParicles, const float ParticleScale, const float InRate, const float InLifetime);

	// Overlap event functions
	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	UPROPERTY(Transient)
	TWeakObjectPtr<class AAOSCharacterBase> OwnerCharacter;

	// �ݸ��� �ڽ� ������Ʈ �迭
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	TArray<class UBoxComponent*> CollisionBoxes;

	// ��ƼŬ ������Ʈ �迭
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	TArray<class UParticleSystemComponent*> Particles;

	// �ߺ� ó���� ���͸� �����ϱ� ���� ���� (�ߺ� �˻� ����)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	TSet<AActor*> ProcessedActors;

	// ������ ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	FDamageInformation DamageInformation;

	// ��ƼŬ ��� Ÿ�̸� �ڵ�
	UPROPERTY(Transient)
	FTimerHandle ParticleTimer;

	// ��ƼŬ ���� Ÿ�̸� �ڵ�
	UPROPERTY(Transient)
	FTimerHandle ParticleEndTimer;

	// ������ ������ ��ġ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	FVector LastActorLocation = FVector::ZeroVector;

	// ������ ������ ���� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	FVector LastActorForwardVector = FVector::ZeroVector;

	// ������ ������ ��� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	FVector LastActorUpVector = FVector::ZeroVector;

	// ��ƼŬ ����� ����ϴ� �ݺ���
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	int32 Iterator = 0;

	// ��ƼŬ �����ֱ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0", AllowPrivateAccess = "true"))
	int32 Lifetime = 0;

	// ��ƼŬ ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0", AllowPrivateAccess = "true"))
	int32 NumParicles = 0;

	// ��ƼŬ �ݰ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Radius = 0.f;

	// ��ƼŬ ��� �ӵ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Rate = 0.f;

	// �� ��ƼŬ ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Angle = 0.f;

	// ��ƼŬ ������
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Scale = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> Detection;

	// ���Ǵ� ��ƼŬ �ý���
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	UParticleSystem* FreezeSegment = nullptr;
};
