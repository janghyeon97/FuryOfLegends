// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "Structs/CharacterData.h"
#include "Nexus.generated.h"

class ACharacterBase;
class UDataTable;
class USceneComponent;
class UStatComponent;
class UCameraComponent;
class UActionStatComponent;
class UPointLightComponent;
class UParticleSystemComponent;
class UBoxComponent;
class UCapsuleComponent;
class UParticleSystem;
class UStaticMeshComponent;
struct FDamageInformation;

USTRUCT(BlueprintType)
struct FNexusDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FNexusDataRow()
		: TargetingDelay(0.0f)
		, StatTable(nullptr)
		, AbilityStatTable(nullptr)
		, ResourcesTable(nullptr)
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetingDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* StatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* AbilityStatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* ResourcesTable;
};


UCLASS()
class FURYOFLEGENDS_API ANexus : public ACharacterBase
{
	GENERATED_BODY()
	
public:	
	ANexus();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	virtual void LoadGameplayAssets();
	virtual void SetMeshesFromResource();
	virtual void SetAssetsFromResource();
	virtual void SetMaterialColor(UPrimitiveComponent* MeshComponent, int32 MaterialIndex, FName ParameterName, const FLinearColor& Color);
	virtual ACharacterBase* SelectPriorityTarget();

	template<typename T>
	T* LoadOrGetResourceWithStaticLoad(TMap<FName, T*>& ResourceMap, const FName& Key, const TCHAR* Path);

	virtual void LMB_Started() override;
	virtual void LMB_Executed() override;
	virtual void LMB_CheckHit() override;

	void ActivateCamera();
	void BindNexusInfoToHUD(ACharacterBase* Player);
	void RmoveBindNexusInfoToHUD(ACharacterBase* Player);
	void ApplyDynamicRotation(float DeltaTime);

	UFUNCTION()
	virtual void OnCharacterDeath();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnCharacterDeath();

	UFUNCTION()
	void OnCooldownTimeChanged(EActionSlot SlotID, float CurrentCooldownTime, float MaxCooldownTime);

	UFUNCTION()
	void OnCharacterEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCharacterExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void OnAllyAttacked(AActor* DamageReceiver, AActor* DamageCauser, AController* InstigatorActor, FDamageInformation& DamageInformation);

	UFUNCTION()
	virtual void OnTargetEliminated(AActor* Eliminator);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastActivateTargetEffect(bool bActivation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetTargetCharacter(ACharacterBase* NewTarget);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nexus|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<UDataTable> NexusDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nexus|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<UDataTable> StatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nexus|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<UDataTable> AbilityStatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nexus|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<UDataTable> ResourcesTable;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nexus|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<USceneComponent> DefaultRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nexus|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nexus|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<UPointLightComponent> PointLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nexus|Components", Meta = (AllowPrivateAccess))
	UCameraComponent* NexusCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nexus|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<UBoxComponent> DetectionBox;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nexus|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<UParticleSystemComponent> ProtalParticleSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nexus|Components", Meta = (AllowPrivateAccess, DuplicateTransient))
	TObjectPtr<UParticleSystemComponent> TargetBeamParticleSystem;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nexus|Mesh", Meta = (AllowPrivateAccess))
	TObjectPtr<UStaticMeshComponent> SphereGrid_A;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nexus|Mesh", Meta = (AllowPrivateAccess))
	TObjectPtr<UStaticMeshComponent> SphereGrid_B;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nexus|Mesh", Meta = (AllowPrivateAccess))
	TObjectPtr<UStaticMeshComponent> SphereHalo_A;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nexus|Mesh", Meta = (AllowPrivateAccess))
	TObjectPtr<UStaticMeshComponent> Ring_A;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nexus|Mesh", Meta = (AllowPrivateAccess))
	TObjectPtr<UStaticMeshComponent> Refraction_A;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nexus|Mesh", Meta = (AllowPrivateAccess))
	TObjectPtr<UStaticMeshComponent> Ring_B;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nexus|Mesh", Meta = (AllowPrivateAccess))
	TObjectPtr<UStaticMeshComponent> Refraction_B;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nexus|Mesh", Meta = (AllowPrivateAccess))
	TObjectPtr<UStaticMeshComponent> NexusCenter;

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Nexus|Settings", Meta = (AllowPrivateAccess))
	FLinearColor CoreEmissionColor;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Nexus|Settings", Meta = (AllowPrivateAccess))
	FLinearColor CorePrimartColor;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Nexus|Settings", Meta = (AllowPrivateAccess))
	FLinearColor CoreSecondaryColor;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Nexus|Settings", Meta = (AllowPrivateAccess))
	FLinearColor RingPrimaryColor;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Nexus|Settings", Meta = (AllowPrivateAccess))
	FLinearColor RingSecondaryColor;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Nexus|Settings", Meta = (AllowPrivateAccess))
	FLinearColor CenterPrimaryColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nexus|Settings", Meta = (AllowPrivateAccess))
	FRotator PrimaryRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nexus|Settings", Meta = (AllowPrivateAccess))
	FRotator SecondaryRotation;

private:
	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Nexus|GamePlay", Meta = (AllowPrivateAccess))
	ACharacterBase* TargetCharacter;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Nexus|GamePlay", Meta = (AllowPrivateAccess))
	TArray<ACharacterBase*> EnemiesInRange;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Nexus|GamePlay", Meta = (AllowPrivateAccess))
	TArray<ACharacterBase*> AlliesInRange;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Nexus|GamePlay", Meta = (AllowPrivateAccess))
	float TargetingDelay;

	int32 Count;

	FName PrimaryColorParam = FName(TEXT("PrimaryColor"));
	FName SecondaryColorParam = FName(TEXT("SecondaryColor"));
	FName CenterColorParam = FName(TEXT("Color"));

	FTimerHandle TargetingTimerHandle;
};