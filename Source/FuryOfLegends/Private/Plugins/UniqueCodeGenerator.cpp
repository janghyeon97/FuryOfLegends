// Fill out your copyright notice in the Description page of Project Settings.


#include "Plugins/UniqueCodeGenerator.h"


/**
 * UUniqueCodeGenerator 클래스
 *
 * - 게임 내 능력(Ability)과 아이템(Item)을 고유한 32비트 정수로 표현하고 관리하기 위해 설계된 클래스입니다.
 *
 * 주요 기능:
 * - 능력(Ability)의 슬롯과 공격 단계(Attack Phase)를 고유한 코드로 변환하고 이를 디코딩.
 * - 아이템(Item)의 분류(Classification)와 ID를 고유한 코드로 변환하고 이를 디코딩.
 * - 코드를 통해 데이터의 타입(Type), 세부 정보(Slot/Classification), 고유 값(ID/Phase)을 추출하여 원래 데이터를 복원.
 *
 * 작동 방식:
 * - **코드 생성**: 입력된 데이터(슬롯, 단계, 분류 등)를 비트 필드를 사용해 하나의 32비트 정수로 압축.
 *   - 상위 2비트는 데이터 타입(1=Ability, 2=Item)을 나타냄.
 *   - 중간 비트는 슬롯/분류 정보를 저장.
 *   - 하위 비트는 공격 단계나 고유 ID를 저장.
 */






// <소유 오브젝트>_<오브젝트 인덱스>_<실행 오브젝트>_<타이머 분류>_<서브 필드1>_<서브 필드2>
uint32 UUniqueCodeGenerator::GenerateUniqueCode(EObjectType OwnerObject, uint8 ObjectIndex, ETimerCategory TimerCategory, const uint8 SubField1, const uint8 SubField2)
{
	if (ObjectIndex > 255) // 8비트 제한
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid ObjectIndex: %d"), ObjectIndex);
		return 0;
	}
	if (SubField1 > 15 || SubField2 > 15) // 4비트 제한
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid SubFields: SubField1=%d, SubField2=%d"), SubField1, SubField2);
		return 0;
	}

	uint32 Code = 0;

	// [31:28] 4비트 : OwnerObjectType (최대 16개의 소유 오브젝트 타입)
	Code |= (static_cast<uint32>(OwnerObject) & 0xF) << 28;

	// [27:24] 8비트 : ObjectIndex (최대 256개의 객체)
	Code |= (static_cast<uint32>(ObjectIndex) & 0xFF) << 24;

	// [23:16] 4비트 : TimerCategory (최대 16개의 타이머 분류)
	Code |= (static_cast<uint32>(TimerCategory) & 0xF) << 16;

	// [15:8] 8비트 : SubField1 (최대 256개의 값)
	Code |= (static_cast<uint32>(SubField1) & 0xF) << 8;

	// [7:0] 8비트 : SubField2 (최대 256개의 값)
	Code |= (static_cast<uint32>(SubField2) & 0xF);

	return Code;
}




// ObjectType 반환 (OwnerObject)
EObjectType UUniqueCodeGenerator::DecodeObjectType(uint32 Code)
{
	return static_cast<EObjectType>((Code >> 28) & 0xF); // [31:28]
}

// ObjectIndex 반환
uint8 UUniqueCodeGenerator::DecodeObjectIndex(uint32 Code)
{
	return static_cast<uint8>((Code >> 24) & 0xFF); // [27:24]
}

// TimerCategory 반환
ETimerCategory UUniqueCodeGenerator::DecodeTimerCategory(uint32 Code)
{
	return static_cast<ETimerCategory>((Code >> 16) & 0xF); // [23:16]
}

// SubField1 반환
uint8 UUniqueCodeGenerator::DecodeSubField1(uint32 Code)
{
	return static_cast<uint8>((Code >> 8) & 0xF); // [15:8]
}

// SubField2 반환
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
	// FName을 문자열로 변환하고 ":" 구분자로 파싱
	TArray<FString> ParsedStrings;
	UniqueCode.ToString().ParseIntoArray(ParsedStrings, TEXT("_"), true);

	// 첫 번째 요소(ObjectType)가 있는지 확인
	if (!ParsedStrings.IsValidIndex(0))
	{
		return EObjectType::None;
	}

	// FString에서 Enum으로 변환
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
	// FName을 문자열로 변환하고 ":" 구분자로 파싱
	TArray<FString> ParsedStrings;
	UniqueCode.ToString().ParseIntoArray(ParsedStrings, TEXT("_"), true);

	// 첫 번째 요소(ObjectType)가 있는지 확인
	if (!ParsedStrings.IsValidIndex(2))
	{
		return ETeamSide::None;
	}

	// FString에서 Enum으로 변환
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
	// FName을 문자열로 변환하고 ":" 구분자로 파싱
	TArray<FString> ParsedStrings;
	UniqueCode.ToString().ParseIntoArray(ParsedStrings, TEXT("_"), true);

	// 첫 번째 요소(ObjectType)가 있는지 확인
	if (!ParsedStrings.IsValidIndex(4))
	{
		return ETimerCategory::None;
	}

	// FString에서 Enum으로 변환
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