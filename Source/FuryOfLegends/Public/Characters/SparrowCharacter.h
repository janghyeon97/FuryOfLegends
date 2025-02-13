// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AOSCharacterBase.h"
#include "SparrowCharacter.generated.h"

/**
 * 스패로우 캐릭터 클래스입니다. 기본 AOS 캐릭터 기능에 더하여 스패로우 특유의 능력들을 구현합니다.
 */
UCLASS()
class FURYOFLEGENDS_API ASparrowCharacter : public AAOSCharacterBase
{
	GENERATED_BODY()

public:
	ASparrowCharacter();

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostCharacterSpawn() override;

protected:
	// Utility functions
	virtual void Move(const FInputActionValue& InValue) override;
	virtual void Look(const FInputActionValue& InValue) override;

	/** ------------------------------------------------------ Ability Q Functions ------------------------------------------------------ */

	virtual void Q_Started() override;
	virtual void Q_Released() override;
	virtual void Q_Executed() override;
	virtual void Q_CheckHit() override;
	virtual void Q_Canceled() override;

	void HandleRainOfArrows(UAnimMontage* Montage);

	/** ------------------------------------------------------ Ability E Functions ------------------------------------------------------ */

	virtual void E_Executed() override;

	/** ------------------------------------------------------ Ability R Functions ------------------------------------------------------ */

	virtual void R_Executed() override;

	/** ------------------------------------------------------ Ability LMB Functions ------------------------------------------------------ */

	virtual void LMB_Executed() override;

	void ExecutePrimaryAction();
	void ExecuteUltimateAction();


	/** ------------------------------------------------------ Ability RMB Functions ------------------------------------------------------ */

	virtual void RMB_Started() override;
	virtual void RMB_Executed() override;
	virtual void RMB_Canceled() override;

	void ChangeCameraLength(float TargetLength);

	/** ------------------------------------------------------ */
	
	virtual void CancelAction() override;

	FVector ProcessImpactPoint(const FHitResult& ImpactResult);
	UAnimMontage* GetMontageBasedOnAttackSpeed(float AttackSpeed);

	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted) override;
	virtual void OnRep_CharacterStateChanged() override;
	virtual void OnRep_CrowdControlStateChanged() override;

private:
	virtual void ExecuteSomethingSpecial() override;
	bool ValidateAbilityUsage();

	UFUNCTION(Server, Reliable)
	void ServerSpawnArrow(UClass* SpawnArrowClass, FTransform SpawnTransform, FArrowProperties InArrowProperties, FDamageInformation InDamageInfomation);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Particles", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystemComponent> BowParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Arrow", Meta = (AllowPrivateAccess))
	TObjectPtr<AActor> TargetDecalActor;

private:
	TMap<uint32, int32> ExplosionCounts;

	float Ability_Q_Range;
	FVector Ability_Q_DecalLocation;
	FVector Ability_LMB_ImpactPoint;
};
