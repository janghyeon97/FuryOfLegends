#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "Structs/MinionData.h"
#include "MinionBase.generated.h"

class UNavArea;

UCLASS()
class FURYOFLEGENDS_API AMinionBase : public ACharacterBase
{
	GENERATED_BODY()

public:
	AMinionBase();

	virtual void InitializeCharacterResources() override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void StartFadeOut();
	virtual void EnableRagdoll();
	virtual void ApplyDirectionalImpulse();
	virtual void ChangeNavModifierAreaClass(TSubclassOf<UNavArea> NewAreaClass) override;

	// Experience distribution
	virtual void FindNearbyPlayers(TArray<ACharacterBase*>& PlayerCharacters, ETeamSide InTeamSide, float Distance);
	virtual void DistributeExperience(ACharacterBase* Eliminator, const TArray<ACharacterBase*>& NearbyEnemies);

	UFUNCTION()
	virtual void InitializeWidget();

	UFUNCTION()
	virtual void OnCharacterDeath();

	UFUNCTION(NetMulticast, Reliable)
	virtual void OnCharacterDeath_Multicast();

	UFUNCTION()
	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastApplyImpulse(FVector Impulse);

	UFUNCTION()
	void OnRep_SkeletalMesh();

public:
	// Getter and Setter functions for Bounty
	UFUNCTION(BlueprintCallable, Category = "Bounty")
	float GetExpBounty() const;
	UFUNCTION(BlueprintCallable, Category = "Bounty")
	int32 GetGoldBounty() const;
	UFUNCTION(BlueprintCallable, Category = "Bounty")
	void SetExpBounty(float NewExpBounty);
	UFUNCTION(BlueprintCallable, Category = "Bounty")
	void SetGoldBounty(int32 NewGoldBounty);

	// Direction calculation
	UFUNCTION(BlueprintCallable, Category = "Direction")
	int32 GetRelativeDirection(AActor* OtherActor) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MinionBase")
	TObjectPtr<class AMinionAIController> AIController;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "MinionBase", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUserWidgetBarBase> HPBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MinionBase", Meta = (AllowPrivateAccess))
	TObjectPtr<class UNavModifierComponent> NavModifier;

public:
	UPROPERTY(ReplicatedUsing = OnRep_SkeletalMesh, VisibleAnywhere, BlueprintReadOnly, Category = "Minion|GamePlay", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> ReplicatedSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|GamePlay", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> SplineActor;

	// 적 캐릭터와 상대적 방향
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|GamePlay", Meta = (AllowPrivateAccess = "true"))
	int32 RelativeDirection;

	// 경험치 분배 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|GamePlay", Meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = "true"))
	float ExperienceShareRadius;

	// 인원수에 따른 경험치 분배량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|GamePlay", Meta = (AllowPrivateAccess = "true"))
	TMap<int32, float> ShareFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|GamePlay", Meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = "true"))
	float ExpBounty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|GamePlay", Meta = (ClampMin = "0", UIMin = "0", AllowPrivateAccess = "true"))
	int32 GoldBounty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|GamePlay", Meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = "true"))
	float ImpulseStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|GamePlay", Meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = "true"))
	float ChaseThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|GamePlay", Meta = (ClampMin = "0.0", UIMin = "0.0", AllowPrivateAccess = "true"))
	float FadeOutDuration;

protected:
	FTimerHandle FadeOutTimerHandle;
	FTimerHandle DeathMontageTimerHandle;

	float RagdollBlendTime;
	float CurrentFadeDeath;
};
