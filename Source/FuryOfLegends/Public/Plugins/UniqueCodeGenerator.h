// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/ActionData.h"
#include "Structs/CharacterData.h"
#include "UniqueCodeGenerator.generated.h"


// Ÿ�̸� �з�
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
    None        UMETA(DisplayName = "None"),       // Ÿ�̸� ����
    Inventory   UMETA(DisplayName = "Inventory"),  // �κ��丮������ ����
    BuffList    UMETA(DisplayName = "BuffList"),   // ���� ����Ʈ������ ����
    Both        UMETA(DisplayName = "Both")        // �κ��丮 & ���� ����Ʈ ��� ����
};



UCLASS()
class FURYOFLEGENDS_API UUniqueCodeGenerator : public UObject
{
    GENERATED_BODY()


public:
    /** ���� �ڵ� ���� */
    static uint32 GenerateUniqueCode(EObjectType OwnerObject, uint8 ObjectIndex, ETimerCategory TimerCategory, const uint8 SubField1 = 0, const uint8 SubField2 = 0);

    /** ���� �ʵ� ���ڵ� */
    static EObjectType DecodeObjectType(uint32 Code);
    static ETimerCategory DecodeTimerCategory(uint32 Code);
    static uint8 DecodeObjectIndex(uint32 Code);
    static uint8 DecodeSubField1(uint32 Code);
    static uint8 DecodeSubField2(uint32 Code);
};