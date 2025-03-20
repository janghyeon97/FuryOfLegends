#include "Item/Item.h"
#include "Game/ArenaPlayerState.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"
#include "Structs/ActionData.h"
#include "Item/ItemData.h"

AItem::AItem()
{
    FunctionMap.Add(ECharacterStat::MaxHealthPoints, &AItem::ModifyMaxHealthPoints);
    FunctionMap.Add(ECharacterStat::MaxManaPoints, &AItem::ModifyMaxManaPoints);
    FunctionMap.Add(ECharacterStat::HealthRegeneration, &AItem::ModifyHealthRegeneration);
    FunctionMap.Add(ECharacterStat::ManaRegeneration, &AItem::ModifyManaRegeneration);
    FunctionMap.Add(ECharacterStat::AttackDamage, &AItem::ModifyAttackDamage);
    FunctionMap.Add(ECharacterStat::AbilityPower, &AItem::ModifyAbilityPower);
    FunctionMap.Add(ECharacterStat::DefensePower, &AItem::ModifyDefensePower);
    FunctionMap.Add(ECharacterStat::MagicResistance, &AItem::ModifyMagicResistance);
    FunctionMap.Add(ECharacterStat::AttackSpeed, &AItem::ModifyAttackSpeed);
    FunctionMap.Add(ECharacterStat::AbilityHaste, &AItem::ModifyAbilityHaste);
    FunctionMap.Add(ECharacterStat::CriticalChance, &AItem::ModifyCriticalChance);
    FunctionMap.Add(ECharacterStat::MovementSpeed, &AItem::ModifyMovementSpeed);
}

void AItem::Initialize()
{

}

void AItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    RemoveAbilitiesFromCharacter();

    FunctionMap.Empty();
    StatModifiers.Empty();
}


void AItem::Use(AArenaPlayerState* PlayerState)
{
    if (::IsValid(PlayerState) == false)
    {
        return;
    }
}

void AItem::BindToPlayer(AAOSCharacterBase* Character)
{
    if (::IsValid(Character))
    {
        OwnerCharacter = Character;

        if (Character->OnHitEventTriggered.IsAlreadyBound(this, &AItem::OnHit) == false)
        {
            Character->OnHitEventTriggered.AddDynamic(this, &AItem::OnHit);
        }

        if (Character->OnAttackEventTriggered.IsAlreadyBound(this, &AItem::OnAttack) == false)
        {
            Character->OnAttackEventTriggered.AddDynamic(this, &AItem::OnAttack);
        }

        if (Character->OnAbilityEffectsEventTriggered.IsAlreadyBound(this, &AItem::OnAbilityEffects) == false)
        {
            Character->OnAbilityEffectsEventTriggered.AddDynamic(this, &AItem::OnAbilityEffects);
        }

        if (Character->OnReceiveDamageEnteredEvent.IsAlreadyBound(this, &AItem::OnReceiveDamageEntered) == false)
        {
            Character->OnReceiveDamageEnteredEvent.AddDynamic(this, &AItem::OnReceiveDamageEntered);
        }

        ApplyAbilitiesToCharacter();
    }
}



void AItem::OnHit(FDamageInformation& DamageInformation)
{
    // Implement functionality for when character is hit
}

void AItem::OnAttack(FDamageInformation& DamageInformation)
{
    // Implement functionality for when character attacks
}

void AItem::OnAbilityEffects(FDamageInformation& DamageInformation)
{
    // Implement functionality for ability effects
}

void AItem::OnReceiveDamageEntered(bool& bResult)
{
    // Implement functionality for when character receives damage
}

void AItem::ApplyAbilitiesToCharacter()
{
    if (!OwnerCharacter.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid OwnerCharacter in ApplyAbilitiesToCharacter."));
        return;
    }

    RemoveAbilitiesFromCharacter();

    for (const FItemStatModifier& Stat : StatModifiers)
    {
        // ItemAbility.Key는 ECharacterStat, ItemAbility.Value는 int32
        if (!FunctionMap.Contains(Stat.Key))
        {
            UE_LOG(LogTemp, Warning, TEXT("Ability type %d not found in FunctionMap"), static_cast<int32>(Stat.Key));
            continue;
        }

        // FunctionMap에서 해당 키의 함수를 가져옴
        ModifierFunction Function = FunctionMap[Stat.Key];
        if (Function != nullptr)
        {
            int32 ModifierValue = CurrentStackPerSlot * Stat.Value;

            // 능력치 적용
            (this->*Function)(OwnerCharacter.Get(), ModifierValue);

            // 적용된 수치 기록 (기존 값이 있으면 누적)
            if (ModifiedStats.Contains(Stat.Key))
            {
                ModifiedStats[Stat.Key] += ModifierValue;
            }
            else
            {
                ModifiedStats.Add(Stat.Key, ModifierValue);
            }
        }
    }
}


void AItem::RemoveAbilitiesFromCharacter()
{
    if (!OwnerCharacter.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid OwnerCharacter in RemoveAbilitiesFromCharacter."));
        return;
    }

    for (const FItemStatModifier& Stat : StatModifiers)
    {
        if (!FunctionMap.Contains(Stat.Key))
        {
            UE_LOG(LogTemp, Warning, TEXT("CharacterStat %s not found in FunctionMap"), *StaticEnum<ECharacterStat>()->GetNameStringByValue(static_cast<int64>(Stat.Key)));
            continue;
        }

        // StatModifiers에서 능력치 확인
        if (!ModifiedStats.Contains(Stat.Key))
        {
            UE_LOG(LogTemp, Warning, TEXT("No modifier found for CharacterStat %d in ModifiedStats."), *StaticEnum<ECharacterStat>()->GetNameStringByValue(static_cast<int64>(Stat.Key)));
            continue;
        }

        ModifierFunction Function = FunctionMap[Stat.Key];
        if (Function != nullptr)
        {
            int32 ModifierValue = ModifiedStats[Stat.Key];

            (this->*Function)(OwnerCharacter.Get(), -ModifierValue);
            ModifiedStats.Remove(Stat.Key);

            UE_LOG(LogTemp, Log, TEXT("CharacterStat %s removed from ModifiedStats."), *StaticEnum<ECharacterStat>()->GetNameStringByValue(static_cast<int64>(Stat.Key)));
        }
    }
}


template <typename T>
void AItem::ModifyStat(AAOSCharacterBase* Character, T(UStatComponent::* Getter)() const, void (UStatComponent::* Setter)(T), int32 Value)
{
    if (!::IsValid(Character))
    {
        return;
    }

    UStatComponent* PlayerStatComponent = Character->GetStatComponent();
    if (!::IsValid(PlayerStatComponent))
    {
        return;
    }

    T CurrentValue = (PlayerStatComponent->*Getter)();
    (PlayerStatComponent->*Setter)(Value);
}

void AItem::ModifyMaxHealthPoints(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetMaxHP, &UStatComponent::ModifyAccumulatedFlatMaxHP, Value);
    ModifyStat(Character, &UStatComponent::GetCurrentHP, &UStatComponent::ModifyCurrentHP, Value);
}

void AItem::ModifyMaxManaPoints(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetMaxMP, &UStatComponent::ModifyAccumulatedFlatMaxMP, Value);
    ModifyStat(Character, &UStatComponent::GetCurrentMP, &UStatComponent::ModifyCurrentMP, Value);
}

void AItem::ModifyHealthRegeneration(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetHealthRegeneration, &UStatComponent::ModifyAccumulatedFlatHealthRegeneration, Value);
}

void AItem::ModifyManaRegeneration(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetManaRegeneration, &UStatComponent::ModifyAccumulatedFlatManaRegeneration, Value);
}

void AItem::ModifyAttackDamage(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetAttackDamage, &UStatComponent::ModifyAccumulatedFlatAttackDamage, Value);
}

void AItem::ModifyAbilityPower(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetAbilityPower, &UStatComponent::ModifyAccumulatedFlatAbilityPower, Value);
}

void AItem::ModifyDefensePower(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetDefensePower, &UStatComponent::ModifyAccumulatedFlatDefensePower, Value);
}

void AItem::ModifyMagicResistance(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetMagicResistance, &UStatComponent::ModifyAccumulatedFlatMagicResistance, Value);
}

void AItem::ModifyAttackSpeed(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetAttackSpeed, &UStatComponent::ModifyAccumulatedPercentAttackSpeed, Value);
}

void AItem::ModifyAbilityHaste(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetAbilityHaste, &UStatComponent::ModifyAccumulatedFlatAbilityHaste, Value);
}

void AItem::ModifyCriticalChance(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetCriticalChance, &UStatComponent::ModifyAccumulatedFlatCriticalChance, Value);
}

void AItem::ModifyMovementSpeed(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetMovementSpeed, &UStatComponent::ModifyAccumulatedPercentMovementSpeed, Value);
}
