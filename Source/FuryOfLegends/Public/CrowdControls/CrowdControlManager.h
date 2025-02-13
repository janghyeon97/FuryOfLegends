// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/CustomCombatData.h"
#include "CrowdControlManager.generated.h"

class UCrowdControlEffect;

USTRUCT(BlueprintType)
struct FCrowdControlEffectPool
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<UCrowdControlEffect*> EffectPool;
};

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UCrowdControlManager : public UObject
{
	GENERATED_BODY()

private:
    UCrowdControlManager();

public:
    // �� ���� ���� ȿ���� ���� ��ü�� �̸� ����
    void InitializeEffectPools();

    // ȿ�� Ŭ���� ��������
    TSubclassOf<UCrowdControlEffect> GetEffectClass(ECrowdControl Type) const;

    // ��ü Ǯ���� ��� ������ ȿ�� ��������
    UCrowdControlEffect* GetEffect(ECrowdControl Type);

    // ����� ���� ȿ���� Ǯ�� ��ȯ
    void ReturnEffect(ECrowdControl Type, UCrowdControlEffect* Effect);

    static UCrowdControlManager* Get()
    {
        if (!Instance)
        {
            Instance = NewObject<UCrowdControlManager>();
            Instance->AddToRoot(); // Garbage Collection ����
            Instance->InitializeEffectPools();
        }
        return Instance;
    }

    static void Release()
    {
        if (Instance)
        {
            Instance->RemoveFromRoot(); // Garbage Collection�� �ٽ� �߰�
            Instance = nullptr; // �����͸� null�� �����Ͽ� �����ϰ� ����
        }
    }

private:
    static UCrowdControlManager* Instance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CrowdControlsMap", meta = (AllowPrivateAccess = "true"))
    TMap<ECrowdControl, TSubclassOf<UCrowdControlEffect>> CrowdControlClasses;

    // ��ü Ǯ ����
    UPROPERTY()
    TMap<ECrowdControl, FCrowdControlEffectPool> EffectPools;

    // �ʱ� Ǯ ũ�� ����
    const int32 PoolSizePerType = 10;
};
