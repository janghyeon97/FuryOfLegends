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

	// 콜리전 박스 컴포넌트 배열
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	TArray<class UBoxComponent*> CollisionBoxes;

	// 파티클 컴포넌트 배열
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	TArray<class UParticleSystemComponent*> Particles;

	// 중복 처리된 액터를 추적하기 위한 집합 (중복 검사 방지)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	TSet<AActor*> ProcessedActors;

	// 데미지 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	FDamageInformation DamageInformation;

	// 파티클 재생 타이머 핸들
	UPROPERTY(Transient)
	FTimerHandle ParticleTimer;

	// 파티클 종료 타이머 핸들
	UPROPERTY(Transient)
	FTimerHandle ParticleEndTimer;

	// 마지막 액터의 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	FVector LastActorLocation = FVector::ZeroVector;

	// 마지막 액터의 전방 벡터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	FVector LastActorForwardVector = FVector::ZeroVector;

	// 마지막 액터의 상단 벡터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	FVector LastActorUpVector = FVector::ZeroVector;

	// 파티클 재생에 사용하는 반복자
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	int32 Iterator = 0;

	// 파티클 생명주기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0", AllowPrivateAccess = "true"))
	int32 Lifetime = 0;

	// 파티클 개수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0", AllowPrivateAccess = "true"))
	int32 NumParicles = 0;

	// 파티클 반경
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Radius = 0.f;

	// 파티클 재생 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Rate = 0.f;

	// 각 파티클 사이 각도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Angle = 0.f;

	// 파티클 스케일
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Freeze Segment", Meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Scale = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> Detection;

	// 사용되는 파티클 시스템
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Freeze Segment", Meta = (AllowPrivateAccess = "true"))
	UParticleSystem* FreezeSegment = nullptr;
};
