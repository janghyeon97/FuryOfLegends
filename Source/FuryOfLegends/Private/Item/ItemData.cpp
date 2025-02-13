// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemData.h"
#include "Components/StatComponent.h"
#include "Plugins/ExpressionEvaluator.h"



FItemTableRow::FItemTableRow()
	: ItemCode(0)
	, Name(FString())
	, Price(0)
	, Description(FString())
	, Icon(nullptr)
	, MaxConcurrentUses(1)
	, MaxStackPerSlot(1)
	, MaxInventoryQuantity(1)
	, Classification(EItemClassification::None)
	, StatModifiers(TArray<FItemStatModifier>())
	, RequiredItems(TArray<int>())
	, UniqueAttributes(TMap<FName, int32>())
	, ItemClass(nullptr)
{
	StatGetters = {
		{TEXT("MaxHealth"), [](const UStatComponent& StatComponent) { return StatComponent.GetMaxHP(); }},
		{TEXT("CurrentHealth"), [](const UStatComponent& StatComponent) { return StatComponent.GetCurrentHP(); }},
		{TEXT("MaxMana"), [](const UStatComponent& StatComponent) { return StatComponent.GetMaxMP(); }},
		{TEXT("CurrentMana"), [](const UStatComponent& StatComponent) { return StatComponent.GetCurrentMP(); }},
		{TEXT("HealthRegeneration"), [](const UStatComponent& StatComponent) { return StatComponent.GetHealthRegeneration(); }},
		{TEXT("ManaRegeneration"), [](const UStatComponent& StatComponent) { return StatComponent.GetManaRegeneration(); }},
		{TEXT("AttackDamage"), [](const UStatComponent& StatComponent) { return StatComponent.GetAttackDamage(); }},
		{TEXT("AbilityPower"), [](const UStatComponent& StatComponent) { return StatComponent.GetAbilityPower(); }},
		{TEXT("DefensePower"), [](const UStatComponent& StatComponent) { return StatComponent.GetDefensePower(); }},
		{TEXT("MagicResistance"), [](const UStatComponent& StatComponent) { return StatComponent.GetMagicResistance(); }},
		{TEXT("AttackSpeed"), [](const UStatComponent& StatComponent) { return StatComponent.GetAttackSpeed(); }},
		{TEXT("MovementSpeed"), [](const UStatComponent& StatComponent) { return StatComponent.GetMovementSpeed(); }},
	};

	StatGettersInt = {
		{TEXT("AbilityHaste"), [](const UStatComponent& StatComponent) { return StatComponent.GetAbilityHaste(); }},
		{TEXT("CriticalChance"), [](const UStatComponent& StatComponent) { return StatComponent.GetCriticalChance(); }},
	};
}



FString FItemTableRow::ConverClassificationToString() const
{
	switch (Classification)
	{
	case EItemClassification::None:                 return TEXT("None");
	case EItemClassification::Starter:              return TEXT("Starter");
	case EItemClassification::Potions:              return TEXT("Potions");
	case EItemClassification::Consumables:          return TEXT("Consumables");
	case EItemClassification::Trinkets:             return TEXT("Trinkets");
	case EItemClassification::Boots:                return TEXT("Boots");
	case EItemClassification::Basic:                return TEXT("Basic");
	case EItemClassification::Epic:                 return TEXT("Epic");
	case EItemClassification::Legendary:            return TEXT("Legendary");
	case EItemClassification::Distributed:          return TEXT("Distributed");
	case EItemClassification::ChampionExclusive:    return TEXT("Champion Exclusive");
	default:                                    return TEXT("Unknown");
	}
}

FString FItemTableRow::ConvertCharacterStatToString(ECharacterStat StatToConvert) const
{
	//return StaticEnum<ECharacterStat>()->GetNameStringByValue(static_cast<int64>(StatToConvert));

	switch (StatToConvert)
	{
	case ECharacterStat::None:               return TEXT("None");
	case ECharacterStat::CurrentHealth:      return TEXT("Current Health");
	case ECharacterStat::CurrentMana:        return TEXT("Current Mana");
	case ECharacterStat::MaxHealthPoints:    return TEXT("Max Health");
	case ECharacterStat::MaxManaPoints:      return TEXT("Max Mana");
	case ECharacterStat::HealthRegeneration: return TEXT("Health Regeneration");
	case ECharacterStat::ManaRegeneration:   return TEXT("Mana Regeneration");
	case ECharacterStat::AttackDamage:       return TEXT("Attack Damage");
	case ECharacterStat::AbilityPower:       return TEXT("Ability Power");
	case ECharacterStat::DefensePower:       return TEXT("Defense Power");
	case ECharacterStat::MagicResistance:    return TEXT("Magic Resistance");
	case ECharacterStat::AttackSpeed:        return TEXT("Attack Speed");
	case ECharacterStat::AbilityHaste:       return TEXT("Ability Haste");
	case ECharacterStat::CriticalChance:     return TEXT("Critical Chance");
	case ECharacterStat::MovementSpeed:      return TEXT("Movement Speed");
	default:                                 return TEXT("Unknown");
	}
}


FString FItemTableRow::ConvertToRichText(UStatComponent* StatComponent) const
{
	FString Result = Description;

	if (!StatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ConvertToRichText] Invalid StatComponent."));
		return Result;
	}

	// 색상 구분자 처리
	Result = ApplyColorTags(Result);

	// StatComponent 값 구분자 처리
	Result = ReplaceStatTags(Result, StatComponent);

	// 수식 구분자 처리
	Result = ReplaceCalcTags(Result, StatComponent);

	return Result;
}



/**
 * ApplyColorTags 함수는 주어진 텍스트에서 색상 구분자를 찾아
 * HTML 태그 형식으로 대체합니다.
 *
 * @param Text: 입력 텍스트입니다.
 * @return 색상 태그가 적용된 텍스트입니다.
 */
FString FItemTableRow::ApplyColorTags(const FString& Text) const
{
	FString Result = Text;
	Result = Result.Replace(TEXT("{color=red}"), TEXT("<red>"));
	Result = Result.Replace(TEXT("{color=green}"), TEXT("<green>"));
	Result = Result.Replace(TEXT("{color=blue}"), TEXT("<blue>"));
	Result = Result.Replace(TEXT("{/color}"), TEXT("</>"));
	return Result;
}


/**
 * ReplaceStatTags 함수는 주어진 텍스트에서 {stat=...} 형식의 스탯 구분자를 찾아
 * 해당 구분자를 StatComponent의 값으로 대체합니다.
 *
 * @param Text: 입력 텍스트입니다.
 * @param StatComponent: 스탯 값을 제공하는 컴포넌트입니다.
 * @return 변환된 텍스트입니다.
 *
 * 주요 작업:
 * 1. StatComponent가 유효한지 확인합니다.
 * 2. StatGetters 맵을 순회하여 각 스탯 구분자를 StatComponent의 값으로 대체합니다.
 * 3. StatGettersInt 맵을 순회하여 각 스탯 구분자를 StatComponent의 정수 값으로 대체합니다.
 */
FString FItemTableRow::ReplaceStatTags(const FString& Text, UStatComponent* StatComponent) const
{
	FString Result = Text;

	// StatComponent가 유효하지 않으면 반환
	if (!StatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ReplaceStatTags] Invalid StatComponent."));
		return Result;
	}
	

	// StatGetters의 각 함수 포인터를 사용하여 값 대체
	for (const auto& Pair : StatGetters)
	{
		FString Tag = FString::Printf(TEXT("{stat=%s}"), *Pair.Key);
		if (StatGetters.Contains(Pair.Key))
		{
			float Value = Pair.Value(*StatComponent); 
			Result = Result.Replace(*Tag, *FString::SanitizeFloat(Value));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ReplaceStatTags] Stat not found: %s"), *Pair.Key);
		}
	}

	// StatGettersInt의 각 함수 포인터를 사용하여 값 대체
	for (const auto& Pair : StatGettersInt)
	{
		FString Tag = FString::Printf(TEXT("{stat=%s}"), *Pair.Key);
		if (StatGettersInt.Contains(Pair.Key))
		{
			int32 Value = Pair.Value(*StatComponent); 
			Result = Result.Replace(*Tag, *FString::FromInt(Value));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ReplaceStatTags] Stat not found: %s"), *Pair.Key);
		}
	}
	
	return Result;
}



/**
 * ReplaceCalcTags 함수는 주어진 텍스트에서 {calc=...} 형식의 수식 구분자를 찾아 평가된 결과로 대체합니다.
 * - Text: 입력 텍스트입니다.
 * - StatComponent: 스탯 값을 제공하는 컴포넌트입니다.
 *
 * 주요 작업:
 * 1. 수식 구분자를 찾아 평가합니다.
 * 2. 평가된 결과로 수식 구분자를 대체합니다.
 * 3. StatComponent에서 스탯 값을 가져와 변수 맵에 추가합니다.
 */
FString FItemTableRow::ReplaceCalcTags(const FString& Text, UStatComponent* StatComponent) const
{
	if (!StatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ReplaceCalcTags] Invalid StatComponent."));
		return Text;
	}

	
	// std::unordered_map을 사용하여 변수 준비
	std::unordered_map<std::string, double> Variables;
	for (const auto& Pair : StatGetters)
	{
		Variables[std::string(TCHAR_TO_UTF8(*Pair.Key))] = Pair.Value(*StatComponent);
	}

	for (const auto& Pair : StatGettersInt)
	{
		Variables[std::string(TCHAR_TO_UTF8(*Pair.Key))] = static_cast<double>(Pair.Value(*StatComponent));
	}


	FString Result = Text;
	int32 StartIndex = 0;

	while ((StartIndex = Result.Find(TEXT("{calc="), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex)) != INDEX_NONE)
	{
		// 수식 구분자 찾기
		int32 EndIndex = Result.Find(TEXT("}"), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex);
		if (EndIndex == INDEX_NONE || EndIndex <= StartIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ReplaceCalcTags] Invalid or mismatched braces in CalcTag."));
			break;
		}

		// {calc=...} 태그 처리
		FString CalcTag = Result.Mid(StartIndex + 6, EndIndex - StartIndex - 6);

		// 수식 평가를 위해 문자열을 가져옴
		std::string CalcStr = TCHAR_TO_UTF8(*CalcTag);

		// 수식 평가	
		double CalcResult = 0.0;
		if (ExpressionEvaluator().Evaluate(CalcStr, Variables, CalcResult))
		{
			// 평가된 결과를 수식 태그와 교체
			Result = Result.Replace(*FString::Printf(TEXT("{calc=%s}"), *CalcTag), *FString::SanitizeFloat(CalcResult));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ReplaceCalcTags] Failed to evaluate expression: %s"), *CalcTag);
		}

		StartIndex = EndIndex + 1;
	}
	

	return Result;
}