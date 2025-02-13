// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Structs/CharacterStatData.h"
#include "Item/ItemData.h"
#include "Item.generated.h"

class AAOSCharacterBase;
class UStatComponent;
class AArenaPlayerState;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API AItem : public AActor
{
	GENERATED_BODY()

public:
    AItem();
   
public:
    virtual void Initialize();
    virtual void Use(AArenaPlayerState* PlayerState);
    virtual void BindToPlayer(AAOSCharacterBase* Character);
    virtual void ApplyAbilitiesToCharacter();
    virtual void RemoveAbilitiesFromCharacter();

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    virtual void OnHit(FDamageInformation& DamageInformation);

    UFUNCTION()
    virtual void OnAttack(FDamageInformation& DamageInformation);

    UFUNCTION()
    virtual void OnAbilityEffects(FDamageInformation& DamageInformation);

    UFUNCTION()
    virtual void OnReceiveDamageEntered(bool& bResult);

public:
    // 아이템 정보
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Info", meta = (AllowPrivateAccess = "true"))
    EItemActivationState ActivationState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Info", meta = (AllowPrivateAccess = "true"))
    int32 ItemCode;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Info", meta = (AllowPrivateAccess = "true"))
    FString Name;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Info", meta = (AllowPrivateAccess = "true"))
    int32 Price;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Info", meta = (AllowPrivateAccess = "true"))
    FString Description;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Info", meta = (AllowPrivateAccess = "true"))
    UTexture* Icon;

    // 아이템 사용 설정
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Usage", meta = (AllowPrivateAccess = "true"))
    int32 MaxConcurrentUses; // 한 번에 활성화할 수 있는 최대 중첩

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Usage", meta = (AllowPrivateAccess = "true"))
    int32 MaxStackPerSlot; // 한 슬롯에서 중첩 가능한 최대 개수

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Usage", meta = (AllowPrivateAccess = "true"))
    int32 MaxInventoryQuantity; // 인벤토리에서 보유 가능한 최대 개수

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Item|Usage", meta = (AllowPrivateAccess = "true"))
    int32 ConcurrentUses;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Item|Usage", meta = (AllowPrivateAccess = "true"))
    int32 CurrentStackPerSlot;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Item|Usage", meta = (AllowPrivateAccess = "true"))
    float CooldownRatio;

    // 스탯 정보
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Stats", meta = (AllowPrivateAccess = "true"))
    EItemClassification Classification;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Stats", meta = (AllowPrivateAccess = "true"))
    TArray<FItemStatModifier> StatModifiers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Stats", meta = (AllowPrivateAccess = "true"))
    TArray<int> RequiredItems;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Stats", meta = (AllowPrivateAccess = "true"))
    TMap<FName, int32> UniqueAttributes;

protected:
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Item")
    TWeakObjectPtr<AAOSCharacterBase> OwnerCharacter;

protected:
    using ModifierFunction = void (AItem::*)(AAOSCharacterBase*, int32);

    void ModifyMaxHealthPoints(AAOSCharacterBase* Character, int32 Value);
    void ModifyMaxManaPoints(AAOSCharacterBase* Character, int32 Value);
    void ModifyHealthRegeneration(AAOSCharacterBase* Character, int32 Value);
    void ModifyManaRegeneration(AAOSCharacterBase* Character, int32 Value);
    void ModifyAttackDamage(AAOSCharacterBase* Character, int32 Value);
    void ModifyAbilityPower(AAOSCharacterBase* Character, int32 Value);
    void ModifyDefensePower(AAOSCharacterBase* Character, int32 Value);
    void ModifyMagicResistance(AAOSCharacterBase* Character, int32 Value);
    void ModifyAttackSpeed(AAOSCharacterBase* Character, int32 Value);
    void ModifyAbilityHaste(AAOSCharacterBase* Character, int32 Value);
    void ModifyCriticalChance(AAOSCharacterBase* Character, int32 Value);
    void ModifyMovementSpeed(AAOSCharacterBase* Character, int32 Value);

private:
    TMap<ECharacterStat, ModifierFunction> FunctionMap;
    TMap<ECharacterStat, int32> ModifiedStats;

    template <typename T>
    void ModifyStat(AAOSCharacterBase* Character, T(UStatComponent::* Getter)() const, void (UStatComponent::* Setter)(T), int32 Value);
};