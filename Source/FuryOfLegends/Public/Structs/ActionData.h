#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/UniqueAttributeData.h"
#include "ActionData.generated.h"

UENUM(BlueprintType)
enum class EActionType : uint8
{
    None        UMETA(DisplayName = "None"),       // ����
    Basic       UMETA(DisplayName = "Basic"),      // �⺻
    Buff        UMETA(DisplayName = "Buff"),       // �̵�
    Dash        UMETA(DisplayName = "Dash"),       // ���
    Blink       UMETA(DisplayName = "Blink"),      // �����̵�
    Cleanse     UMETA(DisplayName = "Cleanse"),    // CC ����
};


UENUM(BlueprintType)
enum class EActivationType : uint8
{
    None        UMETA(DisplayName = "None"),        // ����
    Passive     UMETA(DisplayName = "Passive"),     // �нú�
    Active      UMETA(DisplayName = "Active"),      // Ȱ��ȭ(��� �ʿ�)
    Toggle      UMETA(DisplayName = "Toggle"),      // �ѱ�/���� ��ȯ
    Channeling  UMETA(DisplayName = "Channeling"),  // ä�θ�
    Charged     UMETA(DisplayName = "Charged"),     // ��¡(����)
    Targeted    UMETA(DisplayName = "Targeted"),    // ��� ����
    Ranged      UMETA(DisplayName = "RangedTargeted") // ���Ÿ� ��� ����
};

UENUM(BlueprintType)
enum class EMobilityType : uint8
{
    None    UMETA(DisplayName = "None"),            // ����
    Dash    UMETA(DisplayName = "Dash"),            // ���
    Blink   UMETA(DisplayName = "Blink"),           // �����̵�
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EActionSlot : uint8
{
    None        = 0        UMETA(DisplayName = "None"),         // ���� ����
    Q           = 1 << 0   UMETA(DisplayName = "Q Slot"),       // Q �ɷ� ����
    E           = 1 << 1   UMETA(DisplayName = "E Slot"),       // E �ɷ� ����
    R           = 1 << 2   UMETA(DisplayName = "R Slot"),       // R �ɷ� ����
    LMB         = 1 << 3   UMETA(DisplayName = "Left Mouse"),   // ��Ŭ��
    RMB         = 1 << 4   UMETA(DisplayName = "Right Mouse"),  // ��Ŭ��
    Intrinsic   = 1 << 5   UMETA(DisplayName = "Utility"),      // �⺻ �ɷ� (ü��/���� ��� ��)
    Passive     = 1 << 6   UMETA(DisplayName = "Passive"),      // �нú� �ɷ�
};
ENUM_CLASS_FLAGS(EActionSlot);


USTRUCT(BlueprintType)
struct FActionDefinition
{
    GENERATED_BODY()

public:
    FActionDefinition()
        : Name(NAME_None)
        , Description(FText())
        , RequiredLevel(1)
        , CastingTime(0.0f)
        , ActionType(EActionType::None)
        , ActivationType(EActivationType::None)
        , CollisionDetection(ECollisionChannel::ECC_Visibility)
    {
    }

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CastingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EActionType ActionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EActivationType ActivationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TEnumAsByte<ECollisionChannel> CollisionDetection;
};

USTRUCT(BlueprintType)
struct FActionAttributes
{
    GENERATED_BODY()

public:
    FActionAttributes()
        : Name(NAME_None)
        , AttackDamage(0.f)
        , AbilityDamage(0.f)
        , PhysicalScaling(0.f)
        , MagicalScaling(0.f)
        , CooldownTime(0.f)
        , Cost(0.f)
        , Radius(0.f)
        , Range(0.f)
        , ReuseDuration(0.f)
        , UniqueAttributes(TArray<FUniqueAttribute>())
    {}

    bool operator==(const FActionAttributes& Other) const
    {
        return Name == Other.Name;
    }

    bool IsValid() const
    {
        return !Name.IsNone();
    }


public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AbilityDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PhysicalScaling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MagicalScaling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Range;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReuseDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FUniqueAttribute> UniqueAttributes;
};

USTRUCT(BlueprintType)
struct FAction
{
    GENERATED_BODY()

public:
    FAction()
        : ActionDefinition(FActionDefinition())
        , ActionAttributes(TArray<FActionAttributes>())
    {}

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FActionDefinition ActionDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FActionAttributes> ActionAttributes;
};

USTRUCT(BlueprintType)
struct FActionTableRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    FActionTableRow()
        : Actions(TArray<FAction>())
    {
    }

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAction> Actions;
};


UCLASS()
class FURYOFLEGENDS_API UActionData : public UObject
{
	GENERATED_BODY()


};
