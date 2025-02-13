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
    // 각 군중 제어 효과에 대한 객체를 미리 생성
    void InitializeEffectPools();

    // 효과 클래스 가져오기
    TSubclassOf<UCrowdControlEffect> GetEffectClass(ECrowdControl Type) const;

    // 객체 풀에서 사용 가능한 효과 가져오기
    UCrowdControlEffect* GetEffect(ECrowdControl Type);

    // 사용이 끝난 효과를 풀에 반환
    void ReturnEffect(ECrowdControl Type, UCrowdControlEffect* Effect);

    static UCrowdControlManager* Get()
    {
        if (!Instance)
        {
            Instance = NewObject<UCrowdControlManager>();
            Instance->AddToRoot(); // Garbage Collection 방지
            Instance->InitializeEffectPools();
        }
        return Instance;
    }

    static void Release()
    {
        if (Instance)
        {
            Instance->RemoveFromRoot(); // Garbage Collection에 다시 추가
            Instance = nullptr; // 포인터를 null로 설정하여 안전하게 해제
        }
    }

private:
    static UCrowdControlManager* Instance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CrowdControlsMap", meta = (AllowPrivateAccess = "true"))
    TMap<ECrowdControl, TSubclassOf<UCrowdControlEffect>> CrowdControlClasses;

    // 객체 풀 관리
    UPROPERTY()
    TMap<ECrowdControl, FCrowdControlEffectPool> EffectPools;

    // 초기 풀 크기 설정
    const int32 PoolSizePerType = 10;
};
