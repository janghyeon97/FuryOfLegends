// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AOSCharacterBase.h"
#include "AuroraCharacter.generated.h"


class USplineComponent;
class USplineMeshComponent;

USTRUCT(BlueprintType)
struct FTerrainSegment
{
	GENERATED_BODY()

public:
	FTerrainSegment()
		: StartIndex(0), EndIndex(0), Type("Flat") {}

	FTerrainSegment(int32 InStartIndex, int32 InEndIndex, const FString& InType)
		: StartIndex(InStartIndex), EndIndex(InEndIndex), Type(InType) {}

public:
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	int32 StartIndex;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	int32 EndIndex;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	FString Type; // "Flat", "Uphill", "Downhill", "Dip"
};



UCLASS()
class FURYOFLEGENDS_API AAuroraCharacter : public AAOSCharacterBase
{
	GENERATED_BODY()
	
public:
	AAuroraCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Utility functions
	virtual void Move(const FInputActionValue& InValue) override;
	virtual void Look(const FInputActionValue& InValue) override;

	/** ------------------------------------------------------ Ability Q Functions ------------------------------------------------------ */

	virtual void Q_Executed() override;
	virtual void Q_Canceled() override;
	virtual void Q_CheckHit() override;

	/** ------------------------------------------------------ Ability E Functions ------------------------------------------------------ */

	virtual void E_Executed() override;
	virtual void E_Canceled() override;

	void ExecutePrimary();
	void ExecuteSecondary();

	/**  애니메이션이 재생된 후 특정 시점이 지나고 대쉬를 시작할 때 호출됨.  */
	void StartDash();

	/**  대쉬가 끝날 때 호출됨.  */
	void EndDash();

	/**  대쉬하고 Spline 을 따라 캐릭터의 속도를 변경하는 함수.  */
	void HandleDashing(float DeltaSeconds);

	/**  캐릭터 앞에 쉴드 매쉬를 스폰하는 함수.   */
	void SpawnShieldMeshes();

	/**
	 * 지정된 거리와 스텝 크기를 기반으로 지형을 탐색하여 경로를 생성합니다.
	 * @param TraceDistance 최대 탐색 거리 (단위: cm)
	 * @param StepSize 한 번의 탐색 스텝 크기 (단위: cm, 기본값: 50cm)
	 * @param HeightThreshold 경로 생성 시 허용되는 최대 높이 차이 (단위: cm, 기본값: 600cm)
	 * @return 탐색된 지형 경로 배열 (FVector 목록)
	 */
	TArray<FVector> TraceTerrainPath(const float TraceDistance, const float StepSize = 50.f, const float HeightThreshold = 600.f);

	/**
	 * 경로 점(PathPoints)을 기반으로 지형 구간(TerrainSegments)을 분석합니다.
	 * - 경로를 높이 변화에 따라 Flat, Uphill, Downhill로 분류합니다.
	 * - 분류된 구간 정보를 OutTerrainSegments에 저장합니다.
	 *
	 * @param PathPoints 경로 배열.
	 * @return TerrainSegments 분석된 지형 구간 정보 배열.
	 */
	TArray<FTerrainSegment> AnalyzeTerrainSegments(const TArray<FVector>& PathPoints, const float& MaxHeightDifference);


	/**
	 * TerrainSegments 배열을 처리하여 지형 구간 정보를 최적화합니다.
	 * - Dip 처리: Downhill -> (Flat) -> Uphill 구간을 Dip으로 병합.
	 * - 오르막(Uphill) 확장: 이전 Flat 구간을 포함하여 시작점 확장.
	 * - 내리막(Downhill) 확장: 이후 Flat 구간을 포함하여 끝점 확장.
	 *
	 * @param TerrainSegments 입력된 지형 구간 정보를 최적화된 형태로 업데이트합니다.
	 */
	void ProcessTerrainSegments(TArray<FTerrainSegment>& TerrainSegments);


	/**
	 * 지형 구간(TerrainSegments)을 기반으로 경로 점(PathPoints)을 평활화(Smoothing)합니다.
	 * - 지정된 구간의 시작점과 끝점 높이를 보간하여 중간 점들을 부드럽게 연결합니다.
	 * - Ease-In/Ease-Out 보간 방식을 기본으로 사용합니다.
	 *
	 * @param PathPoints 경로 점 배열. 평활화된 경로로 업데이트됩니다.
	 * @param TerrainSegments 처리할 지형 구간 배열.
	 */
	void SmoothTerrainSegments(TArray<FVector>& PathPoints, const TArray<FTerrainSegment>& TerrainSegments);
	void VisualizeTerrainSegments(const TArray<FTerrainSegment>& TerrainSegments, const TArray<FVector>& PathPoints);

	/** ------------------------------------------------------ Ability R Functions ------------------------------------------------------ */

	virtual void R_Started() override;
	virtual void R_Executed() override;
	virtual void R_CheckHit() override;

	void FindExplosionTargets(const FVector& Pos, TSet<TWeakObjectPtr<ACharacterBase>>& OutTargets, TEnumAsByte<ECollisionChannel> CollisionChannel, const float InRadius);
	void ScheduleExplosion(TSet<TWeakObjectPtr<ACharacterBase>>& Targets, TSharedPtr<TSet<TWeakObjectPtr<ACharacterBase>>> AffectedCharacters);
	void HandleExplosion(TSet<TWeakObjectPtr<ACharacterBase>>& FrozenEnemy, TSharedPtr<TSet<TWeakObjectPtr<ACharacterBase>>>AffectedCharacters);

	/**
	 * 캐릭터의 착지 지점을 계산하고 낙하를 시작합니다.
	 * @brief LineTrace 를 사용하여 착지 지점을 계산하고 관련 변수를 초기화합니다.
	 */
	void LandWithinDuration();


	/**
	 * 캐릭터의 낙하를 처리합니다.
	 * @brief 낙하 진행률을 계산해 위치와 속도를 업데이트합니다.
	 * @param DeltaSeconds 프레임 간 경과 시간.
	 */
	void HandleFalling(float DeltaSeconds);

	/** ------------------------------------------------------ Ability LMB Functions ------------------------------------------------------ */

	virtual void LMB_Started() override;
	virtual void LMB_Executed() override;
	virtual void LMB_CheckHit() override;
	virtual void LMB_Canceled() override;

	/** ------------------------------------------------------ Ability RMB Functions ------------------------------------------------------ */
	virtual void RMB_Started() override;
	virtual void RMB_Executed() override;
	virtual void RMB_Canceled() override;

	int32 CalculateDirectionIndex();
	void HandleTumbling(float DeltaSeconds);
	void SmoothMovement(float DeltaSeconds);
	FVector CalculateTumblingDestination(const FVector& MoveDirection, const float Rnage);

	
	/** ------------------------------------------------------ */

	virtual void CancelAction() override;
	
	bool ValidateAbilityUsage();
	virtual void ExecuteSomethingSpecial() override;

	virtual void HandleActionNotifyEvent(EActionSlot SlotID, int32 EventID) override;
	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted) override;
	virtual void OnPreDamageReceived(float FinalDamage) override;
	virtual void OnRep_CharacterStateChanged() override;
	virtual void OnRep_CrowdControlStateChanged() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetTargetLocation(FVector InTargetLocation);

private:
	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", meta = (AllowPrivateAccess))
	FVector ReplicatedTargetLocation;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|GamePlay", meta = (AllowPrivateAccess))
	bool bSmoothMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline", meta = (AllowPrivateAccess))
	UStaticMesh* PathMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline", meta = (AllowPrivateAccess))
	UMaterialInterface* PathMaterial;

	/** ------------------------------------------------------ Ability E Values ------------------------------------------------------ */
	bool bIsDashing;
	float DashingDistance;
	float DashingDuration;
	float DashingElapsedTime;

	float DistanceAlongSpline;
	float PreviousDistanceAlongSpline;

	FVector DashingDestination;

	class ASplineActor* IcePath = nullptr;

	/* Ability RMB */
	bool bIsTumbling;

	float TumblingSpeed;
	float TumblingDuration;
	float TumblingDistance;
	float TumblingElapsedTime;
	float TumblingHeightScale;
	float TumbleHeightThreshold;

	FVector TumblingDirection;
	FVector TumblingDestination;

	/* Ability R */
	bool bIsFalling;
	float FallingDuration;
	float FallingElapsedTime;
	FVector FallingDestination;

	FTimerHandle RecastTimer;
	FTimerHandle ProcessExplosionTimer;
};
