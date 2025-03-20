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
		{ ECharacterStat::MaxHealthPoints, [](const UStatComponent& StatComponent) { return StatComponent.GetMaxHP(); } },
		{ ECharacterStat::CurrentHealth, [](const UStatComponent& StatComponent) { return StatComponent.GetCurrentHP(); }},
		{ ECharacterStat::MaxManaPoints, [](const UStatComponent& StatComponent) { return StatComponent.GetMaxMP(); }},
		{ ECharacterStat::CurrentMana, [](const UStatComponent& StatComponent) { return StatComponent.GetCurrentMP(); }},
		{ ECharacterStat::HealthRegeneration, [](const UStatComponent& StatComponent) { return StatComponent.GetHealthRegeneration(); }},
		{ ECharacterStat::ManaRegeneration, [](const UStatComponent& StatComponent) { return StatComponent.GetManaRegeneration(); }},
		{ ECharacterStat::AttackDamage, [](const UStatComponent& StatComponent) { return StatComponent.GetAttackDamage(); }},
		{ ECharacterStat::AbilityPower, [](const UStatComponent& StatComponent) { return StatComponent.GetAbilityPower(); }},
		{ ECharacterStat::DefensePower, [](const UStatComponent& StatComponent) { return StatComponent.GetDefensePower(); }},
		{ ECharacterStat::MagicResistance, [](const UStatComponent& StatComponent) { return StatComponent.GetMagicResistance(); }},
		{ ECharacterStat::AttackSpeed, [](const UStatComponent& StatComponent) { return StatComponent.GetAttackSpeed(); }},
		{ ECharacterStat::MovementSpeed, [](const UStatComponent& StatComponent) { return StatComponent.GetMovementSpeed(); }},
	};

	StatGettersInt = {
		{ ECharacterStat::AbilityHaste, [](const UStatComponent& StatComponent) { return StatComponent.GetAbilityHaste(); }},
		{ ECharacterStat::CriticalChance, [](const UStatComponent& StatComponent) { return StatComponent.GetCriticalChance(); }},
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
	default:										return TEXT("Unknown");
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

	// 스탯 태그 변환
	Result = ReplaceCharacterStatTags(Result, StatComponent);
	Result = ReplaceItemStatTags(Result);
	Result = ReplaceItemAttributeTags(Result);

	// 수식 태그 변환
	Result = ReplaceCalcTags(Result, StatComponent);

	// 줄바꿈 태그 변환
	Result = ReplaceLineBreakTags(Result);

	return Result;
}




FString FItemTableRow::ReplaceLineBreakTags(const FString& Text) const
{
	return Text.Replace(TEXT("<br>"), TEXT("\n"));
}




/**
 * ---------------- 현재 사용 하지 않음 ----------------
 * ApplyColorTags 함수는 주어진 텍스트에서 색상 구분자를 찾아
 * HTML 태그 형식으로 대체합니다.
 *
 * @param Text: 입력 텍스트입니다.
 * @return 색상 태그가 적용된 텍스트입니다.

FString FItemTableRow::ApplyColorTags(const FString& Text) const
{
	FString Result = Text;
	Result = Result.Replace(TEXT("{color=red}"), TEXT("<red>"));
	Result = Result.Replace(TEXT("{color=green}"), TEXT("<green>"));
	Result = Result.Replace(TEXT("{color=blue}"), TEXT("<blue>"));
	Result = Result.Replace(TEXT("{/color}"), TEXT("</>"));
	return Result;
}
 */


FString FItemTableRow::ReplaceItemAttributeTags(const FString& Text) const
{
	FString Result = Text;

	for (const auto& Pair : UniqueAttributes)
	{
		FString Tag = FString::Printf(TEXT("<ItemAttribute=%s>"), *Pair.Key.ToString());
		UE_LOG(LogTemp, Warning, TEXT("ItemAttribute Tag: %s"), *Tag);

		Result = Result.Replace(*Tag, *FString::FromInt(Pair.Value));
	}

	return Result;
}


/**
 * ReplaceItemStatTags 함수는 주어진 텍스트에서 {ItemStat=...} 형식의 스탯 구분자를 찾아
 * 해당 구분자를 StatModifiers의 값으로 대체합니다.
 *
 * @param Text: 입력 텍스트입니다.
 * @return 변환된 텍스트입니다.
 */
FString FItemTableRow::ReplaceItemStatTags(const FString& Text) const
{
	const UEnum* EnumPtr = StaticEnum<ECharacterStat>();
	FString Result = Text;

	for (const FItemStatModifier& Modifier : StatModifiers)
	{
		if (!EnumPtr || !EnumPtr->IsValidEnumValue(static_cast<int64>(Modifier.Key))) {
			UE_LOG(LogTemp, Error, TEXT("InitializeActions: EnumPtr is None."));
			continue;
		}

		FString Tag = FString::Printf(TEXT("<ItemStat=%s>"), *EnumPtr->GetNameStringByValue(static_cast<int64>(Modifier.Key)));
		UE_LOG(LogTemp, Warning, TEXT("ItemStat Tag: %s"), *Tag);

		Result = Result.Replace(*Tag, *FString::SanitizeFloat(Modifier.Value));
	}

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
FString FItemTableRow::ReplaceCharacterStatTags(const FString& Text, UStatComponent* StatComponent) const
{
	if (!StatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ReplaceCharacterStatTags] Invalid StatComponent."));
		return Text;
	}

	FString Result = Text;
	const UEnum* EnumPtr = StaticEnum<ECharacterStat>();

	// 정규식 패턴으로 <CharacterStat=StatName> 찾기
	const FRegexPattern StatPattern(TEXT("<CharacterStat=([A-Za-z0-9_]+)>"));
	FRegexMatcher Matcher(StatPattern, Text);

	// 변환할 태그 목록 저장 (한 번에 처리하기 위해)
	TArray<FString> TagsToReplace;
	TArray<FString> Replacements;

	while (Matcher.FindNext())
	{
		FString StatName = Matcher.GetCaptureGroup(1);  // 예: "MaxHealthPoints"

		ECharacterStat StatEnum = (ECharacterStat)EnumPtr->GetValueByNameString(StatName);

		if (StatEnum == ECharacterStat::None)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ReplaceCharacterStatTags] Invalid stat name: %s"), *StatName);
			continue;  // 변환 실패 시 스킵
		}

		// 실수(float) 스탯 변환
		if (const TFunction<float(const UStatComponent&)>* StatFunc = StatGetters.Find(StatEnum))
		{
			float Value = (*StatFunc)(*StatComponent);
			TagsToReplace.Add(Matcher.GetCaptureGroup(0)); // <CharacterStat=MaxHealthPoints>
			Replacements.Add(FString::Printf(TEXT("%.2f"), Value)); // 100.00 같은 값
		}
		// 정수(int) 스탯 변환
		else if (const TFunction<int32(const UStatComponent&)>* StatFuncInt = StatGettersInt.Find(StatEnum))
		{
			int32 Value = (*StatFuncInt)(*StatComponent);
			TagsToReplace.Add(Matcher.GetCaptureGroup(0)); // <CharacterStat=AbilityHaste>
			Replacements.Add(FString::Printf(TEXT("%d"), Value)); // 10 같은 값
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ReplaceCharacterStatTags] Stat not found: %s"), *StatName);
		}
	}

	// 한 번에 태그를 치환하여 정규식 충돌 방지
	for (int32 i = 0; i < TagsToReplace.Num(); i++)
	{
		Result = Result.Replace(*TagsToReplace[i], *Replacements[i]);
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

	FString Result = Text;
	int32 StartIndex = 0;

	while ((StartIndex = Result.Find(TEXT("<calc="), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex)) != INDEX_NONE)
	{
		// 수식 구분자 찾기
		int32 EndIndex = Result.Find(TEXT(">"), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex);
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
		if (ExpressionEvaluator().Evaluate(CalcStr, CalcResult))
		{
			// 평가된 결과를 수식 태그와 교체
			Result = Result.Replace(*FString::Printf(TEXT("<calc=%s>"), *CalcTag), *FString::SanitizeFloat(CalcResult));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ReplaceCalcTags] Failed to evaluate expression: %s"), *CalcTag);
		}

		StartIndex = EndIndex + 1;
	}
	

	return Result;
}