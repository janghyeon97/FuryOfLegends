// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdControls/CrowdControlManager.h"
#include "CrowdControls/StunEffect.h"
#include "CrowdControls/SlowEffect.h"
#include "CrowdControls/SnareEffect.h"


UCrowdControlManager* UCrowdControlManager::Instance = nullptr;

UCrowdControlManager::UCrowdControlManager()
{
	static ConstructorHelpers::FClassFinder<UStunEffect> StunEffectClass(TEXT("/Script/FuryOfLegends.StunEffect"));
	if (StunEffectClass.Succeeded()) CrowdControlClasses.Add(ECrowdControl::Stun, StunEffectClass.Class);

	static ConstructorHelpers::FClassFinder<USlowEffect> SlowEffectClass(TEXT("/Script/FuryOfLegends.SlowEffect"));
	if (SlowEffectClass.Succeeded()) CrowdControlClasses.Add(ECrowdControl::Slow, SlowEffectClass.Class);

	static ConstructorHelpers::FClassFinder<USnareEffect> SnareEffectClass(TEXT("/Script/FuryOfLegends.SnareEffect"));
	if (SnareEffectClass.Succeeded()) CrowdControlClasses.Add(ECrowdControl::Snare, SnareEffectClass.Class);
}



void UCrowdControlManager::InitializeEffectPools()
{
	for (const auto& Elem : CrowdControlClasses)
	{
		ECrowdControl Type = Elem.Key;
		TSubclassOf<UCrowdControlEffect> EffectClass = Elem.Value;

		FCrowdControlEffectPool PoolStruct;

		for (int32 i = 0; i < PoolSizePerType; ++i)
		{
			UCrowdControlEffect* NewEffect = NewObject<UCrowdControlEffect>(this, EffectClass);
			PoolStruct.EffectPool.Add(NewEffect);
		}

		EffectPools.Add(Type, PoolStruct);
	}
}



TSubclassOf<UCrowdControlEffect> UCrowdControlManager::GetEffectClass(ECrowdControl Type) const
{
	if (const TSubclassOf<UCrowdControlEffect>* FoundClass = CrowdControlClasses.Find(Type))
	{
		if (::IsValid(*FoundClass))
		{
			return *FoundClass;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid class found for crowd control type: %d"), static_cast<int32>(Type));
		}
	}

	return nullptr;
}

UCrowdControlEffect* UCrowdControlManager::GetEffect(ECrowdControl Type)
{
	if (EffectPools.Contains(Type))
	{
		TArray<UCrowdControlEffect*>& Pool = EffectPools[Type].EffectPool;
		if (Pool.Num() > 0)
		{
			UCrowdControlEffect* Effect = Pool.Pop();
			return Effect;
		}
		else
		{
			// 풀이 비어 있으면 새로운 객체 생성
			TSubclassOf<UCrowdControlEffect> EffectClass = GetEffectClass(Type);
			if (EffectClass)
			{
				return NewObject<UCrowdControlEffect>(this, EffectClass);
			}
		}
	}

	return nullptr;
}

void UCrowdControlManager::ReturnEffect(ECrowdControl Type, UCrowdControlEffect* Effect)
{
	if (Effect && EffectPools.Contains(Type))
	{
		EffectPools[Type].EffectPool.Add(Effect);
		Effect->Reset();
	}
}