// Fill out your copyright notice in the Description page of Project Settings.


#include "Plugins/UniqueCodeGenerator.h"


/**
 * UUniqueCodeGenerator Ŭ����
 *
 * - ���� �� �ɷ�(Ability)�� ������(Item)�� ������ 32��Ʈ ������ ǥ���ϰ� �����ϱ� ���� ����� Ŭ�����Դϴ�.
 *
 * �ֿ� ���:
 * - �ɷ�(Ability)�� ���԰� ���� �ܰ�(Attack Phase)�� ������ �ڵ�� ��ȯ�ϰ� �̸� ���ڵ�.
 * - ������(Item)�� �з�(Classification)�� ID�� ������ �ڵ�� ��ȯ�ϰ� �̸� ���ڵ�.
 * - �ڵ带 ���� �������� Ÿ��(Type), ���� ����(Slot/Classification), ���� ��(ID/Phase)�� �����Ͽ� ���� �����͸� ����.
 *
 * �۵� ���:
 * - **�ڵ� ����**: �Էµ� ������(����, �ܰ�, �з� ��)�� ��Ʈ �ʵ带 ����� �ϳ��� 32��Ʈ ������ ����.
 *   - ���� 2��Ʈ�� ������ Ÿ��(1=Ability, 2=Item)�� ��Ÿ��.
 *   - �߰� ��Ʈ�� ����/�з� ������ ����.
 *   - ���� ��Ʈ�� ���� �ܰ質 ���� ID�� ����.
 */






// <���� ������Ʈ>_<������Ʈ �ε���>_<���� ������Ʈ>_<Ÿ�̸� �з�>_<���� �ʵ�1>_<���� �ʵ�2>
uint32 UUniqueCodeGenerator::GenerateUniqueCode(EObjectType OwnerObject, uint8 ObjectIndex, ETimerCategory TimerCategory, const uint8 SubField1, const uint8 SubField2)
{
	if (ObjectIndex > 255) // 8��Ʈ ����
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid ObjectIndex: %d"), ObjectIndex);
		return 0;
	}
	if (SubField1 > 15 || SubField2 > 15) // 4��Ʈ ����
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid SubFields: SubField1=%d, SubField2=%d"), SubField1, SubField2);
		return 0;
	}

	uint32 Code = 0;

	// [31:28] 4��Ʈ : OwnerObjectType (�ִ� 16���� ���� ������Ʈ Ÿ��)
	Code |= (static_cast<uint32>(OwnerObject) & 0xF) << 28;

	// [27:24] 8��Ʈ : ObjectIndex (�ִ� 256���� ��ü)
	Code |= (static_cast<uint32>(ObjectIndex) & 0xFF) << 24;

	// [23:16] 4��Ʈ : TimerCategory (�ִ� 16���� Ÿ�̸� �з�)
	Code |= (static_cast<uint32>(TimerCategory) & 0xF) << 16;

	// [15:8] 8��Ʈ : SubField1 (�ִ� 256���� ��)
	Code |= (static_cast<uint32>(SubField1) & 0xF) << 8;

	// [7:0] 8��Ʈ : SubField2 (�ִ� 256���� ��)
	Code |= (static_cast<uint32>(SubField2) & 0xF);

	return Code;
}




// ObjectType ��ȯ (OwnerObject)
EObjectType UUniqueCodeGenerator::DecodeObjectType(uint32 Code)
{
	return static_cast<EObjectType>((Code >> 28) & 0xF); // [31:28]
}

// ObjectIndex ��ȯ
uint8 UUniqueCodeGenerator::DecodeObjectIndex(uint32 Code)
{
	return static_cast<uint8>((Code >> 24) & 0xFF); // [27:24]
}

// TimerCategory ��ȯ
ETimerCategory UUniqueCodeGenerator::DecodeTimerCategory(uint32 Code)
{
	return static_cast<ETimerCategory>((Code >> 16) & 0xF); // [23:16]
}

// SubField1 ��ȯ
uint8 UUniqueCodeGenerator::DecodeSubField1(uint32 Code)
{
	return static_cast<uint8>((Code >> 8) & 0xF); // [15:8]
}

// SubField2 ��ȯ
uint8 UUniqueCodeGenerator::DecodeSubField2(uint32 Code)
{
	return static_cast<uint8>(Code & 0xF); // [7:0]
}





/*
FName UUniqueCodeGenerator::GenerateUniqueCode(EObjectType ObjectType, uint32 ObjectIndex, FName ObjectName, ETimerCategory TimerCategory, const FString& SubField1, const FString& SubField2)
{
	FString ObjectTypeString = StaticEnum<EObjectType>()->GetNameStringByValue(static_cast<int64>(ObjectType));
	FString TimerPurposeString = StaticEnum<ETimerCategory>()->GetNameStringByValue(static_cast<int64>(TimerCategory));

	return FName(FString::Printf(TEXT("%s_%d_%s_%s_%s_%s"),
		*ObjectTypeString,
		ObjectIndex,
		*ObjectName.ToString(),
		*TimerPurposeString,
		!SubField1.IsEmpty() ? *SubField1 : TEXT("None"),
		!SubField2.IsEmpty() ? *SubField2 : TEXT("None")
	));
}


EObjectType UUniqueCodeGenerator::DecodeObjectType(const FName& UniqueCode)
{
	// FName�� ���ڿ��� ��ȯ�ϰ� ":" �����ڷ� �Ľ�
	TArray<FString> ParsedStrings;
	UniqueCode.ToString().ParseIntoArray(ParsedStrings, TEXT("_"), true);

	// ù ��° ���(ObjectType)�� �ִ��� Ȯ��
	if (!ParsedStrings.IsValidIndex(0))
	{
		return EObjectType::None;
	}

	// FString���� Enum���� ��ȯ
	UEnum* EnumClass = StaticEnum<EObjectType>();
	if (!EnumClass)
	{
		return EObjectType::None;
	}

	int32 EnumValue = EnumClass->GetValueByName(FName(*ParsedStrings[0]));
	if (EnumValue == INDEX_NONE)
	{
		return EObjectType::None;
	}

	return static_cast<EObjectType>(EnumValue);
}


uint32 UUniqueCodeGenerator::DecodeObjectIndex(const FName& UniqueCode)
{
	TArray<FString> ParsedStrings;
	UniqueCode.ToString().ParseIntoArray(ParsedStrings, TEXT("_"), true);
	return ParsedStrings.IsValidIndex(1) ? FCString::Atoi(*ParsedStrings[1]) : 0;
}


ETeamSide UUniqueCodeGenerator::DecodeTeamSide(const FName& UniqueCode)
{
	// FName�� ���ڿ��� ��ȯ�ϰ� ":" �����ڷ� �Ľ�
	TArray<FString> ParsedStrings;
	UniqueCode.ToString().ParseIntoArray(ParsedStrings, TEXT("_"), true);

	// ù ��° ���(ObjectType)�� �ִ��� Ȯ��
	if (!ParsedStrings.IsValidIndex(2))
	{
		return ETeamSide::None;
	}

	// FString���� Enum���� ��ȯ
	UEnum* EnumClass = StaticEnum<ETeamSide>();
	if (!EnumClass)
	{
		return ETeamSide::None;
	}

	int32 EnumValue = EnumClass->GetValueByName(FName(*ParsedStrings[0]));
	if (EnumValue == INDEX_NONE)
	{
		return ETeamSide::None;
	}

	return static_cast<ETeamSide>(EnumValue);
}


ETimerCategory UUniqueCodeGenerator::DecodeTimerPurpose(const FName& UniqueCode)
{
	// FName�� ���ڿ��� ��ȯ�ϰ� ":" �����ڷ� �Ľ�
	TArray<FString> ParsedStrings;
	UniqueCode.ToString().ParseIntoArray(ParsedStrings, TEXT("_"), true);

	// ù ��° ���(ObjectType)�� �ִ��� Ȯ��
	if (!ParsedStrings.IsValidIndex(4))
	{
		return ETimerCategory::None;
	}

	// FString���� Enum���� ��ȯ
	UEnum* EnumClass = StaticEnum<ETeamSide>();
	if (!EnumClass)
	{
		return ETimerCategory::None;
	}

	int32 EnumValue = EnumClass->GetValueByName(FName(*ParsedStrings[0]));
	if (EnumValue == INDEX_NONE)
	{
		return ETimerCategory::None;
	}

	return static_cast<ETimerCategory>(EnumValue);
}


FString UUniqueCodeGenerator::DecodeSubField1(const FName& UniqueCode)
{
	TArray<FString> ParsedStrings;
	UniqueCode.ToString().ParseIntoArray(ParsedStrings, TEXT("_"), true);
	return ParsedStrings.IsValidIndex(5) ? ParsedStrings[4] : TEXT("None");
}

FString UUniqueCodeGenerator::DecodeSubField2(const FName& UniqueCode)
{
	TArray<FString> ParsedStrings;
	UniqueCode.ToString().ParseIntoArray(ParsedStrings, TEXT("_"), true);
	return ParsedStrings.IsValidIndex(6) ? ParsedStrings[5] : TEXT("None");
}

*/