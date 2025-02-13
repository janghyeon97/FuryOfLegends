// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/ActionData.h"
#include "Structs/CharacterData.h"
#include "UniqueCodeGenerator.generated.h"


// 타이머 분류
UENUM(BlueprintType)
enum class ETimerCategory : uint8
{
    None        UMETA(DisplayName = "None"),
    Spawn       UMETA(DisplayName = "Spawn"),
    Respawn     UMETA(DisplayName = "Respawn"),
    Interaction UMETA(DisplayName = "Interaction"),
    Rune        UMETA(DisplayName = "Rune"),
    Action      UMETA(DisplayName = "Action"),
    Item        UMETA(DisplayName = "Item"),
};


UENUM(BlueprintType)
enum class ETimerType : uint8
{
    None        UMETA(DisplayName = "None"),       // 타이머 없음
    Inventory   UMETA(DisplayName = "Inventory"),  // 인벤토리에서만 갱신
    BuffList    UMETA(DisplayName = "BuffList"),   // 버프 리스트에서만 갱신
    Both        UMETA(DisplayName = "Both")        // 인벤토리 & 버프 리스트 모두 갱신
};



UCLASS()
class FURYOFLEGENDS_API UUniqueCodeGenerator : public UObject
{
    GENERATED_BODY()


public:
    /** 고유 코드 생성 */
    static uint32 GenerateUniqueCode(EObjectType OwnerObject, uint8 ObjectIndex, ETimerCategory TimerCategory, const uint8 SubField1 = 0, const uint8 SubField2 = 0);

    /** 개별 필드 디코딩 */
    static EObjectType DecodeObjectType(uint32 Code);
    static ETimerCategory DecodeTimerCategory(uint32 Code);
    static uint8 DecodeObjectIndex(uint32 Code);
    static uint8 DecodeSubField1(uint32 Code);
    static uint8 DecodeSubField2(uint32 Code);
};