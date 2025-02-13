// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent.generated.h"

#pragma region DefineDelegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentHPChangedDelegate, float, InOldCurrentHP, float, InNewCurrentHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHPChangedDelegate, float, InOldMaxHP, float, InNewMaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOutOfCurrentHPDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthFullDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthDepletedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentMPChangedDelegate, float, InOldCurrentMP, float, InNewCurrentMP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxMPChangedDelegate, float, InOldMaxMP, float, InNewMaxMP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnManaFullDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnManaDepletedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentLevelChangedDelegate, int32, InOldCurrentLevel, int32, InNewCurrentLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentEXPChangedDelegate, float, InOldCurrentEXP, float, InNewCurrentEXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxEXPChangedDelegate, float, InOldMaxEXP, float, InNewMaxEXP);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPRegenChangedDelegate, float, InOldHPRegen, float, InNewHPRegen);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMPRegenChangedDelegate, float, InOldMPRegen, float, InNewMPRegen);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackDamageChangedDelegate, float, InOldAttackDamage, float, InNewAttackDamage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityPowerChangedDelegate, float, InOldAbilityPower, float, InNewAbilityPower);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDefensePowerChangedDelegate, float, InOldDefensePower, float, InNewDefensePower);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMagicResistanceChangedDelegate, float, InOldMagicResistance, float, InNewMagicResistance);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackSpeedChangedDelegate, float, InOldAttackSpeed, float, InNewAttackSpeed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityHasteChangedDelegate, int32, InOldAbilityHaste, int32, InNewAbilityHaste);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCriticalChanceChangedDelegate, int32, InOldCriticalChance, int32, InNewCriticalChance);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMovementSpeedChangedDelegate, float, InOldMovementSpeed, float, InNewMovementSpeed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterStatReplicatedDelegate);
#pragma endregion




struct FStatTableRow;
class UDataTable;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FURYOFLEGENDS_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStatComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
	virtual void InitializeComponent() override;
	virtual void InitStatComponent(UDataTable* InStatTable);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; 

	void RecalculateStats();
public:
#pragma region GetterAndSetter

	// Getter functions for Base Stats
	float GetBaseMaxHP() const { return BaseMaxHP; }
	float GetBaseMaxMP() const { return BaseMaxMP; }
	float GetBaseCurrentMP() const { return BaseCurrentMP; }
	float GetBaseHealthRegeneration() const { return BaseHealthRegeneration; }
	float GetBaseManaRegeneration() const { return BaseManaRegeneration; }
	float GetBaseAttackDamage() const { return BaseAttackDamage; }
	float GetBaseAbilityPower() const { return BaseAbilityPower; }
	float GetBaseDefensePower() const { return BaseDefensePower; }
	float GetBaseMagicResistance() const { return BaseMagicResistance; }
	int32 GetBaseAbilityHaste() const { return BaseAbilityHaste; }
	float GetBaseAttackSpeed() const { return BaseAttackSpeed; }
	int32 GetBaseCriticalChance() const { return BaseCriticalChance; }
	float GetBaseMovementSpeed() const { return BaseMovementSpeed; }

	// Getter functions for Current Stats
	float GetMaxHP() const { return MaxHP; }
	float GetCurrentHP() const { return CurrentHP; }
	float GetMaxMP() const { return MaxMP; }
	float GetCurrentMP() const { return CurrentMP; }
	float GetMaxEXP() const { return MaxEXP; }
	float GetCurrentEXP() const { return CurrentEXP; }
	int32 GetCurrentLevel() const { return CurrentLevel; }
	float GetHealthRegeneration() const { return HealthRegeneration; }
	float GetManaRegeneration() const { return ManaRegeneration; }
	float GetAttackDamage() const { return AttackDamage; }
	float GetAbilityPower() const { return AbilityPower; }
	float GetDefensePower() const { return DefensePower; }
	float GetMagicResistance() const { return MagicResistance; }
	int32 GetAbilityHaste() const { return AbilityHaste; }
	float GetAttackSpeed() const { return AttackSpeed; }
	int32 GetCriticalChance() const { return CriticalChance; }
	float GetMovementSpeed() const { return MovementSpeed; }

	// Getter functions for Accumulated Buffs/Items Effects
	float GetAccumulatedPercentAttackSpeed() const { return AccumulatedPercentAttackSpeed; }
	float GetAccumulatedFlatAttackSpeed() const { return AccumulatedFlatAttackSpeed; }
	float GetAccumulatedPercentMovementSpeed() const { return AccumulatedPercentMovementSpeed; }
	float GetAccumulatedFlatMovementSpeed() const { return AccumulatedFlatMovementSpeed; }
	float GetAccumulatedFlatMaxHP() const { return AccumulatedFlatMaxHP; }
	float GetAccumulatedFlatMaxMP() const { return AccumulatedFlatMaxMP; }
	float GetAccumulatedFlatMaxEXP() const { return AccumulatedFlatMaxEXP; }
	float GetAccumulatedFlatHealthRegeneration() const { return AccumulatedFlatHealthRegeneration; }
	float GetAccumulatedFlatManaRegeneration() const { return AccumulatedFlatManaRegeneration; }
	float GetAccumulatedFlatAttackDamage() const { return AccumulatedFlatAttackDamage; }
	float GetAccumulatedFlatAbilityPower() const { return AccumulatedFlatAbilityPower; }
	float GetAccumulatedFlatDefensePower() const { return AccumulatedFlatDefensePower; }
	float GetAccumulatedFlatMagicResistance() const { return AccumulatedFlatMagicResistance; }
	float GetAccumulatedFlatAbilityHaste() const { return AccumulatedFlatAbilityHaste; }
	float GetAccumulatedFlatCriticalChance() const { return AccumulatedFlatCriticalChance; }

	// Modify functions for Accumulated Stats
	virtual void SetCurrentLevel(int32 InCurrentLevel);

	virtual void ModifyCurrentHP(float Delta);
	virtual void ModifyCurrentMP(float Delta);
	virtual void ModifyCurrentEXP(float Delta);

	virtual void ModifyAccumulatedFlatMaxHP(float Delta);
	virtual void ModifyAccumulatedFlatMaxMP(float Delta);
	virtual void ModifyAccumulatedFlatHealthRegeneration(float Delta);
	virtual void ModifyAccumulatedFlatManaRegeneration(float Delta);
	virtual void ModifyAccumulatedFlatAttackDamage(float Delta);
	virtual void ModifyAccumulatedFlatAbilityPower(float Delta);
	virtual void ModifyAccumulatedFlatDefensePower(float Delta);
	virtual void ModifyAccumulatedFlatMagicResistance(float Delta);
	virtual void ModifyAccumulatedFlatAbilityHaste(int32 Delta);
	virtual void ModifyAccumulatedFlatCriticalChance(int32 Delta);
	virtual void ModifyAccumulatedPercentAttackSpeed(float Delta);
	virtual void ModifyAccumulatedFlatAttackSpeed(float Delta);
	virtual void ModifyAccumulatedPercentMovementSpeed(float Delta);
	virtual void ModifyAccumulatedFlatMovementSpeed(float Delta);

	// Setter functions for Current Stats
	virtual void SetMaxHP(float InMaxHP);
	virtual void SetCurrentHP(float InCurrentHP);
	virtual void SetMaxMP(float InMaxMP);
	virtual void SetCurrentMP(float InCurrentMP);
	virtual void SetMaxEXP(float InMaxEXP);
	virtual void SetCurrentEXP(float InCurrentEXP);
	virtual void SetHealthRegeneration(float InHealthRegeneration);
	virtual void SetManaRegeneration(float InManaRegeneration);
	virtual void SetAttackDamage(float InAttackDamage);
	virtual void SetAbilityPower(float InAbilityPower);
	virtual void SetDefensePower(float InDefensePower);
	virtual void SetMagicResistance(float InMagicResistance);
	virtual void SetAbilityHaste(int32 InAbilityHaste);
	virtual void SetAttackSpeed(float InAttackSpeed);
	virtual void SetCriticalChance(int32 InCriticalChance);
	virtual void SetMovementSpeed(float InMovementSpeed);

#pragma endregion

public:
	FOnCurrentHPChangedDelegate OnCurrentHPChanged;
	FOnMaxHPChangedDelegate OnMaxHPChanged;
	FOnOutOfCurrentHPDelegate OnOutOfCurrentHP;
	FOnHealthFullDelegate OnHealthFull;
	FOnHealthDepletedDelegate OnHealthDepleted;

	FOnCurrentMPChangedDelegate OnCurrentMPChanged;
	FOnMaxMPChangedDelegate OnMaxMPChanged;
	FOnManaFullDelegate OnManaFull;
	FOnManaDepletedDelegate OnManaDepleted;

	FOnHPRegenChangedDelegate OnHPRegenChanged;
	FOnMPRegenChangedDelegate OnMPRegenChanged;

	FOnCurrentLevelChangedDelegate OnCurrentLevelChanged;
	FOnCurrentEXPChangedDelegate OnCurrentEXPChanged;
	FOnMaxEXPChangedDelegate OnMaxEXPChanged;

	FOnAttackDamageChangedDelegate OnAttackDamageChanged;
	FOnAbilityPowerChangedDelegate OnAbilityPowerChanged;
	FOnDefensePowerChangedDelegate OnDefensePowerChanged;
	FOnMagicResistanceChangedDelegate OnMagicResistanceChanged;
	FOnAttackSpeedChangedDelegate OnAttackSpeedChanged;
	FOnAbilityHasteChangedDelegate OnAbilityHasteChanged;
	FOnCriticalChanceChangedDelegate OnCriticalChanceChanged;
	FOnMovementSpeedChangedDelegate OnMovementSpeedChanged;

	FOnCharacterStatReplicatedDelegate OnCharacterStatReplicated;

protected:
	UFUNCTION()
	void OnRep_CharacterStatReplicated();

	UFUNCTION(NetMulticast, Reliable)
	void OnOutOfCurrentHP_NetMulticast();

	UFUNCTION(NetMulticast, Reliable)
	void OnMaxHPChanged_NetMulticast(float InOldMaxHP, float InNewMaxHP);

	UFUNCTION(NetMulticast, Reliable)
	void OnCurrentHPChanged_NetMulticast(float InOldCurrentHP, float InNewCurrentHP);

	UFUNCTION(NetMulticast, Reliable)
	void OnMaxMPChanged_NetMulticast(float InOldMaxMP, float InNewMaxMP);

	UFUNCTION(NetMulticast, Reliable)
	void OnCurrentMPChanged_NetMulticast(float InOldCurrentMP, float InNewCurrentMP);

	UFUNCTION(NetMulticast, Reliable)
	void OnMaxEXPChanged_NetMulticast(float InOldMaxEXP, float InNewMaxEXP);

	UFUNCTION(NetMulticast, Reliable)
	void OnCurrentEXPChanged_NetMulticast(float InOldCurrentEXP, float InNewCurrentEXP);

	UFUNCTION(NetMulticast, Reliable)
	void OnCurrentLevelChanged_NetMulticast(int32 InOldCurrentLevel, int32 InNewCurrentLevel);

	UFUNCTION(NetMulticast, Reliable)
	void OnHealthRegenerationChanged_NetMulticast(float InOldHealthRegeneration, float InNewHealthRegeneration);

	UFUNCTION(NetMulticast, Reliable)
	void OnManaRegenerationChanged_NetMulticast(float InOldManaRegeneration, float InNewManaRegeneration);

	UFUNCTION(NetMulticast, Reliable)
	void OnAttackDamageChanged_NetMulticast(float InOldAttackDamage, float InNewAttackDamage);

	UFUNCTION(NetMulticast, Reliable)
	void OnAbilityPowerChanged_NetMulticast(float InOldAbilityPower, float InNewAbilityPower);

	UFUNCTION(NetMulticast, Reliable)
	void OnDefensePowerChanged_NetMulticast(float InOldDefensePower, float InNewDefensePower);

	UFUNCTION(NetMulticast, Reliable)
	void OnMagicResistanceChanged_NetMulticast(float InOldMagicResistance, float InNewMagicResistance);

	UFUNCTION(NetMulticast, Reliable)
	void OnAttackSpeedChanged_NetMulticast(float InOldAttackSpeed, float InNewAttackSpeed);

	UFUNCTION(NetMulticast, Reliable)
	void OnAbilityHasteChanged_NetMulticast(int32 InOldAbilityHaste, int32 InNewAbilityHaste);

	UFUNCTION(NetMulticast, Reliable)
	void OnCriticalChanceChanged_NetMulticast(int32 InOldCriticalChance, int32 InNewCriticalChance);

	UFUNCTION(NetMulticast, Reliable)
	void OnMovementSpeedChanged_NetMulticast(float InOldMovementSpeed, float InNewMovementSpeed);

protected:	
	UPROPERTY()
	TObjectPtr<class UAOSGameInstance> GameInstance;

	// Base Stats (기본 스탯)
	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float BaseMaxHP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float BaseMaxMP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float BaseCurrentMP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float BaseHealthRegeneration;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float BaseManaRegeneration;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float BaseAttackDamage;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float BaseAbilityPower;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float BaseDefensePower;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float BaseMagicResistance;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float BaseAttackSpeed;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float BaseMovementSpeed;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	int32 BaseAbilityHaste;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	int32 BaseCriticalChance;

	// Current Stats (현재 스탯)
	UPROPERTY(ReplicatedUsing = OnRep_CharacterStatReplicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float MaxHP;

	UPROPERTY(ReplicatedUsing = OnRep_CharacterStatReplicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float CurrentHP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float MaxMP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float CurrentMP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float MaxEXP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float CurrentEXP;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	int32 MaxLevel;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	int32 CurrentLevel;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float HealthRegeneration;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float ManaRegeneration;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AttackDamage;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AbilityPower;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float DefensePower;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float MagicResistance;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AttackSpeed;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float MovementSpeed;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	int32 AbilityHaste;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	int32 CriticalChance;

	// Accumulated Buffs/Items Effects (누적 변경값)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedFlatMaxHP;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedFlatMaxMP;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedFlatMaxEXP;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedFlatHealthRegeneration;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedFlatManaRegeneration;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedFlatAttackDamage;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedFlatAbilityPower;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedFlatDefensePower;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedFlatMagicResistance;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	int32 AccumulatedFlatAbilityHaste;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	int32 AccumulatedFlatCriticalChance;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedPercentAttackSpeed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedFlatAttackSpeed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedPercentMovementSpeed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Stat", Meta = (AllowPrivateAccess))
	float AccumulatedFlatMovementSpeed;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	UDataTable* StatTable;
};
